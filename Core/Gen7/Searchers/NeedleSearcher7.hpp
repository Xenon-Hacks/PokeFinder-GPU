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

#ifndef NEEDLESEARCHER7_HPP
#define NEEDLESEARCHER7_HPP

#include <Core/Parents/Searchers/SearcherBase.hpp>
#include <string>
#include <vector>

/**
 * @brief Finds the Gen 7 boot seed from the Continue card's loading-clock needles. Each title -> Continue load draws one
 * rand(17) from the main RNG, the first at the title advance (417 Sun/Moon, 477 Ultra Sun/Ultra Moon), then one more every
 * time the player backs out with B and presses A again. 8 needles identify the seed in most boots, 9-10 in the rest.
 * The GPU kernel runs the 2^32 seeds in about 65-80 s on an RTX 4090; the CPU path is for small ranges.
 */
class NeedleSearcher7 : public SearcherBase<u32>
{
public:
    /**
     * @brief Construct a new NeedleSearcher7 object
     *
     * @param titleAdvance Advance of the first needle (Gen7::titleAdvance)
     * @param needles Needles read, 0-16, in the order they were drawn
     */
    NeedleSearcher7(u32 titleAdvance, const std::vector<u8> &needles);

    /**
     * @brief Computes the needles a seed shows: \p count values from \p titleAdvance
     */
    static std::vector<u8> needlesFor(u32 seed, u32 titleAdvance, u32 count);

    /**
     * @brief Returns the error of the last GPU attempt, empty when none
     */
    std::string getError() const;

    /**
     * @brief Returns whether the GPU did the search
     */
    bool getUsedGPU() const;

    /**
     * @brief Starts the search over \p count seeds from \p start (start 0, count 2^32 for every seed)
     *
     * @param threads CPU threads when the GPU is not used
     * @param useGPU Whether to try the GPU first
     */
    void startSearch(int threads, bool useGPU, u32 start, u64 count);

private:
    std::vector<u8> needles;
    std::string error;
    u32 titleAdvance;
    std::atomic<bool> usedGPU;

    void addResult(u32 seed);
    void searchCPU(u32 start, u64 count, int threads);
    bool searchGPU(u32 start, u64 count);
};

#endif // NEEDLESEARCHER7_HPP
