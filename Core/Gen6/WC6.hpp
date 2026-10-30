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

#ifndef WC6_HPP
#define WC6_HPP

#include <Core/Gen6/Pokemon6.hpp>
#include <Core/Global.hpp>
#include <array>
#include <cstring>

/**
 * @brief Wonder Card (WC6) fields that matter for RNG
 */
class WC6
{
public:
    static constexpr u32 size = 0x108; ///< Size of a .wc6 file

    /**
     * @brief Construct a new WC6 object
     *
     * @param specie Pokemon specie
     * @param form Pokemon form
     * @param level Pokemon level
     * @param tid Card OT trainer ID
     * @param sid Card OT secret ID
     * @param ownOT Whether the Pokemon takes the receiving player's OT (card OT gender 3)
     * @param ec Fixed EC, 0 for random
     * @param pidType PID type
     * @param pid Fixed PID for Gen6::PIDType::Fixed
     * @param ivs IVs (PokeFinder order), Gen6::Random for random
     * @param ivCount Guaranteed perfect IVs
     * @param ability 0/1/2 fixed slot, 3 random 0/1, 4 random 0/1/H
     * @param nature Fixed nature, Gen6::Random for random
     * @param gender 0/1/2 fixed gender, 3 for random
     * @param egg Whether the gift is an egg
     */
    WC6(u16 specie, u8 form, u8 level, u16 tid, u16 sid, bool ownOT, u32 ec, Gen6::PIDType pidType, u32 pid, const std::array<u8, 6> &ivs,
        u8 ivCount, u8 ability, u8 nature, u8 gender, bool egg) :
        ec(ec),
        pid(pid),
        ivs(ivs),
        sid(sid),
        specie(specie),
        tid(tid),
        pidType(pidType),
        ability(ability),
        form(form),
        gender(gender),
        ivCount(ivCount),
        level(level),
        nature(nature),
        egg(egg),
        ownOT(ownOT)
    {
    }

    /**
     * @brief Parses a .wc6 file
     *
     * @param data Pointer to 0x108 bytes of card data
     *
     * @return Parsed card
     */
    static WC6 parse(const u8 *data)
    {
        auto u16le = [data](u32 offset) { return static_cast<u16>(data[offset] | (data[offset + 1] << 8)); };
        auto u32le = [data](u32 offset) {
            return static_cast<u32>(data[offset]) | (static_cast<u32>(data[offset + 1]) << 8) | (static_cast<u32>(data[offset + 2]) << 16)
                | (static_cast<u32>(data[offset + 3]) << 24);
        };

        // Card IV bytes are HP Atk Def Spe SpA SpD; >31 is random, 0xFC-0xFE also sets 1-3 perfect IVs
        constexpr std::array<u8, 6> order = { 0, 1, 2, 4, 5, 3 };
        std::array<u8, 6> ivs;
        u8 ivCount = 0;
        for (u8 i = 0; i < 6; i++)
        {
            u8 value = data[0xaf + order[i]];
            if (value <= 31)
            {
                ivs[i] = value;
            }
            else
            {
                ivs[i] = Gen6::Random;
                if (value >= 0xfc && value <= 0xfe)
                {
                    ivCount = value - 0xfb;
                }
            }
        }

        // PKHeX ShinyType6: 0 fixed, 1 random, 2 always shiny, 3 never shiny
        Gen6::PIDType pidType;
        switch (data[0xa3])
        {
        case 0:
            pidType = Gen6::PIDType::Fixed;
            break;
        case 2:
            pidType = Gen6::PIDType::Shiny;
            break;
        case 3:
            pidType = Gen6::PIDType::Locked;
            break;
        default:
            pidType = Gen6::PIDType::Random;
            break;
        }

        u8 nature = data[0xa0];
        if (nature >= 25)
        {
            nature = Gen6::Random;
        }

        return WC6(u16le(0x82), data[0x84], data[0xd0], u16le(0x68), u16le(0x6a), data[0xb5] == 3, u32le(0x70), pidType, u32le(0xd4), ivs,
                   ivCount, data[0xa2], nature, data[0xa1], data[0xd1] == 1);
    }

    u8 getAbility() const
    {
        return ability;
    }

    u32 getEC() const
    {
        return ec;
    }

    bool getEgg() const
    {
        return egg;
    }

    u8 getForm() const
    {
        return form;
    }

    u8 getGender() const
    {
        return gender;
    }

    u8 getIV(int index) const
    {
        return ivs[index];
    }

    std::array<u8, 6> getIVs() const
    {
        return ivs;
    }

    u8 getIVCount() const
    {
        return ivCount;
    }

    u8 getLevel() const
    {
        return level;
    }

    u8 getNature() const
    {
        return nature;
    }

    bool getOwnOT() const
    {
        return ownOT;
    }

    u32 getPID() const
    {
        return pid;
    }

    Gen6::PIDType getPIDType() const
    {
        return pidType;
    }

    u16 getSID() const
    {
        return sid;
    }

    u16 getSpecie() const
    {
        return specie;
    }

    u16 getTID() const
    {
        return tid;
    }

private:
    u32 ec;
    u32 pid;
    std::array<u8, 6> ivs;
    u16 sid;
    u16 specie;
    u16 tid;
    Gen6::PIDType pidType;
    u8 ability;
    u8 form;
    u8 gender;
    u8 ivCount;
    u8 level;
    u8 nature;
    bool egg;
    bool ownOT;
};

#endif // WC6_HPP
