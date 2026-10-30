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

#include "EggGenerator6.hpp"
#include <Core/Enum/Method.hpp>
#include <Core/Gen6/Pokemon6.hpp>
#include <Core/Gen6/States/EggState6.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/RNG/MT.hpp>
#include <Core/RNG/RNGList.hpp>
#include <Core/Util/Utilities.hpp>

namespace
{
    /**
     * @brief Plain MT19937 seeded with init_by_array, as the game does for each egg
     */
    class EggMT
    {
    public:
        EggMT(u32 key0, u32 key1) : index(624)
        {
            state[0] = 19650218;
            for (u32 i = 1; i < 624; i++)
            {
                state[i] = 0x6c078965 * (state[i - 1] ^ (state[i - 1] >> 30)) + i;
            }

            const u32 key[2] = { key0, key1 };
            u32 i = 1;
            u32 j = 0;
            for (u32 k = 624; k > 0; k--)
            {
                state[i] = (state[i] ^ ((state[i - 1] ^ (state[i - 1] >> 30)) * 1664525)) + key[j] + j;
                i++;
                j++;
                if (i >= 624)
                {
                    state[0] = state[623];
                    i = 1;
                }
                if (j >= 2)
                {
                    j = 0;
                }
            }
            for (u32 k = 623; k > 0; k--)
            {
                state[i] = (state[i] ^ ((state[i - 1] ^ (state[i - 1] >> 30)) * 1566083941)) - i;
                i++;
                if (i >= 624)
                {
                    state[0] = state[623];
                    i = 1;
                }
            }
            state[0] = 0x80000000;
        }

        u32 next()
        {
            if (index >= 624)
            {
                for (u32 k = 0; k < 624; k++)
                {
                    u32 y = (state[k] & 0x80000000) | (state[(k + 1) % 624] & 0x7fffffff);
                    state[k] = state[(k + 397) % 624] ^ (y >> 1) ^ ((y & 1) ? 0x9908b0df : 0);
                }
                index = 0;
            }

            u32 y = state[index++];
            y ^= y >> 11;
            y ^= (y << 7) & 0x9d2c5680;
            y ^= (y << 15) & 0xefc60000;
            y ^= y >> 18;
            return y;
        }

        u32 rand(u32 n)
        {
            return Gen6::rand(next(), n);
        }

        bool rand2()
        {
            return next() < 0x80000000;
        }

    private:
        u32 state[624];
        u32 index;
    };

    bool isPowerItem(u8 item)
    {
        return item >= 2 && item <= 7;
    }
}

EggGenerator6::EggGenerator6(u32 initialAdvances, u32 maxAdvances, const Daycare &daycare, const Profile6 &profile,
                             const StateFilter &filter) :
    EggGenerator(initialAdvances, maxAdvances, 0, Method::None, daycare, profile, filter),
    pidRolls((daycare.getMasuda() ? 6 : 0) + (profile.getShinyCharm() ? 2 : 0))
{
}

EggState6 EggGenerator6::generate(u64 seed) const
{
    return generate(seed, 0, 0);
}

std::vector<EggState6> EggGenerator6::generateChain(u32 seed) const
{
    bool mainPID = pidRolls == 0;
    RNGList<u32, MT, 4> rngList(seed, initialAdvances);

    std::vector<EggState6> states;
    for (u32 cnt = 0; cnt <= maxAdvances; cnt++, rngList.advanceState())
    {
        // Without Charm/Masuda the PID of the egg being accepted is drawn first [3DSRNGTool], then the next seed, low word first
        u32 pid = mainPID ? rngList.next() : 0;
        u32 low = rngList.next();
        u32 high = rngList.next();

        EggState6 state = generate((static_cast<u64>(high) << 32) | low, initialAdvances + cnt, pid);
        if (filter.compare(static_cast<const State &>(state)))
        {
            states.emplace_back(state);
        }
    }

    return states;
}

