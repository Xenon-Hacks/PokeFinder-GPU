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

#include "Generator8a.hpp"

static u8 size(u16 code, Xoro8a &rng, bool seeded)
{
    if (code < 0x100)
    {
        return static_cast<u8>(code);
    }
    switch (code)
    {
    case 0x100:
    {
        u32 a = rng.rand(0x81, seeded);
        return static_cast<u8>(a + rng.rand(0x80, seeded));
    }
    case 0x101:
        return static_cast<u8>(rng.rand(16, seeded));
    case 0x102:
        return static_cast<u8>(0x10 + rng.rand(32, seeded));
    case 0x103:
        return static_cast<u8>(0x30 + rng.rand(160, seeded));
    case 0x104:
        return static_cast<u8>(0xD0 + rng.rand(32, seeded));
    case 0x105:
        return static_cast<u8>(0xF0 + rng.rand(16, seeded));
    default:
        return 0x80;
    }
}

namespace Generator8a
{
    State8a generate(Xoro8a &rng, const Spec8a &spec, bool seeded)
    {
        State8a state;
        u32 start = rng.draws;

        state.ec = spec.ec < 0 ? rng.u32Draw(seeded) : static_cast<u32>(spec.ec);
        state.tid = spec.tid < 0 ? rng.u32Draw(seeded) : static_cast<u32>(spec.tid);

        state.pidDraws = 0;
        if (spec.pid == Spec8a::neverShiny)
        {
            u32 pid = rng.u32Draw(seeded);
            state.pidDraws = 1;
            state.pid = isShiny(pid, state.tid) ? pid ^ 0x10000000 : pid;
        }
        else if (spec.pid == Spec8a::alwaysShiny)
        {
            u32 pid = rng.u32Draw(seeded);
            state.pidDraws = 1;
            if (!isShiny(pid, state.tid))
            {
                pid = (pid & 0xFFFF) | ((((state.tid >> 16) ^ state.tid ^ pid) & 0xFFFF) << 16);
            }
            state.pid = pid;
        }
        else if (spec.pid == Spec8a::randomPID)
        {
            u32 pid = 0;
            for (u8 i = 0; i < (spec.rolls ? spec.rolls : 1); i++)
            {
                pid = rng.u32Draw(seeded);
                state.pidDraws++;
                if (isShiny(pid, state.tid))
                {
                    break;
                }
            }
            state.pid = pid;
        }
        else
        {
            state.pid = static_cast<u32>(spec.pid);
        }
        state.shiny = isShiny(state.pid, state.tid);

        std::array<s8, 6> ivs = spec.ivs;
        if (spec.flawless > 0)
        {
            if (spec.flawless < 6)
            {
                ivs = { -1, -1, -1, -1, -1, -1 };
                for (u8 i = 0; i < spec.flawless; i++)
                {
                    u32 index;
                    do
                    {
                        index = rng.rand(6, seeded);
                    } while (ivs[index] != -1);
                    ivs[index] = 31;
                }
            }
            else
            {
                ivs = { -1, -1, -1, -1, -1, -1 };
            }
        }
        for (int i = 0; i < 6; i++)
        {
            state.ivs[i] = ivs[i] == -1 ? static_cast<u8>(rng.rand(32, seeded)) : static_cast<u8>(ivs[i]);
        }

        if (spec.ability == 0xFE)
        {
            state.ability = static_cast<u8>(rng.rand(3, seeded));
        }
        else if (spec.ability == 0xFF)
        {
            state.ability = static_cast<u8>(rng.rand(2, seeded));
        }
        else
        {
            state.ability = spec.ability;
        }

        state.form = spec.form;
        if (spec.form == 0xFF)
        {
            state.form = spec.formCount ? static_cast<u16>(rng.rand(spec.formCount, seeded)) : static_cast<u16>(rng.u32Draw(seeded));
        }

        u8 ratio = spec.genderRatio;
        u8 gender = spec.gender;
        if (gender == 0xFF)
        {
            if (ratio == 0)
            {
                gender = 0;
            }
            else if (ratio == 255)
            {
                gender = 2;
            }
            else if (ratio == 254)
            {
                gender = 1;
            }
            else
            {
                // 0x2B8C3AC: rand(253), not 252 as in PKHeX's Overworld8aRNG (proven live)
                gender = rng.rand(253, seeded) + 1 < ratio ? 1 : 0;
            }
        }
        // 0x2B8B41C: the species ratio wins over the spec
        state.gender = ratio == 255 ? 2 : ratio == 254 ? 1 : ratio == 0 ? 0 : (gender < 2 ? gender : 0);

        state.nature = spec.nature < 0 ? static_cast<u8>(rng.rand(25, seeded)) : static_cast<u8>(spec.nature);
        state.height = size(spec.height, rng, seeded);
        state.weight = size(spec.weight, rng, seeded);
        state.draws = rng.draws - start;
        return state;
    }
}
