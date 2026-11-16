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

#include "SpawnerGenerator8a.hpp"

namespace SpawnerGenerator8a
{
    std::vector<SpawnerState8a> generate(u64 groupSeed, u32 fills, u32 fillSize, u8 rolls, u8 genderRatio, bool shinyOnly)
    {
        std::vector<SpawnerState8a> states;
        Spec8a spec;
        spec.rolls = rolls ? rolls : 1;
        spec.genderRatio = genderRatio;

        u64 group = groupSeed;
        for (u32 fill = 0; fill < fills; fill++)
        {
            Xoro8a r(group);
            for (u32 slot = 0; slot < fillSize; slot++)
            {
                SpawnerState8a s;
                s.fill = fill;
                s.slot = slot;
                s.groupSeed = group;
                s.slotSeed = r.next();
                s.alphaSeed = r.next();

                Xoro8a slotRng(s.slotSeed);
                s.slotRoll = static_cast<double>(slotRng.next()) * 0x1p-64;
                s.entitySeed = slotRng.next();

                Xoro8a entity(s.entitySeed);
                s.state = Generator8a::generate(entity, spec, true);
                if (!shinyOnly || s.state.shiny)
                {
                    states.emplace_back(s);
                }
            }
            group = r.next();
        }
        return states;
    }
}
