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

#ifndef MIRAGE6_HPP
#define MIRAGE6_HPP

#include <Core/Global.hpp>
#include <Core/RNG/TinyMT.hpp>
#include <array>
#include <vector>

/**
 * @brief OR/AS Mirage spots (live OR, research Part IV section 9 and Part V A7)
 *
 * The save's RandomGroup daily value (u32 at save 0x1600) advances once per day the game processes (at load or at midnight):
 * value = TinyMT_first(value), where TinyMT_first(seed) is the first TinyMT output after seeding with \p seed. Neither the main MT
 * nor the main TinyMT is involved.
 * - Spot of the day: TinyMT_first(value + TID) % 33 + 1.
 * - Hidden item at the spot: candidate[TinyMT_first(value) % 6] (no TID).
 * - Spot 1 is Crescent Isle (zone 467, landing confirmed), 2-9 Mirage Forest (zones 184-191), 10-17 Mirage Cave (sky map zones
 *   192-199), 18-25 Mirage Island (200-207), 26-33 Mirage Mountain (208, 460-466).
 */
namespace MirageSpot6
{
    /**
     * @brief Kind of Mirage spot
     */
    enum class Kind : u8
    {
        CrescentIsle,
        Forest,
        Cave,
        Island,
        Mountain
    };

    /**
     * @brief One day of the calendar
     */
    struct Day
    {
        u32 value; ///< Daily value for the day
        u16 item; ///< Hidden item
        u16 zone; ///< Zone of the spot (sky map record)
        u8 spot; ///< Spot of the day, 1-33
        u8 number; ///< Spot number within its kind, 1-8 (1 for Crescent Isle)
        Kind kind; ///< Kind of spot
    };

    /**
     * @brief First TinyMT output after seeding with \p seed
     */
    inline u32 tinyFirst(u32 seed)
    {
        TinyMT rng(seed);
        return rng.next();
    }

    /**
     * @brief Daily value after one day change
     */
    inline u32 nextDay(u32 value)
    {
        return tinyFirst(value);
    }

    /**
     * @brief Spot of the day (1-33) for \p value and trainer ID \p tid
     */
    inline u8 spot(u32 value, u16 tid)
    {
        return static_cast<u8>(tinyFirst(value + tid) % 33 + 1);
    }

    /**
     * @brief Kind of spot \p spot
     */
    constexpr Kind kind(u8 spot)
    {
        if (spot == 1)
        {
            return Kind::CrescentIsle;
        }
        if (spot <= 9)
        {
            return Kind::Forest;
        }
        if (spot <= 17)
        {
            return Kind::Cave;
        }
        if (spot <= 25)
        {
            return Kind::Island;
        }
        return Kind::Mountain;
    }

    /**
     * @brief Zone of spot \p spot as the sky map lists it (live SLAN records)
     */
    constexpr u16 zone(u8 spot)
    {
        if (spot == 1)
        {
            return 467;
        }
        if (spot == 26)
        {
            return 208;
        }
        if (spot >= 27)
        {
            return 460 + spot - 27;
        }
        return 184 + spot - 2;
    }

    /**
     * @brief Hidden item for \p value at a spot of \p kind (code table 0x585D14, live OR)
     */
    inline u16 item(u32 value, Kind kind)
    {
        constexpr std::array<std::array<u16, 6>, 5> candidates = { {
            { 93, 93, 326, 226, 327, 227 }, // Crescent Isle
            { 86, 86, 87, 87, 326, 326 }, // Forest
            { 72, 72, 73, 73, 226, 226 }, // Cave
            { 88, 88, 89, 89, 327, 327 }, // Island
            { 74, 74, 75, 75, 227, 227 } // Mountain
        } };
        return candidates[static_cast<u8>(kind)][tinyFirst(value) % 6];
    }

    /**
     * @brief Mirage spots for \p days days, starting on the day of \p value
     *
     * @param value Daily value (save 0x1600) for the first day
     * @param tid Trainer ID
     * @param days Number of days
     *
     * @return One entry per day
     */
    inline std::vector<Day> calendar(u32 value, u16 tid, u32 days)
    {
        std::vector<Day> result;
        result.reserve(days);
        for (u32 i = 0; i < days; i++, value = nextDay(value))
        {
            u8 s = spot(value, tid);
            Kind k = kind(s);
            u8 number = s == 1 ? 1 : static_cast<u8>((s - 2) % 8 + 1);
            result.push_back({ value, item(value, k), zone(s), s, number, k });
        }
        return result;
    }
}

#endif // MIRAGE6_HPP
