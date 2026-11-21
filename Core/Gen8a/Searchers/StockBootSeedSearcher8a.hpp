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

#ifndef STOCKBOOTSEEDSEARCHER8A_HPP
#define STOCKBOOTSEEDSEARCHER8A_HPP

#include <Core/Parents/Searchers/SearcherBase.hpp>
#include <array>
#include <string>
#include <vector>

/**
 * @brief What an unmodified console shows of one freshly caught Unown
 *
 * The game never shows IVs: the stats only contain the effort level an IV starts at (IV 0-19 = 0, 20-25 = 1, 26-30 = 2,
 * 31 = 3). So the observation is six effort levels, the nature and the weight as displayed.
 */
struct StockUnown8a
{
    std::array<u8, 6> effort; ///< Effort level 0-3 in generator order (HP, Atk, Def, SpA, SpD, Spe)
    u8 nature;
    u16 weight; ///< Displayed weight in 0.1 kg (40-60)
};

/**
 * @brief One boot seed that reproduces all five Unown
 */
struct StockBootSeedResult8a
{
    u64 seed; ///< s0 at boot (s1 is 0x82A2B175229D6A5B)
    u32 advance; ///< draws from boot to the first Unown's EC
};

/**
 * @brief Recovers the boot seed of the shared RNG from five Unown using only what a stock console shows.
 *
 * Loading a save in an area with five uncaught Unown makes them back to back about 120-150 draws after boot. The only
 * exact draws visible are the natures (5 bits each); an effort level 3 (IV 31) or 2 (IV 26-30) adds one more nearly
 * exact draw. Guessing the low 5 bits of s0 at those draws turns them into linear equations over the 64 seed bits; the
 * remaining kernel is walked on the GPU and every seed is tested against the effort levels of two Unown, then fully
 * simulated on the CPU. This is done for every alignment (start advance and redraws between the Unown), most likely first.
 */
class StockBootSeedSearcher8a : public SearcherBase<StockBootSeedResult8a>
{
public:
    /**
     * @brief Construct a new StockBootSeedSearcher8a object
     *
     * @param unown The five Unown in the order the game made them
     * @param minAdvance Lowest start advance (draws from boot to the first EC)
     * @param maxAdvance Highest start advance
     * @param maxExtra Highest number of redraws searched between two Unown
     */
    StockBootSeedSearcher8a(const std::vector<StockUnown8a> &unown, u32 minAdvance, u32 maxAdvance, u32 maxExtra = 7);

    /**
     * @brief Checks one seed and start advance against the observations
     */
    bool verify(u64 seed, u32 advance) const;

    /**
     * @brief Displayed weight (0.1 kg) of an Unown with the given weight scalar
     */
    static u16 displayedWeight(u8 scalar);

    std::string getError() const;

    /**
     * @brief Number of alignments searched in total
     */
    u64 getAlignments() const;

    /**
     * @brief Starts the search (OpenCL GPU required). It stops at the first seed found.
     */
    void startSearch();

private:
    struct Alignment
    {
        u32 nature0;
        std::array<u8, 4> extra;
        u8 redraw;
        u32 cost;
    };

    std::vector<StockUnown8a> unown;
    std::vector<Alignment> alignments;
    std::string error;
    u32 minAdvance;
    u32 maxAdvance;
    int extraBlock;
    int extraStat;
    int testBlock[2];

    bool searchGPU();
};

#endif // STOCKBOOTSEEDSEARCHER8A_HPP
