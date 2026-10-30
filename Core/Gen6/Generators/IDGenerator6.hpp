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

#ifndef IDGENERATOR6_HPP
#define IDGENERATOR6_HPP

#include <Core/Parents/Generators/IDGenerator.hpp>
#include <array>

class IDState;

/**
 * @brief TID/SID generator for X/Y
 *
 * The IDs are one TinyMT output drawn right after the player name is confirmed: TID = low 16 bits, SID = high 16 bits (live X 1.0).
 */
class IDGenerator6 : public IDGenerator
{
public:
    /**
     * @brief Construct a new IDGenerator6 object
     *
     * @param initialAdvances Initial number of TinyMT advances
     * @param maxAdvances Maximum number of advances
     * @param filter State filter
     */
    IDGenerator6(u32 initialAdvances, u32 maxAdvances, const IDFilter &filter);

    /**
     * @brief Generates states from a TinyMT state
     *
     * @param state TinyMT state (4 words, as read from RAM 0x8C52808)
     *
     * @return Vector of computed states
     */
    std::vector<IDState> generate(const std::array<u32, 4> &state) const;

    /**
     * @brief Generates states from a TinyMT seed
     *
     * @param seed TinyMT seed (console clock in ms at boot)
     *
     * @return Vector of computed states
     */
    std::vector<IDState> generate(u32 seed) const;
};

#endif // IDGENERATOR6_HPP
