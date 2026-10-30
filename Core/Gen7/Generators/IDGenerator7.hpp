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

#ifndef IDGENERATOR7_HPP
#define IDGENERATOR7_HPP

#include <Core/Gen8/States/IDState8.hpp>
#include <Core/Parents/Generators/IDGenerator.hpp>

/**
 * @brief Gen 7 trainer ID generator: ID = low 32 bits of one main draw when the name is confirmed, TID = low 16, SID = high 16,
 * the 6-digit ID shown in game = ID % 1,000,000 (the same shape as Gen 8, so the Gen 8 ID state is reused)
 */
class IDGenerator7 : public IDGenerator
{
public:
    /**
     * @brief Construct a new IDGenerator7 object
     *
     * @param initialAdvances Initial number of advances (1012 on Sun/Moon when the name is confirmed)
     * @param maxAdvances Maximum number of advances
     * @param filter State filter
     */
    IDGenerator7(u32 initialAdvances, u32 maxAdvances, const IDFilter &filter);

    /**
     * @brief Generates states from the boot seed
     *
     * @param seed Main RNG seed
     */
    std::vector<IDState8> generate(u32 seed) const;
};

#endif // IDGENERATOR7_HPP
