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

#ifndef SEEDWINDOW6_HPP
#define SEEDWINDOW6_HPP

#include <Core/Gen6/States/State6.hpp>
#include <Core/Global.hpp>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

/**
 * @brief X/Y seeds as a function of the console clock
 *
 * When the title screen loads the game seeds MT with save Misc[0x00] + T, where T is the console clock in ms since 2000-01-01
 * truncated to 32 bits. A seed therefore maps back to one console time every 2^32 ms (49 days 17:02:47.296).
 */
namespace SeedWindow6
{
    constexpr u64 period = 1ULL << 32; ///< ms between two console times that give the same seed

    /**
     * @brief Seed the title screen sets at console time \p ms
     *
     * @param saveSeed Save Misc[0x00]
     * @param ms Console ms since 2000
     *
     * @return Initial MT seed
     */
    constexpr u32 seedAt(u32 saveSeed, u64 ms)
    {
        return saveSeed + static_cast<u32>(ms);
    }

    /**
     * @brief Console times at or after \p fromMs whose title screen gives \p seed
     *
     * @param seed Initial MT seed
     * @param saveSeed Save Misc[0x00]
     * @param fromMs Earliest console ms since 2000
     * @param count Number of times to return
     *
     * @return Console times in ms since 2000
     */
    inline std::vector<u64> timesFor(u32 seed, u32 saveSeed, u64 fromMs, u32 count)
    {
        u64 offset = static_cast<u32>(seed - saveSeed);
        u64 time = fromMs - (fromMs % period) + offset;
        if (time < fromMs)
        {
            time += period;
        }

        std::vector<u64> times;
        for (u32 i = 0; i < count; i++)
        {
            times.emplace_back(time + i * period);
        }
        return times;
    }

    /**
     * @brief Result of a seed window generation
     */
    struct Window
    {
        std::vector<State6> states; ///< Matching states, sorted by console time then advance
        u64 seeds = 0; ///< Seeds in the window
        u64 seedsWithHits = 0; ///< Seeds with at least one matching state
    };

    /**
     * @brief Runs \p generator for every seed of the console window [\p msStart, \p msEnd]
     *
     * @tparam Generator Gen6 generator with generate(u32 seed)
     * @param generator Generator
     * @param saveSeed Save Misc[0x00]
     * @param msStart First console ms since 2000
     * @param msEnd Last console ms since 2000
     * @param threads Number of threads
     *
     * @return Matching states and how many seeds have one
     */
    template <class Generator>
    Window generate(const Generator &generator, u32 saveSeed, u64 msStart, u64 msEnd, int threads)
    {
        Window window;
        window.seeds = msEnd - msStart + 1;

        std::mutex mutex;
        std::atomic<u64> next(msStart);
        std::atomic<u64> hits(0);
        auto worker = [&] {
            std::vector<State6> local;
            for (u64 ms = next.fetch_add(1); ms <= msEnd; ms = next.fetch_add(1))
            {
                u32 seed = seedAt(saveSeed, ms);
                auto states = generator.generate(seed);
                if (!states.empty())
                {
                    hits++;
                    for (auto &state : states)
                    {
                        state.setSeed(seed, ms);
                        local.emplace_back(state);
                    }
                }
            }

            std::lock_guard<std::mutex> guard(mutex);
            window.states.insert(window.states.end(), local.begin(), local.end());
        };

        std::vector<std::thread> pool;
        for (int i = 1; i < std::max(threads, 1); i++)
        {
            pool.emplace_back(worker);
        }
        worker();
        for (auto &thread : pool)
        {
            thread.join();
        }

        std::ranges::stable_sort(window.states, [](const State6 &left, const State6 &right) {
            return left.getMs() != right.getMs() ? left.getMs() < right.getMs() : left.getAdvances() < right.getAdvances();
        });
        window.seedsWithHits = hits;
        return window;
    }
}

#endif // SEEDWINDOW6_HPP