EggState6 EggGenerator6::generate(u64 seed, u32 advances, u32 mainPID) const
{
    u16 tsv = profile.getTID() ^ profile.getSID();
    u16 specie = daycare.getEggSpecie();
    const PersonalInfo *info = PersonalLoader::getPersonal(profile.getVersion(), specie);

    EggMT rng(seed & 0xffffffff, seed >> 32);

    // Nidoran and Volbeat/Illumise flip a coin for the species
    u8 gender;
    if (specie == 29 || specie == 32 || specie == 313 || specie == 314)
    {
        bool male = rng.rand2();
        gender = male ? 0 : 1;
        if (specie == 29 || specie == 32)
        {
            info = PersonalLoader::getPersonal(profile.getVersion(), male ? 32 : 29);
        }
        else
        {
            info = PersonalLoader::getPersonal(profile.getVersion(), male ? 313 : 314);
        }
    }
    else
    {
        switch (info->getGender())
        {
        case 255:
            gender = 2;
            break;
        case 254:
            gender = 1;
            break;
        case 0:
            gender = 0;
            break;
        default:
            gender = rng.rand(252) + 1 < info->getGender();
            break;
        }
    }

    u8 nature = rng.rand(25);
    if (daycare.getEverstoneCount() == 2)
    {
        nature = daycare.getParentNature(rng.rand2() ? 0 : 1);
    }
    else if (daycare.getParentItem(0) == 1)
    {
        nature = daycare.getParentNature(0);
    }
    else if (daycare.getParentItem(1) == 1)
    {
        nature = daycare.getParentNature(1);
    }

    // Ability comes from the female, or from the non-Ditto parent
    u8 parentAbility = daycare.getParentAbility(daycare.getParentGender(1) == 3 ? 0 : 1);
    u8 ability = rng.rand(100);
    if (parentAbility == 2)
    {
        ability = ability < 20 ? 0 : ability < 40 ? 1 : 2;
    }
    else if (parentAbility == 1)
    {
        ability = ability < 20 ? 0 : 1;
    }
    else
    {
        ability = ability < 80 ? 0 : 1;
    }

    // Inheritance: 1 = parent 0, 2 = parent 1
    std::array<u8, 6> inheritance = { 0, 0, 0, 0, 0, 0 };
    u8 item0 = daycare.getParentItem(0);
    u8 item1 = daycare.getParentItem(1);
    bool power = isPowerItem(item0) || isPowerItem(item1);
    if (isPowerItem(item0) && isPowerItem(item1))
    {
        if (rng.rand2())
        {
            inheritance[item0 - 2] = 1;
        }
        else
        {
            inheritance[item1 - 2] = 2;
        }
    }
    else if (isPowerItem(item0))
    {
        inheritance[item0 - 2] = 1;
    }
    else if (isPowerItem(item1))
    {
        inheritance[item1 - 2] = 2;
    }

    u8 inheritanceCount = (item0 == 8 || item1 == 8) ? 5 : 3;
    for (u8 i = power ? 1 : 0; i < inheritanceCount; i++)
    {
        u8 index;
        do
        {
            index = rng.rand(6);
        } while (inheritance[index] != 0);
        inheritance[index] = rng.rand2() ? 1 : 2;
    }

    std::array<u8, 6> ivs;
    for (u8 i = 0; i < 6; i++)
    {
        u8 iv = rng.next() >> 27;
        if (inheritance[i] == 1)
        {
            iv = daycare.getParentIV(0, i);
        }
        else if (inheritance[i] == 2)
        {
            iv = daycare.getParentIV(1, i);
        }
        ivs[i] = iv;
    }

    u32 ec = rng.next();

    u32 pid = mainPID;
    for (u8 roll = 0; roll < pidRolls; roll++)
    {
        pid = rng.next();
        if (Utilities::isShiny<false>(pid, tsv))
        {
            break;
        }
    }

    return EggState6(advances, ec, pid, ivs, ability, gender, nature, Utilities::getShiny<false>(pid, tsv), inheritance, seed,
                     pidRolls == 0, info);
}
