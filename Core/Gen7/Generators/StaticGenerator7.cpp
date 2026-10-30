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

#include "StaticGenerator7.hpp"
#include <Core/Enum/Lead.hpp>
#include <Core/Enum/Method.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/RNG/SFMT.hpp>
#include <Core/Util/Utilities.hpp>

StaticGenerator7::StaticGenerator7(u32 initialAdvances, u32 maxAdvances, u8 blinkDraws, Lead lead, const StaticTemplate7 &staticTemplate,
                                   const Profile7 &profile, const StateFilter &filter) :
    StaticGenerator(initialAdvances, maxAdvances, 0, Method::None, lead, staticTemplate, profile, filter), blinkDraws(blinkDraws)
{
}

std::vector<GeneratorState> StaticGenerator7::generate(u32 seed) const
{
    const PersonalInfo *info = staticTemplate.getInfo();
    u16 tsv = profile.getTID() ^ profile.getSID();
    StaticKind7 kind = staticTemplate.getKind();
    bool synchronize = lead <= Lead::SynchronizeEnd;

    SFMT base(seed, initialAdvances);
    std::vector<GeneratorState> states;
    for (u32 cnt = 0; cnt <= maxAdvances; cnt++, base.next())
    {
        SFMT rng = base;
        auto r32 = [&rng] { return static_cast<u32>(rng.next()); };

        bool natureFixed = staticTemplate.getNature() != 255;
        u8 nature = staticTemplate.getNature();
        std::array<u8, 6> ivs = staticTemplate.getIVs();
        u32 ec;
        u32 pid;
        u8 ability = staticTemplate.getAbility();
        u8 gender = staticTemplate.getGender();

        if (kind == StaticKind7::Trade)
        {
            // rand(32) per unset IV in HP, Atk, Def, SpA, SpD, Spe order, then EC and a PID locked against the trade's OT
            for (u8 &iv : ivs)
            {
                if (iv == 255)
                {
                    iv = static_cast<u8>(rng.next() % 32);
                }
            }
            ec = r32();
            pid = r32();
            if (Utilities::isShiny<false>(pid, staticTemplate.getTradeTSV()))
            {
                pid ^= 0x10000000;
            }
        }
        else
        {
            if (kind == StaticKind7::Legend || kind == StaticKind7::Totem)
            {
                // The static rule: lead roll, the on-screen models' blinks, 6 throwaway Pokemon
                u8 leadRoll = static_cast<u8>(rng.next() % 100);
                if (synchronize && leadRoll >= 50)
                {
                    natureFixed = true;
                    nature = toInt(lead);
                }
                rng.advance(blinkDraws + 60);
            }
            else if (synchronize)
            {
                // Gifts take the lead's nature with no roll
                natureFixed = true;
                nature = toInt(lead);
            }

            ec = r32();
            if (kind == StaticKind7::WildGift)
            {
                u8 rolls = profile.getShinyCharm() ? 3 : 1;
                for (u8 roll = 0; roll < rolls; roll++)
                {
                    pid = r32();
                    if (Utilities::isShiny<false>(pid, tsv))
                    {
                        break;
                    }
                }
            }
            else
            {
                pid = r32();
                if (Utilities::isShiny<false>(pid, tsv))
                {
                    pid ^= 0x10000000;
                }
            }

            for (u8 i = 0; i < staticTemplate.getIVCount();)
            {
                u8 index = static_cast<u8>(rng.next() % 6);
                if (ivs[index] == 255)
                {
                    ivs[index] = 31;
                    i++;
                }
            }
            for (u8 &iv : ivs)
            {
                if (iv == 255)
                {
                    iv = static_cast<u8>(rng.next() % 32);
                }
            }

            if (staticTemplate.getAbilityDraw())
            {
                ability = static_cast<u8>(rng.next() % 2);
            }
            if (!natureFixed)
            {
                nature = static_cast<u8>(rng.next() % 25);
            }
            if (gender == 255)
            {
                u8 ratio = info->getGender();
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
            }
        }

        if (gender == 255)
        {
            gender = 2;
        }
        u8 shiny = Utilities::getShiny<false>(pid, tsv);
        GeneratorState state(initialAdvances + cnt, ec, pid, ivs, ability, gender, staticTemplate.getLevel(), nature, shiny, info);
        if (filter.compare(state))
        {
            states.emplace_back(state);
        }
    }
    return states;
}
