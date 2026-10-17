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

#include "WildGenerator6.hpp"
#include <Core/Enum/Lead.hpp>
#include <Core/Enum/Method.hpp>
#include <Core/Gen6/States/State6.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/RNG/MT.hpp>
#include <Core/RNG/RNGList.hpp>

WildGenerator6::WildGenerator6(u32 initialAdvances, u32 maxAdvances, u32 delay, Encounter6 encounter, Lead lead, u16 specie, u8 level,
                               u8 ivCount, bool shinyLocked, const Profile6 &profile, const StateFilter &filter) :
    Generator(initialAdvances, maxAdvances, delay, Method::None, profile, filter),
    info(PersonalLoader::getPersonal(profile.getVersion(), specie)),
    encounter(encounter),
    lead(lead),
    ivCount(ivCount),
    level(level),
    shinyLocked(shinyLocked)
{
}

std::vector<State6> WildGenerator6::generate(u32 seed) const
{
    Gen6::Request6 request;
    request.pidType = shinyLocked ? Gen6::PIDType::Locked : Gen6::PIDType::Random;
    request.pidRolls = getPIDRolls(encounter, profile.getShinyCharm(), shinyLocked);
    request.ivCount = ivCount;
    request.ability = encounter == Encounter6::Horde ? Gen6::AbilityRandomHidden : Gen6::AbilityRandom;
    if (lead <= Lead::SynchronizeEnd)
    {
        request.nature = toInt(lead);
    }
    else if ((lead == Lead::CuteCharmF || lead == Lead::CuteCharmM) && !info->getFixedGender())
    {
        request.gender = lead == Lead::CuteCharmF ? 0 : 1;
    }

    u8 members = encounter == Encounter6::Horde ? 5 : 1;

    RNGList<u32, MT, 256> rngList(seed, initialAdvances + offset + 60);

    std::vector<State6> states;
    for (u32 cnt = 0; cnt <= maxAdvances; cnt++, rngList.advanceState())
    {
        for (u8 member = 0; member < members; member++)
        {
            auto result = Gen6::generate(rngList, request, info, profile.getTID(), profile.getSID());

            State6 state(initialAdvances + cnt, result, level, encounter == Encounter6::Horde ? member + 1 : 0, info);
            if (filter.compare(state))
            {
                states.emplace_back(state);
            }
        }
    }

    return states;
}

u32 WildGenerator6::getDefaultDelay(Encounter6 encounter)
{
    switch (encounter)
    {
    case Encounter6::RockSmash:
        return 274; // Yes press -> Tiny decision and 60 skip, live X 1.0
    case Encounter6::Horde:
        return 200; // Honey press -> 60 skip, live on Azahar (3DSRNGTool uses 174)
    default:
        return 0;
    }
}

u8 WildGenerator6::getPIDRolls(Encounter6 encounter, bool shinyCharm, bool shinyLocked)
{
    if (shinyLocked)
    {
        return 1;
    }

    u8 rolls = 1;
    if (encounter == Encounter6::FriendSafari)
    {
        rolls += 4;
    }
    if (shinyCharm)
    {
        rolls += 2;
    }
    return rolls;
}
