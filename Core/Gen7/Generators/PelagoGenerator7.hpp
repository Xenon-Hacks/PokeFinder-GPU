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

#ifndef PELAGOGENERATOR7_HPP
#define PELAGOGENERATOR7_HPP

#include <Core/Gen7/Profile7.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <Core/Parents/Generators/Generator.hpp>

class GeneratorState;

/**
 * @brief Poké Pelago visitor generator: which species visits comes from the Pelago TinyMT in the save; its stats are a 12-draw block
 * from the main RNG when Pelago is entered (EC, a discarded OT ID draw, PID, 6 IVs, ability, nature, gender), the last blocks built
 */
class PelagoGenerator7 : public Generator<Profile7, StateFilter>
{
public:
    /**
     * @brief Construct a new PelagoGenerator7 object
     *
     * @param initialAdvances Initial number of advances
     * @param maxAdvances Maximum number of advances
     * @param specie Visiting specie
     * @param level Visitor level
     * @param profile Profile Information
     * @param filter State filter
     */
    PelagoGenerator7(u32 initialAdvances, u32 maxAdvances, u16 specie, u8 level, const Profile7 &profile, const StateFilter &filter);

    /**
     * @brief Generates states for the \p seed
     *
     * @param seed Starting PRNG state
     *
     * @return Vector of computed states
     */
    std::vector<GeneratorState> generate(u32 seed) const;

private:
    u16 specie;
    u8 level;
};

#endif // PELAGOGENERATOR7_HPP
