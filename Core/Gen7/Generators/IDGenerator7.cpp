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

#include "IDGenerator7.hpp"
#include <Core/RNG/SFMT.hpp>

IDGenerator7::IDGenerator7(u32 initialAdvances, u32 maxAdvances, const IDFilter &filter) : IDGenerator(initialAdvances, maxAdvances, filter)
{
}

std::vector<IDState8> IDGenerator7::generate(u32 seed) const
{
    SFMT sfmt(seed, initialAdvances);

    std::vector<IDState8> states;
    for (u32 cnt = 0; cnt <= maxAdvances; cnt++)
    {
        u32 id = static_cast<u32>(sfmt.next());
        u16 tid = id & 0xffff;
        u16 sid = id >> 16;

        IDState8 state(initialAdvances + cnt, tid, sid, id % 1000000);
        if (filter.compare(state))
        {
            states.emplace_back(state);
        }
    }
    return states;
}
