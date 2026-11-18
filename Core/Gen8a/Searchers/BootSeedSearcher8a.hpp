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

#ifndef BOOTSEEDSEARCHER8A_HPP
#define BOOTSEEDSEARCHER8A_HPP

#include <Core/Parents/Searchers/SearcherBase.hpp>
#include <array>
#include <string>
#include <vector>

/**
 * @brief What the summary screen shows of one Unown made on the area load
 */
struct UnownObservation8a
{
    std::array<u8, 6> ivs;
    s8 ability = -1; ///< slot 0/1, or -1 when unknown (both Unown abilities are Levitate)
    u8 nature;
    u8 weightMin = 0; ///< weight scalar range, 0-255 when unknown
    u8 weightMax = 255;
};

/**
 * @brief One boot seed that reproduces every observed Unown
 */
struct BootSeedResult8a
{
    u64 seed; ///< s0 at boot (s1 is 0x82A2B175229D6A5B)
    u32 advance; ///< draws from boot to the first Unown's EC
    u32 end; ///< draws from boot to the end of the last observed Unown
    std::vector<u8> weights; ///< weight scalar of each observed Unown
};

/**
 * @brief Recovers the boot seed of the shared RNG from the Unown made when a save is loaded.
 *
 * At boot the shared RNG is (64-bit hardware random, 0x82A2B175229D6A5B). Loading the save in an area with uncaught
 * Unown makes them back to back 120-170 draws later (each Unown: EC, trainer id, PID, 6 IVs, ability, nature with
 * redraws, weight). The search goes over every start advance and every gap to the second Unown (3 + its
 * nature/weight redraws), solves the seed bits from the IVs (\ref Linear8a) and checks every candidate by simulation.
 */
class BootSeedSearcher8a : public SearcherBase<BootSeedResult8a>
{
public:
    /**
     * @brief Construct a new BootSeedSearcher8a object
     *
     * @param unown Observed Unown in the order the game made them (the order of the area's uncaught forms)
     * @param minAdvance Lowest start advance (draws from boot to the first EC)
     * @param maxAdvance Highest start advance
     * @param maxGap Highest gap to the second Unown (3 = no redraws)
     */
    BootSeedSearcher8a(const std::vector<UnownObservation8a> &unown, u32 minAdvance, u32 maxAdvance, u32 maxGap = 14);

    /**
     * @brief Checks one seed and start advance against the observations. Returns the end advance, or 0 when it does not fit
     */
    u32 verify(u64 seed, u32 advance, std::vector<u8> *weights = nullptr) const;

    std::string getError() const;
    bool getUsedGPU() const;

    /**
     * @brief Starts the search
     *
     * @param threads CPU threads when the GPU is not used
     * @param useGPU Whether to try the GPU first
     */
    void startSearch(int threads, bool useGPU);

private:
    std::vector<UnownObservation8a> unown;
    std::vector<u64> seen;
    std::string error;
    u32 minAdvance;
    u32 maxAdvance;
    u32 maxGap;
    std::atomic<bool> usedGPU;

    void addResult(u64 seed, u32 advance);
    void searchCPU(int threads);
    bool searchGPU();
};

#endif // BOOTSEEDSEARCHER8A_HPP
