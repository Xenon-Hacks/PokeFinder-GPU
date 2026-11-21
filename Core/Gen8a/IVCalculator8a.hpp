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

#ifndef IVCALCULATOR8A_HPP
#define IVCALCULATOR8A_HPP

#include <Core/Global.hpp>
#include <array>
#include <string>
#include <vector>

/**
 * @brief Stats, effort levels and IV ranges of Legends: Arceus
 *
 * The IV never enters the stat formula directly. The game turns each IV into a starting effort level
 * (IV 0-19 = 0, 20-25 = 1, 26-30 = 2, 31 = 3) and adds the grit levels fed to that stat (maximum 10). So the most the
 * screen can tell is the IV band, and 31 is the only exact value. Arithmetic follows the game's stat functions
 * (single precision, round half away for the effort part, truncation for the level part).
 */
namespace IVCalculator8a
{
    /**
     * @brief Base stats of one species and form
     */
    struct Entry
    {
        u16 specie;
        u8 form;
        std::array<u8, 6> stats; ///< HP Atk Def SpA SpD Spe
    };

    /**
     * @brief IV range of one stat
     */
    struct Range
    {
        u8 min;
        u8 max;
    };

    /**
     * @brief Result of \ref analyze for one stat
     */
    struct StatResult
    {
        std::vector<u8> effort; ///< Effort levels that fit the input
        std::vector<Range> ranges; ///< IV ranges that fit, adjacent bands merged
    };

    /**
     * @brief Result of \ref analyze
     */
    struct Result
    {
        std::array<StatResult, 6> stats;
        std::vector<std::string> errors; ///< Inputs that cannot occur in the game
        std::vector<int> forced; ///< Stats that must be 31 because of the guaranteed perfect IVs
    };

    /**
     * @brief Every species and form of the game
     */
    const std::vector<Entry> &getEntries();

    /**
     * @brief Base stats of a species and form, or nullptr when it is not in the game
     */
    const Entry *getEntry(u16 specie, u8 form);

    /**
     * @brief Starting effort level of an IV (0-3)
     */
    u8 bias(u8 iv);

    /**
     * @brief Effort levels a freshly caught Pokemon shows, in the order of \p ivs
     */
    std::array<u8, 6> effortLevels(const std::array<u8, 6> &ivs);

    /**
     * @brief Stat the game shows
     *
     * @param stat 0-5 in HP Atk Def SpA SpD Spe order
     * @param base Base stat
     * @param level Level
     * @param effort Effort level 0-10
     * @param nature Nature (the mint's nature if one was used)
     */
    u16 calcStat(int stat, u8 base, u8 level, u8 effort, u8 nature);

    /**
     * @brief Works out which IVs fit what the summary shows
     *
     * @param base Base stats
     * @param level Level
     * @param nature Nature
     * @param effort Effort level per stat, -1 when unknown
     * @param stats Stat per stat, -1 when unknown
     * @param grit Grit levels applied per stat, -1 when unknown
     * @param flawless Number of IVs the encounter guarantees as 31 (alphas and legends: 3)
     */
    Result analyze(const std::array<u8, 6> &base, u8 level, u8 nature, const std::array<int, 6> &effort, const std::array<int, 6> &stats,
                   const std::array<int, 6> &grit, int flawless);
}

#endif // IVCALCULATOR8A_HPP
