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

#ifndef WC7_HPP
#define WC7_HPP

#include <Core/Global.hpp>
#include <array>

namespace Gen7
{
    /**
     * @brief Wonder Card PID type (card + 0x3B): how the game's generator treats the PID
     */
    enum class PIDType7 : u8
    {
        Fixed = 0, ///< The card's PID, no draw
        Random = 1, ///< One random draw, can be shiny
        Shiny = 2, ///< One random draw forced square shiny against the OT
        Locked = 3 ///< One random draw, a shiny result is turned non-shiny with PID ^= 0x10000000
    };

    constexpr u8 AbilityRandom = 3; ///< rand(2), slot 0 or 1
    constexpr u8 AbilityRandomHidden = 4; ///< rand(3), slot 0, 1 or hidden
    constexpr u8 Random = 255; ///< Draw the field (IV, nature); gender uses 3
}

/**
 * @brief Gen 7 Wonder Card: the fields of the gift data the game builds the Pokemon from (same layout as a .wc6; the gift data starts at
 * file offset 0x68)
 */
class WC7
{
public:
    static constexpr u32 size = 0x108; ///< Size of a .wc7 file

    /**
     * @brief Construct a new WC7 object
     *
     * @param specie Pokemon specie
     * @param form Pokemon form
     * @param level Pokemon level, 0 for rand(100) + 1
     * @param tid Card OT trainer ID
     * @param sid Card OT secret ID
     * @param ownOT Whether the Pokemon takes the receiving player's OT (card OT gender 3)
     * @param ec Fixed EC, 0 for random
     * @param pidType PID type
     * @param pid Fixed PID for Gen7::PIDType7::Fixed
     * @param ivs IVs in HP/Atk/Def/SpA/SpD/Spe order, Gen7::Random for random
     * @param ivCount Guaranteed perfect IVs
     * @param ability 0/1/2 fixed slot, 3 random 0/1, 4 random 0/1/H
     * @param nature Fixed nature, Gen7::Random for random
     * @param gender 0/1/2 fixed gender, 3 for random
     * @param egg Whether the gift is an egg
     */
    WC7(u16 specie, u8 form, u8 level, u16 tid, u16 sid, bool ownOT, u32 ec, Gen7::PIDType7 pidType, u32 pid, const std::array<u8, 6> &ivs,
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
     * @brief Parses a .wc7 file
     *
     * @param data Pointer to 0x108 bytes of card data
     *
     * @return Parsed card
     */
    static WC7 parse(const u8 *data)
    {
        auto u16le = [data](u32 offset) { return static_cast<u16>(data[offset] | (data[offset + 1] << 8)); };
        auto u32le = [data](u32 offset) {
            return static_cast<u32>(data[offset]) | (static_cast<u32>(data[offset + 1]) << 8) | (static_cast<u32>(data[offset + 2]) << 16)
                | (static_cast<u32>(data[offset + 3]) << 24);
        };

        // Card IV bytes are HP Atk Def Spe SpA SpD; > 31 is random, 0xFC-0xFE also sets 1-3 perfect IVs
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
                ivs[i] = Gen7::Random;
                if (value >= 0xfc && value <= 0xfe)
                {
                    ivCount = value - 0xfb;
                }
            }
        }

        u8 pidType = data[0xa3];
        if (pidType > 3)
        {
            pidType = 1;
        }

        u8 nature = data[0xa0];
        if (nature >= 25)
        {
            nature = Gen7::Random;
        }

        u8 ability = data[0xa2];
        if (ability > 4)
        {
            ability = Gen7::AbilityRandom;
        }

        return WC7(u16le(0x82), data[0x84], data[0xd0], u16le(0x68), u16le(0x6a), data[0xb5] == 3, u32le(0x70),
                   static_cast<Gen7::PIDType7>(pidType), u32le(0xd4), ivs, ivCount, ability, nature, data[0xa1], data[0xd1] == 1);
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

    u8 getIV(u8 index) const
    {
        return ivs[index];
    }

    u8 getIVCount() const
    {
        return ivCount;
    }

    const std::array<u8, 6> &getIVs() const
    {
        return ivs;
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

    Gen7::PIDType7 getPIDType() const
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
    Gen7::PIDType7 pidType;
    u8 ability;
    u8 form;
    u8 gender;
    u8 ivCount;
    u8 level;
    u8 nature;
    bool egg;
    bool ownOT;
};

#endif // WC7_HPP
