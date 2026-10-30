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

#include "PelagoGenerator7.hpp"
#include <Core/Enum/Method.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/Parents/States/State.hpp>
#include <Core/RNG/SFMT.hpp>
#include <Core/Util/Utilities.hpp>

PelagoGenerator7::PelagoGenerator7(u32 initialAdvances, u32 maxAdvances, u16 specie, u8 level, const Profile7 &profile,
                                   const StateFilter &filter) :
    Generator(initialAdvances, maxAdvances, 0, Method::None, profile, filter), specie(specie), level(level)
{
}

std::vector<GeneratorState> PelagoGenerator7::generate(u32 seed) const
{
    const PersonalInfo *info = PersonalLoader::getPersonal(profile.getVersion(), specie);
    u16 tsv = profile.getTID() ^ profile.getSID();
    u8 ratio = info->getGender();
    u8 rolls = profile.getShinyCharm() ? 3 : 1;

    SFMT base(seed, initialAdvances);
    std::vector<GeneratorState> states;
    for (u32 cnt = 0; cnt <= maxAdvances; cnt++, base.next())
    {
        SFMT rng = base;
        auto r32 = [&rng] { return static_cast<u32>(rng.next()); };

        u32 ec = r32();

        // The visitor is built with its OT left unset, so the fill draws an OT ID that the game then replaces with the player's
        r32();

        u32 pid = 0;
        for (u8 roll = 0; roll < rolls; roll++)
        {
            pid = r32();
            if (Utilities::isShiny<false>(pid, tsv))
            {
                break;
            }
        }

        std::array<u8, 6> ivs;
        for (u8 &iv : ivs)
        {
            iv = static_cast<u8>(rng.next() % 32);
        }

        u8 ability = static_cast<u8>(rng.next() % 2);
        u8 nature = static_cast<u8>(rng.next() % 25);

        u8 gender;
        if (ratio == 255)
        {
            gender = 2;
        }
        else if (ratio == 254)
        {
            gender = 1;
        }
        else if (ratio == 0)
        {
            gender = 0;
        }
        else
        {
            gender = (rng.next() % 252) + 1 < ratio ? 1 : 0;
        }

        GeneratorState state(initialAdvances + cnt, ec, pid, ivs, ability, gender, level, nature, Utilities::getShiny<false>(pid, tsv), info);
        if (filter.compare(state))
        {
            states.emplace_back(state);
        }
    }
    return states;
}
