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

#include "IDGenerator6.hpp"
#include <Core/Parents/States/IDState.hpp>
#include <Core/RNG/TinyMT.hpp>

static std::vector<IDState> generateIDs(TinyMT &rng, u32 initialAdvances, u32 maxAdvances, const IDFilter &filter)
{
    rng.jump(initialAdvances);

    std::vector<IDState> states;
    for (u32 cnt = 0; cnt <= maxAdvances; cnt++)
    {
        u32 sidtid = rng.next();
        u16 tid = sidtid & 0xffff;
        u16 sid = sidtid >> 16;

        IDState state(initialAdvances + cnt, tid, sid, (tid ^ sid) >> 4);
        if (filter.compare(state))
        {
            states.emplace_back(state);
        }
    }

    return states;
}

IDGenerator6::IDGenerator6(u32 initialAdvances, u32 maxAdvances, const IDFilter &filter) : IDGenerator(initialAdvances, maxAdvances, filter)
{
}

std::vector<IDState> IDGenerator6::generate(const std::array<u32, 4> &state) const
{
    TinyMT rng(state[0], state[1], state[2], state[3]);
    return generateIDs(rng, initialAdvances, maxAdvances, filter);
}

std::vector<IDState> IDGenerator6::generate(u32 seed) const
{
    TinyMT rng(seed);
    return generateIDs(rng, initialAdvances, maxAdvances, filter);
}
