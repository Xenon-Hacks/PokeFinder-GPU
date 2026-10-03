/*
 * This file is part of PokéFinder
 * Copyright (C) 2017-2026 by Admiral_Fish, bumba, and EzPzStreamz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "IVSearcher5GPU.hpp"
#include <Core/Util/DateTime.hpp>
#include <algorithm>
#include <thread>

static std::vector<u32> getKeypressValues(const std::vector<Keypress> &keypresses)
{
    std::vector<u32> values;
    values.reserve(keypresses.size());
    for (const auto &keypress : keypresses)
    {
        values.emplace_back(keypress.value);
    }
    return values;
}

template <class Generator, class State>
IVSearcher5GPU<Generator, State>::IVSearcher5GPU(u32 initialAdvances, u32 maxAdvances, const Generator &generator,
                                                 const Profile5 &profile) :
    SearcherBase5<Generator, State>(generator, profile), initialAdvances(initialAdvances), maxAdvances(maxAdvances)
{
    scanner = std::make_unique<GPUIVScanner5>(profile, getKeypressValues(this->keypresses), generator.getIVPrefilter(), initialAdvances,
                                              maxAdvances);
}

template <class Generator, class State>
IVSearcher5GPU<Generator, State>::IVSearcher5GPU(u32 initialAdvances, u32 maxAdvances,
                                                 const fph::MetaFphMap<u64, std::array<u8, 6>> &ivCache, const Generator &generator,
                                                 const Profile5 &profile) :
    SearcherBase5<Generator, State>(generator, profile), initialAdvances(initialAdvances), maxAdvances(maxAdvances)
{
    // Same advance range IVSearcher5Fast looks up
    std::vector<std::pair<u64, std::array<u8, 6>>> entries;
    for (const auto &[key, ivs] : ivCache)
    {
        u32 advance = key >> 32;
        if (advance >= initialAdvances && advance <= (initialAdvances + maxAdvances))
        {
            entries.emplace_back(key, ivs);
        }
    }
    scanner = std::make_unique<GPUIVScanner5>(profile, getKeypressValues(this->keypresses), entries);
}

template <class Generator, class State>
bool IVSearcher5GPU<Generator, State>::isSupported(const Generator &generator, u32 initialAdvances, u32 maxAdvances, bool ivCache)
{
    auto prefilter = generator.getIVPrefilter();
    return prefilter.supported && (ivCache || GPUIVScanner5::supportsMT(prefilter, initialAdvances, maxAdvances));
}

template <class Generator, class State>
std::string IVSearcher5GPU<Generator, State>::getError() const
{
    return scanner->getError();
}

template <class Generator, class State>
bool IVSearcher5GPU<Generator, State>::isReady() const
{
    return scanner->isReady();
}

template <class Generator, class State>
void IVSearcher5GPU<Generator, State>::startSearch(int threads, const Date &start, const Date &end)
{
    this->threads = std::max(threads, 1);
    SearcherBase5<Generator, State>::startSearch(threads, start, end);
}

template <class Generator, class State>
void IVSearcher5GPU<Generator, State>::finish(const std::vector<GPUIVScanner5::Candidate> &candidates, size_t begin, size_t end,
                                              const Date &day, u16 timer0)
{
    std::vector<SearcherState5<State>> found;
    std::vector<std::pair<u32, std::array<u8, 6>>> ivs;
    for (size_t i = begin; i < end;)
    {
        if (this->cancelled.load(std::memory_order_relaxed))
        {
            break;
        }

        u32 index = candidates[i].index;
        u64 seed = candidates[i].seed;

        std::vector<State> states;
        if (scanner->usesCache())
        {
            // Matches IVSearcher5Fast, which generates each cached IV advance on its own
            for (; i < end && candidates[i].index == index; i++)
            {
                auto advanceStates = this->generator.generate(seed, { { candidates[i].ivAdvance, candidates[i].ivs } });
                states.insert(states.end(), advanceStates.begin(), advanceStates.end());
            }
        }
        else
        {
            // Matches IVSearcher5, which generates every passing IV advance of a seed together
            ivs.clear();
            for (; i < end && candidates[i].index == index; i++)
            {
                if (this->generator.compareIVs(candidates[i].ivs))
                {
                    ivs.emplace_back(candidates[i].ivAdvance, candidates[i].ivs);
                }
            }
            if (!ivs.empty())
            {
                states = this->generator.generate(seed, ivs);
            }
        }

        if (!states.empty())
        {
            DateTime dt(day, index % 86400);
            auto button = this->keypresses[index / 86400].button;
            for (const auto &state : states)
            {
                found.emplace_back(dt, seed, button, timer0, state);
            }
        }
    }

    if (!found.empty())
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->results.insert(this->results.end(), found.begin(), found.end());
    }
}

template <class Generator, class State>
void IVSearcher5GPU<Generator, State>::search(const Date &start, const Date &end)
{
    u32 keyTotal = static_cast<u32>(this->keypresses.size());
    u32 keysPerLaunch = scanner->getKeypressesPerLaunch();
    std::vector<GPUIVScanner5::Candidate> candidates;
    while (true)
    {
        Date day = start + this->index.fetch_add(1, std::memory_order_relaxed);
        if (day > end)
        {
            break;
        }

        for (u16 timer0 = this->profile.getTimer0Min(); timer0 <= this->profile.getTimer0Max(); timer0++)
        {
            for (u32 keyStart = 0; keyStart < keyTotal; keyStart += keysPerLaunch)
            {
                if (this->cancelled.load(std::memory_order_relaxed))
                {
                    return;
                }

                u32 keyCount = std::min(keysPerLaunch, keyTotal - keyStart);
                candidates.clear();
                bool ok;
                {
                    // The device is shared, other threads finish their candidates on the CPU meanwhile
                    std::lock_guard<std::mutex> lock(scannerMutex);
                    ok = scanner->scan(day, timer0, keyStart, keyCount, candidates);
                }
                if (!ok)
                {
                    this->cancelled.store(true, std::memory_order_relaxed);
                    return;
                }

                std::ranges::sort(candidates, [](const auto &left, const auto &right) {
                    return left.index != right.index ? left.index < right.index : left.ivAdvance < right.ivAdvance;
                });

                // Searcher threads are capped at one per day, so spread loose filters' candidates over the spare threads
                int helpers = candidates.size() < 1024 ? 1 : std::max(1, threads / std::max(1, this->activeThreads.load()));
                if (helpers == 1)
                {
                    finish(candidates, 0, candidates.size(), day, timer0);
                }
                else
                {
                    std::vector<size_t> bounds { 0 };
                    for (int h = 1; h < helpers; h++)
                    {
                        size_t bound = std::max(bounds.back(), candidates.size() * h / helpers);
                        while (bound > 0 && bound < candidates.size() && candidates[bound].index == candidates[bound - 1].index)
                        {
                            bound++;
                        }
                        bounds.emplace_back(bound);
                    }
                    bounds.emplace_back(candidates.size());

                    std::vector<std::thread> workers;
                    for (int h = 1; h < helpers; h++)
                    {
                        workers.emplace_back([&, h] { finish(candidates, bounds[h], bounds[h + 1], day, timer0); });
                    }
                    finish(candidates, bounds[0], bounds[1], day, timer0);
                    for (auto &worker : workers)
                    {
                        worker.join();
                    }
                }

                this->progress.fetch_add(keyCount, std::memory_order_relaxed);
            }
        }
    }
}

#include <Core/Gen5/Generators/StaticGenerator5.hpp>
#include <Core/Gen5/Generators/WildGenerator5.hpp>
#include <Core/Gen5/States/State5.hpp>
#include <Core/Gen5/States/WildState5.hpp>

template class IVSearcher5GPU<StaticGenerator5, State5>;
template class IVSearcher5GPU<WildGenerator5, WildState5>;
