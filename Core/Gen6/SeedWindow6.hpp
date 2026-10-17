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
    constexpr u64 maxConsoleMs = 36525ULL * 86400000ULL - 1; ///< 2099-12-31 23:59:59.999, the latest 3DS clock setting

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
     * @brief A boot whose seed was found without the save seed
     */
    struct Boot
    {
        u64 titleMs; ///< Console time the title screen appeared, as noted (ms since 2000)
        u32 seed; ///< Seed found for that boot (full 2^32 search + narrowing)
    };

    /**
     * @brief Save seed estimated from boots whose seeds were found without it
     */
    struct Recovery
    {
        u32 saveSeed; ///< seed - noted title time of the first boot
        s64 worstMismatch; ///< Largest disagreement (ms) of the other boots with that estimate
        bool consistent; ///< Whether every other boot agrees within the tolerance
    };

    /**
     * @brief Estimates Misc[0x00] from boots whose seeds were found without it
     *
     * seed = save seed + T, so the save seed is seed - T. The noted title time is only good to about a second, so the estimate is
     * off by that error. The error is a constant shift of the clock: clock searches still find the seed (with the console time
     * shifted by the same amount) and Calibrate absorbs it into the delay, so the estimate works like the real value. More boots
     * cannot remove the error (every boot gives the same seed - T relation), but they check that the found seeds are right:
     * a wrong seed would put the other boots' implied title times far from the noted ones.
     *
     * @param boots Found boots, at least one
     * @param tolerance How far the noted title times may be off, in ms
     *
     * @return Estimated save seed and how well the other boots agree
     */
    inline Recovery recoverSaveSeed(const std::vector<Boot> &boots, u64 tolerance)
    {
        Recovery recovery { boots[0].seed - static_cast<u32>(boots[0].titleMs), 0, true };
        for (size_t i = 1; i < boots.size(); i++)
        {
            // Implied title time of this boot minus the noted one, as a signed 32-bit ms difference
            s64 mismatch = static_cast<s32>(boots[i].seed - recovery.saveSeed - static_cast<u32>(boots[i].titleMs));
            recovery.worstMismatch = std::max(recovery.worstMismatch, mismatch < 0 ? -mismatch : mismatch);
        }
        recovery.consistent = static_cast<u64>(recovery.worstMismatch) <= 2 * tolerance;
        return recovery;
    }

    /**
     * @brief Timer plan for hitting one seed
     */
    struct TimerPlan
    {
        u64 clockSet; ///< Console time to set (whole minute); start the timer when it is confirmed
        u64 target; ///< Console time the title screen must seed at
        u64 timer; ///< ms from the clock confirm to the timed press
        bool valid; ///< false when the target falls after the latest 3DS clock setting
    };

    /**
     * @brief Plans the clock and timer that make the title screen set \p seed
     *
     * The title screen seeds at press + \p delay, so the press has to come at target - delay. The 3DS clock can only be set to a
     * whole minute and starts counting from second 0 when it is confirmed, so the clock is set to the last minute that still
     * leaves at least \p lead ms before the press.
     *
     * @param seed Target initial MT seed
     * @param saveSeed Save Misc[0x00]
     * @param aroundMs Earliest console time to aim for, ms since 2000
     * @param delay Calibrated ms from the timed press to the title screen seeding
     * @param lead Minimum ms between the clock confirm and the press
     *
     * @return Timer plan
     */
    inline TimerPlan planTimer(u32 seed, u32 saveSeed, u64 aroundMs, u64 delay, u64 lead)
    {
        constexpr u64 minute = 60000;
        TimerPlan plan;
        plan.target = timesFor(seed, saveSeed, aroundMs + delay + lead, 1)[0];
        plan.clockSet = (plan.target - delay - lead) / minute * minute;
        plan.timer = plan.target - delay - plan.clockSet;
        plan.valid = plan.target <= maxConsoleMs;
        return plan;
    }

    /**
     * @brief Delay measured from an attempt
     *
     * @param foundMs Console time of the seed the attempt actually got (from the Seed Finder)
     * @param clockSet Console time the clock was confirmed at
     * @param timer ms the press was timed at
     *
     * @return ms from the press to the title screen seeding
     */
    constexpr s64 measuredDelay(u64 foundMs, u64 clockSet, u64 timer)
    {
        return static_cast<s64>(foundMs) - static_cast<s64>(clockSet) - static_cast<s64>(timer);
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
