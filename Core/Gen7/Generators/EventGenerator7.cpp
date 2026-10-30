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

#include "EventGenerator7.hpp"
#include <Core/Enum/Method.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/Parents/States/State.hpp>
#include <Core/RNG/SFMT.hpp>
#include <Core/Util/Utilities.hpp>

EventGenerator7::EventGenerator7(u32 initialAdvances, u32 maxAdvances, u8 blocksBefore, const WC7 &wc7, const Profile7 &profile,
                                 const StateFilter &filter) :
    Generator(initialAdvances, maxAdvances, 0, Method::None, profile, filter), wc7(wc7), blocksBefore(blocksBefore)
{
}

std::vector<GeneratorState> EventGenerator7::generate(u32 seed) const
{
    const PersonalInfo *info = PersonalLoader::getPersonal(profile.getVersion(), wc7.getSpecie(), wc7.getForm());
    u16 tid = wc7.getOwnOT() ? profile.getTID() : wc7.getTID();
    u16 sid = wc7.getOwnOT() ? profile.getSID() : wc7.getSID();
    u16 tsv = tid ^ sid;

    struct Block
    {
        u32 ec;
        u32 pid;
        std::array<u8, 6> ivs;
        u8 ability;
        u8 gender;
        u8 level;
        u8 nature;
    };

    // MysteryGiftSave::CreatePokemonParamFromGiftData, then the shared fill: no lead roll, no throwaways
    auto block = [&](SFMT &rng) {
        Block result;
        auto r32 = [&rng] { return static_cast<u32>(rng.next()); };

        result.level = wc7.getLevel();
        if (result.level == 0)
        {
            result.level = static_cast<u8>(rng.next() % 100) + 1;
        }

        result.ec = wc7.getEC() != 0 ? wc7.getEC() : r32();

        switch (wc7.getPIDType())
        {
        case Gen7::PIDType7::Fixed:
            result.pid = wc7.getPID();
            break;
        case Gen7::PIDType7::Shiny:
        {
            // The low half is kept, the high half becomes low ^ TID ^ SID: always a square shiny
            u32 low = r32() & 0xffff;
            result.pid = ((low ^ tid ^ sid) << 16) | low;
            break;
        }
        case Gen7::PIDType7::Locked:
            result.pid = r32();
            if (Utilities::isShiny<false>(result.pid, tsv))
            {
                result.pid ^= 0x10000000;
            }
            break;
        default:
            result.pid = r32();
            break;
        }

        result.ivs = wc7.getIVs();
        for (u8 i = 0; i < wc7.getIVCount();)
        {
            u8 index = static_cast<u8>(rng.next() % 6);
            if (result.ivs[index] == Gen7::Random)
            {
                result.ivs[index] = 31;
                i++;
            }
        }
        for (u8 &iv : result.ivs)
        {
            if (iv == Gen7::Random)
            {
                iv = static_cast<u8>(rng.next() % 32);
            }
        }

        switch (wc7.getAbility())
        {
        case Gen7::AbilityRandom:
            result.ability = static_cast<u8>(rng.next() % 2);
            break;
        case Gen7::AbilityRandomHidden:
            result.ability = static_cast<u8>(rng.next() % 3);
            break;
        default:
            result.ability = wc7.getAbility();
            break;
        }

        result.nature = wc7.getNature();
        if (result.nature == Gen7::Random)
        {
            result.nature = static_cast<u8>(rng.next() % 25);
        }

        result.gender = wc7.getGender();
        if (result.gender == 3)
        {
            u8 ratio = info->getGender();
            if (ratio == 255)
            {
                result.gender = 2;
            }
            else if (ratio == 254)
            {
                result.gender = 1;
            }
            else if (ratio == 0)
            {
                result.gender = 0;
            }
            else
            {
                result.gender = (rng.next() % 252) + 1 < ratio ? 1 : 0;
            }
        }

        return result;
    };

    SFMT base(seed, initialAdvances);
    std::vector<GeneratorState> states;
    for (u32 cnt = 0; cnt <= maxAdvances; cnt++, base.next())
    {
        SFMT rng = base;
        for (u8 i = 0; i < blocksBefore; i++)
        {
            block(rng);
        }
        Block result = block(rng);

        GeneratorState state(initialAdvances + cnt, result.ec, result.pid, result.ivs, result.ability, result.gender, result.level,
                             result.nature, Utilities::getShiny<false>(result.pid, tsv), info);
        if (filter.compare(state))
        {
            states.emplace_back(state);
        }
    }
    return states;
}
