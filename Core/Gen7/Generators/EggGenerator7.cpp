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

#include "EggGenerator7.hpp"
#include <Core/Enum/Method.hpp>
#include <Core/Gen7/States/EggState7.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/RNG/SFMT.hpp>
#include <Core/Util/Utilities.hpp>

namespace
{
    /**
     * @brief The game's gfl2::math::Random (TinyMT32, mat1 8F7011EE, mat2 FC78FF1F, tmat 3793FDFF) restored from a raw 16-byte state,
     * as EggAdopt does from the save; Next(n) is the tempered output % n
     */
    class EggTiny
    {
    public:
        EggTiny(const std::array<u32, 4> &state) : s(state), draws(0)
        {
        }

        u32 next()
        {
            draws++;
            u32 y = s[3];
            u32 x = (s[0] & 0x7fffffff) ^ s[1] ^ s[2];
            x ^= x << 1;
            y ^= (y >> 1) ^ x;
            s[0] = s[1];
            s[1] = s[2];
            s[2] = x ^ (y << 10);
            s[3] = y;
            if (y & 1)
            {
                s[1] ^= 0x8f7011ee;
                s[2] ^= 0xfc78ff1f;
            }

            u32 t1 = s[0] + (s[2] >> 8);
            u32 t0 = s[3] ^ t1;
            if (t1 & 1)
            {
                t0 ^= 0x3793fdff;
            }
            return t0;
        }

        u32 rand(u32 n)
        {
            return next() % n;
        }

        const std::array<u32, 4> &getState() const
        {
            return s;
        }

        u8 getDraws() const
        {
            return draws;
        }

    private:
        std::array<u32, 4> s;
        u8 draws;
    };

    bool isPowerItem(u8 item)
    {
        return item >= 2 && item <= 7;
    }
}

EggGenerator7::EggGenerator7(const Daycare &daycare, bool sameSpecies, const Profile7 &profile, const StateFilter &filter) :
    EggGenerator(0, 0, 0, Method::None, daycare, profile, filter),
    pidRolls((daycare.getMasuda() ? 6 : 0) + (profile.getShinyCharm() ? 2 : 0)),
    sameSpecies(sameSpecies)
{
}

EggState7 EggGenerator7::generate(const std::array<u32, 4> &state) const
{
    return generate(state, 0, 0);
}

std::array<u32, 4> EggGenerator7::reject(std::array<u32, 4> state, u32 count)
{
    EggTiny rng(state);
    for (u32 i = 0; i < count; i++)
    {
        rng.next();
    }
    return rng.getState();
}

std::vector<EggState7> EggGenerator7::generateChain(std::array<u32, 4> state, u32 rejects, u32 count) const
{
    state = reject(state, rejects);

    std::vector<EggState7> states;
    for (u32 i = 0; i < count; i++)
    {
        EggState7 egg = generate(state, i + 1, 0);
        state = egg.getState();
        if (filter.compare(static_cast<const State &>(egg)))
        {
            states.emplace_back(egg);
        }
    }
    return states;
}

std::vector<EggState7> EggGenerator7::generateMainPID(const std::array<u32, 4> &state, u32 seed, u32 initialAdvances,
                                                      u32 maxAdvances) const
{
    SFMT rng(seed, initialAdvances);

    std::vector<EggState7> states;
    for (u32 cnt = 0; cnt <= maxAdvances; cnt++)
    {
        // The PID is the low word of the u64 drawn when the egg is accepted
        u32 pid = static_cast<u32>(rng.next());
        EggState7 egg = generate(state, initialAdvances + cnt, pid);
        if (filter.compare(static_cast<const State &>(egg)))
        {
            states.emplace_back(egg);
        }
    }
    return states;
}

EggState7 EggGenerator7::generate(const std::array<u32, 4> &state, u32 advances, u32 mainPID) const
{
    u16 tsv = profile.getTID() ^ profile.getSID();
    u16 specie = daycare.getEggSpecie();
    const PersonalInfo *info = PersonalLoader::getPersonal(profile.getVersion(), specie);

    EggTiny rng(state);

    // Nidoran and Volbeat/Illumise flip a coin for the species in place of the gender draw: 0 is the male line
    u8 gender;
    if (specie == 29 || specie == 32 || specie == 313 || specie == 314)
    {
        bool male = rng.rand(2) == 0;
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

    // The nature is always drawn; an Everstone then overrides it, two Everstones pick the parent with rand(2)
    u8 nature = rng.rand(25);
    if (daycare.getEverstoneCount() == 2)
    {
        nature = daycare.getParentNature(rng.rand(2) == 0 ? 0 : 1);
    }
    else if (daycare.getParentItem(0) == 1)
    {
        nature = daycare.getParentNature(0);
    }
    else if (daycare.getParentItem(1) == 1)
    {
        nature = daycare.getParentNature(1);
    }

    // Ability follows the mother role: the female, or the non-Ditto parent
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

    // Inheritance: 1 = parent 0 (father role), 2 = parent 1 (mother role)
    std::array<u8, 6> inheritance = { 0, 0, 0, 0, 0, 0 };
    u8 item0 = daycare.getParentItem(0);
    u8 item1 = daycare.getParentItem(1);
    bool power = isPowerItem(item0) || isPowerItem(item1);
    if (isPowerItem(item0) && isPowerItem(item1))
    {
        if (rng.rand(2) == 0)
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
        // A taken stat re-rolls only the stat
        u8 index;
        do
        {
            index = rng.rand(6);
        } while (inheritance[index] != 0);
        inheritance[index] = rng.rand(2) == 0 ? 1 : 2;
    }

    std::array<u8, 6> ivs;
    for (u8 i = 0; i < 6; i++)
    {
        u8 iv = rng.rand(32);
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

    // Masuda 6 rolls, Shiny Charm +2, each stops at a shiny; with no rolls the PID is the main RNG draw at the accept
    u32 pid = mainPID;
    for (u8 roll = 0; roll < pidRolls; roll++)
    {
        pid = rng.next();
        if (Utilities::isShiny<false>(pid, tsv))
        {
            break;
        }
    }

    // Same-species parents roll rand(100) + 1 <= 50 for the mother's ball, else the father's (the ball itself is not tracked)
    if (sameSpecies)
    {
        rng.rand(100);
    }

    u8 draws = rng.getDraws();

    // Accepting runs EggAdopt's Next() and EggClear's Next() on top of the egg's draws
    rng.next();
    rng.next();

    return EggState7(advances, ec, pid, ivs, ability, gender, nature, Utilities::getShiny<false>(pid, tsv), inheritance, rng.getState(),
                     draws, pidRolls == 0, info);
}
