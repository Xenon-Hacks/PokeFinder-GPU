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

#include "StaticGenerator6.hpp"
#include <Core/Enum/Lead.hpp>
#include <Core/Enum/Method.hpp>
#include <Core/Gen6/States/State6.hpp>
#include <Core/RNG/MT.hpp>
#include <Core/RNG/RNGList.hpp>

StaticGenerator6::StaticGenerator6(u32 initialAdvances, u32 maxAdvances, u32 delay, Lead lead, const StaticTemplate6 &staticTemplate,
                                   const Profile6 &profile, const StateFilter &filter) :
    StaticGenerator(initialAdvances, maxAdvances, delay, Method::None, lead, staticTemplate, profile, filter)
{
}

Gen6::Request6 StaticGenerator6::getRequest(const StaticTemplate6 &staticTemplate, const Profile6 &profile, Lead lead)
{
    Gen6::Request6 request;
    bool locked = staticTemplate.getShiny() == Shiny::Never;
    request.pidType = locked ? Gen6::PIDType::Locked : Gen6::PIDType::Random;
    // Gifts (fossils) roll once even with the Shiny Charm, verified live on Y 1.5
    request.pidRolls = (locked || staticTemplate.getGift() || !profile.getShinyCharm()) ? 1 : 3;
    request.ivs = staticTemplate.getIVs();
    request.ivCount = staticTemplate.getIVCount();
    request.ability = staticTemplate.getAbility() == 255 ? Gen6::AbilityRandom : staticTemplate.getAbility();
    if (staticTemplate.getGender() != 255)
    {
        request.gender = staticTemplate.getGender();
    }
    if (staticTemplate.getNature() != 255)
    {
        request.nature = staticTemplate.getNature();
    }
    else if (lead <= Lead::SynchronizeEnd && !staticTemplate.getGift())
    {
        request.nature = toInt(lead);
    }
    return request;
}

std::vector<State6> StaticGenerator6::generate(u32 seed) const
{
    const PersonalInfo *info = staticTemplate.getInfo();
    Gen6::Request6 request = getRequest(staticTemplate, profile, lead);

    // Encounters run the 60-output container (6 dummy Pokemon) between the trigger and the EC; gifts and fossils do not
    u32 container = staticTemplate.getGift() ? 0 : 60;
    RNGList<u32, MT, 128> rngList(seed, initialAdvances + offset + container);

    std::vector<State6> states;
    for (u32 cnt = 0; cnt <= maxAdvances; cnt++, rngList.advanceState())
    {
        auto result = Gen6::generate(rngList, request, info, profile.getTID(), profile.getSID());

        State6 state(initialAdvances + cnt, result, staticTemplate.getLevel(), 0, info);
        if (filter.compare(state))
        {
            states.emplace_back(state);
        }
    }

    return states;
}
