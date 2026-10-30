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

#ifndef POKEMON6_HPP
#define POKEMON6_HPP

#include <Core/Global.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/Util/Utilities.hpp>
#include <array>

/**
 * @brief Shared X/Y Pokemon generation from the main MT19937
 *
 * Every non-egg Pokemon in X/Y goes through one game function (Y 1.5 0x14F600) that fills a request struct. It draws, in order:
 * EC, PID (one output per roll), perfect IV slots with rand(6) re-rolled on repeats, the remaining IVs (u32 >> 27, HP Atk Def SpA SpD Spe),
 * ability, nature rand(25) and gender rand(252). A field the request fixes skips its draw. The encounter code burns 60 MT outputs
 * between the trigger frame and the call, so the EC is output trigger + 60. All of this was matched live on X 1.0 and Y 1.5.
 */
namespace Gen6
{
    /**
     * @brief Game rand(n): multiply-high for n < 0x10000, modulo otherwise
     *
     * @param value 32bit PRNG output
     * @param n Upper bound
     *
     * @return Value in [0, n)
     */
    constexpr u32 rand(u32 value, u32 n)
    {
        return n < 0x10000 ? static_cast<u32>((static_cast<u64>(value) * n) >> 32) : value % n;
    }

    /**
     * @brief How the PID is chosen
     */
    enum class PIDType : u8
    {
        Random, ///< Random rolls, the first shiny roll stops the loop
        Locked, ///< Random rolls, a shiny roll is turned non-shiny with PID ^= 0x10000000
        Shiny, ///< One random roll forced shiny (square)
        Fixed ///< Fixed PID, no draw
    };

    constexpr u8 AbilityRandom = 253; ///< u32 >> 31, slot 0 or 1
    constexpr u8 AbilityRandomHidden = 254; ///< rand(3), slot 0, 1 or hidden
    constexpr u8 Random = 255; ///< Draw the field (IV, nature, gender)

    /**
     * @brief Generation request, mirrors the struct the game passes to its generator
     */
    struct Request6
    {
        std::array<u8, 6> ivs = { Random, Random, Random, Random, Random, Random }; ///< Fixed IVs, Random draws
        PIDType pidType = PIDType::Random; ///< PID mode
        u32 fixedEC = 0; ///< EC when hasEC is set
        u32 fixedPID = 0; ///< PID for PIDType::Fixed
        u8 pidRolls = 1; ///< Number of PID rolls
        u8 ivCount = 0; ///< Guaranteed perfect IVs
        u8 ability = AbilityRandom; ///< 0/1/2 fixed slot, AbilityRandom or AbilityRandomHidden
        u8 nature = Random; ///< Fixed nature (Synchronize), Random draws
        u8 gender = Random; ///< 0/1/2 fixed gender (Cute Charm), Random draws from the species ratio
        bool hasEC = false; ///< Whether the EC is fixed
    };

    /**
     * @brief Result of a generation
     */
    struct Result6
    {
        u32 ec;
        u32 pid;
        std::array<u8, 6> ivs;
        u8 ability;
        u8 gender;
        u8 nature;
        u8 shiny;
    };

    /**
     * @brief Draws one Pokemon from \p rng
     *
     * @tparam RNG Object with u32 next()
     * @param rng PRNG positioned at the EC output
     * @param request Generation request
     * @param info Species information
     * @param tid Trainer ID
     * @param sid Secret ID
     *
     * @return Generated Pokemon
     */
    template <class RNG>
    Result6 generate(RNG &rng, const Request6 &request, const PersonalInfo *info, u16 tid, u16 sid)
    {
        u16 tsv = tid ^ sid;
        Result6 result;

        result.ec = request.hasEC ? request.fixedEC : rng.next();

        switch (request.pidType)
        {
        case PIDType::Random:
            for (u8 roll = 0; roll < request.pidRolls; roll++)
            {
                result.pid = rng.next();
                if (Utilities::isShiny<false>(result.pid, tsv))
                {
                    break;
                }
            }
            break;
        case PIDType::Locked:
            for (u8 roll = 0; roll < request.pidRolls; roll++)
            {
                result.pid = rng.next();
                if (Utilities::isShiny<false>(result.pid, tsv))
                {
                    result.pid ^= 0x10000000;
                    break;
                }
            }
            break;
        case PIDType::Shiny:
        {
            u32 pid = rng.next();
            u16 low = pid & 0xffff;
            result.pid = (static_cast<u32>(low ^ tsv) << 16) | low;
            break;
        }
        case PIDType::Fixed:
            result.pid = request.fixedPID;
            break;
        }
        result.shiny = Utilities::getShiny<false>(result.pid, tsv);

        result.ivs = request.ivs;
        for (u8 i = 0; i < request.ivCount;)
        {
            u8 index = rand(rng.next(), 6);
            if (result.ivs[index] == Random)
            {
                result.ivs[index] = 31;
                i++;
            }
        }
        for (u8 &iv : result.ivs)
        {
            if (iv == Random)
            {
                iv = rng.next() >> 27;
            }
        }

        if (request.ability == AbilityRandom)
        {
            result.ability = rng.next() >> 31;
        }
        else if (request.ability == AbilityRandomHidden)
        {
            result.ability = rand(rng.next(), 3);
        }
        else
        {
            result.ability = request.ability;
        }

        result.nature = request.nature == Random ? rand(rng.next(), 25) : request.nature;

        if (request.gender != Random)
        {
            result.gender = request.gender;
        }
        else
        {
            switch (info->getGender())
            {
            case 255:
                result.gender = 2;
                break;
            case 254:
                result.gender = 1;
                break;
            case 0:
                result.gender = 0;
                break;
            default:
                result.gender = rand(rng.next(), 252) + 1 < info->getGender();
                break;
            }
        }

        return result;
    }
}

#endif // POKEMON6_HPP
