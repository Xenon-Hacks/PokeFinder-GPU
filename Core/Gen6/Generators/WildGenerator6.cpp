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
#include <Core/Gen6/Pokemon6.hpp>
#include <Core/Gen6/States/State6.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/RNG/MT.hpp>
#include <Core/RNG/RNGList.hpp>
#include <algorithm>
#include <array>

WildGenerator6::WildGenerator6(u32 initialAdvances, u32 maxAdvances, u32 delay, Encounter6 encounter, Lead lead,
                               const std::vector<Slot> &targets, u8 haMember, u8 ivCount, bool shinyLocked, const Profile6 &profile,
                               const StateFilter &filter) :
    Generator(initialAdvances, maxAdvances, delay, Method::None, profile, filter),
    targets(targets),
    encounter(encounter),
    lead(lead),
    haMember(haMember),
    ivCount(ivCount),
    shinyLocked(shinyLocked)
{
}

std::vector<State6> WildGenerator6::generate(u32 seed) const
{
    u8 members = isHorde(encounter) ? 5 : 1;

    std::vector<Gen6::Request6> requests(members);
    for (u8 member = 0; member < members; member++)
    {
        Gen6::Request6 &request = requests[member];
        const PersonalInfo *info = targets[member].getInfo();

        request.pidType = shinyLocked ? Gen6::PIDType::Locked : Gen6::PIDType::Random;
        request.pidRolls = getPIDRolls(encounter, profile.getShinyCharm(), shinyLocked);
        request.ivCount = ivCount;
        // The hidden-ability horde member is fixed to the hidden ability and draws nothing for it (live X)
        request.ability = (members == 5 && member + 1 == haMember) ? 2 : Gen6::AbilityRandom;
        if (lead <= Lead::SynchronizeEnd)
        {
            request.nature = toInt(lead);
        }
        else if ((lead == Lead::CuteCharmF || lead == Lead::CuteCharmM) && !info->getFixedGender())
        {
            request.gender = lead == Lead::CuteCharmF ? 0 : 1;
        }
    }

    // A horde Pokemon shows as every member: each member starts 13 outputs (11 without the Shiny Charm) after the one before,
    // so the target only exists on 5 short runs of Advances. Generate span Advances on both sides to find all of them.
    constexpr u32 span = 96;
    struct Key
    {
        u32 pid;
        std::array<u8, 6> ivs;
    };
    std::array<std::array<Key, 5>, 256> history;
    std::vector<size_t> pending;
    size_t resolved = 0;

    bool horde = members == 5;
    u32 before = horde ? std::min(initialAdvances, span) : 0;
    u32 after = horde ? span : 0;
    u32 start = initialAdvances - before;

    RNGList<u32, MT, 256> rngList(seed, start + offset + 60);

    std::vector<State6> states;
    for (u32 cnt = 0; cnt <= before + maxAdvances + after; cnt++, rngList.advanceState())
    {
        u32 advance = start + cnt;
        bool inRange = cnt >= before && cnt - before <= maxAdvances;
        for (u8 member = 0; member < members; member++)
        {
            const Slot &slot = targets[member];
            auto result = Gen6::generate(rngList, requests[member], slot.getInfo(), profile.getTID(), profile.getSID());
            for (u8 i = 0; i < potential && i < 6; i++)
            {
                result.ivs[i] = 31;
            }
            if (horde)
            {
                history[advance & 255][member] = { result.pid, result.ivs };
            }

            if (inRange)
            {
                State6 state(advance, result, slot.getMaxLevel(), horde ? member + 1 : 0, slot.getInfo());
                if (filter.compare(state))
                {
                    if (horde)
                    {
                        pending.emplace_back(states.size());
                    }
                    states.emplace_back(state);
                }
            }
        }

        for (; resolved < pending.size() && states[pending[resolved]].getAdvances() + span <= advance; resolved++)
        {
            State6 &state = states[pending[resolved]];
            u32 hit = state.getAdvances();
            for (u32 x = hit - std::min(hit - start, span); x <= hit + span; x++)
            {
                for (u8 member = 0; member < members; member++)
                {
                    const Key &key = history[x & 255][member];
                    if (key.pid == state.getPID() && key.ivs == state.getIVs())
                    {
                        state.addSpot(member + 1, x);
                    }
                }
            }
        }
    }

    return states;
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
