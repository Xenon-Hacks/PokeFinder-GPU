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

#include "EventGenerator6.hpp"
#include <Core/Enum/Method.hpp>
#include <Core/Gen6/States/State6.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/RNG/MT.hpp>
#include <Core/RNG/RNGList.hpp>

EventGenerator6::EventGenerator6(u32 initialAdvances, u32 maxAdvances, u32 delay, const WC6 &wc6, const Profile6 &profile,
                                 const StateFilter &filter, bool generateTwice) :
    Generator(initialAdvances, maxAdvances, delay, Method::None, profile, filter), wc6(wc6), generateTwice(generateTwice)
{
}

std::vector<State6> EventGenerator6::generate(u32 seed) const
{
    const PersonalInfo *info = PersonalLoader::getPersonal(profile.getVersion(), wc6.getSpecie(), wc6.getForm());

    Gen6::Request6 request;
    request.ivs = wc6.getIVs();
    request.ivCount = wc6.getIVCount();
    request.pidType = wc6.getPIDType();
    request.fixedPID = wc6.getPID();
    request.hasEC = wc6.getEC() != 0;
    request.fixedEC = wc6.getEC();
    switch (wc6.getAbility())
    {
    case 3:
        request.ability = Gen6::AbilityRandom;
        break;
    case 4:
        request.ability = Gen6::AbilityRandomHidden;
        break;
    default:
        request.ability = wc6.getAbility();
        break;
    }
    request.nature = wc6.getNature();
    if (wc6.getGender() <= 2)
    {
        request.gender = wc6.getGender();
    }

    u16 tid = wc6.getOwnOT() ? profile.getTID() : wc6.getTID();
    u16 sid = wc6.getOwnOT() ? profile.getSID() : wc6.getSID();

    RNGList<u32, MT, 128> rngList(seed, initialAdvances + offset);

    std::vector<State6> states;
    for (u32 cnt = 0; cnt <= maxAdvances; cnt++, rngList.advanceState())
    {
        if (generateTwice)
        {
            // The first generation is a throwaway block of the same shape
            Gen6::generate(rngList, request, info, tid, sid);
        }
        auto result = Gen6::generate(rngList, request, info, tid, sid);

        State6 state(initialAdvances + cnt, result, wc6.getLevel(), 0, info);
        if (filter.compare(state))
        {
            states.emplace_back(state);
        }
    }

    return states;
}
