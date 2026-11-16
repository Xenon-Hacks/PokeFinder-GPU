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

#ifndef RNG8A_HPP
#define RNG8A_HPP

#include <Core/Global.hpp>

/**
 * @brief Legends: Arceus (1.1.1) Xoroshiro128+ as the game uses it.
 *
 * Every generator in the game is a plain Xoroshiro128+ (rotl 24 / shl 16 / rotl 37). A seeded generator starts at
 * (seed, 0x82A2B175229D6A5B). The game's shared ("global") generator starts the same way at boot, from a 64-bit
 * hardware-random seed, and is stepped by every random event that has no seed of its own (Unown, statics, gifts,
 * blinks, ambient sounds, AI rolls).
 *
 * rand(n): mask the output to the next power of two minus one and redraw while the value is >= n; n == 1 gives 0 with no
 * draw (shared path). 32-bit draws: the seeded path redraws 0xFFFFFFFF, the shared path (0xC12A70) does not.
 */
struct Xoro8a
{
    static constexpr u64 C = 0x82A2B175229D6A5BULL;

    u64 s0;
    u64 s1;
    u32 draws = 0;

    Xoro8a(u64 seed) : s0(seed), s1(C)
    {
    }

    Xoro8a(u64 seed0, u64 seed1) : s0(seed0), s1(seed1)
    {
    }

    static constexpr u64 rotl(u64 x, int k)
    {
        return (x << k) | (x >> (64 - k));
    }

    u64 next()
    {
        u64 result = s0 + s1;
        u64 t = s0 ^ s1;
        s0 = rotl(s0, 24) ^ t ^ (t << 16);
        s1 = rotl(t, 37);
        draws++;
        return result;
    }

    void advance(u64 count)
    {
        for (u64 i = 0; i < count; i++)
        {
            next();
        }
    }

    /**
     * @brief rand(n) with the game's mask-and-redraw rule. n == 0 is a full 32-bit draw. n == 1 gives 0: with no draw on
     * the shared path, after one draw on a seeded generator (\p seeded)
     */
    u32 rand(u32 n, bool seeded = false)
    {
        if (n == 0)
        {
            return static_cast<u32>(next());
        }
        if (n == 1)
        {
            if (seeded)
            {
                next();
            }
            return 0;
        }
        u64 mask = 1;
        while (mask < n)
        {
            mask <<= 1;
        }
        mask--;
        u64 result;
        do
        {
            result = next() & mask;
        } while (result >= n);
        return static_cast<u32>(result);
    }

    /**
     * @brief 32-bit draw. The seeded path (\p seeded) redraws 0xFFFFFFFF, the shared path does not
     */
    u32 u32Draw(bool seeded)
    {
        u32 result;
        do
        {
            result = static_cast<u32>(next());
        } while (seeded && result == 0xFFFFFFFF);
        return result;
    }
};

#endif // RNG8A_HPP
