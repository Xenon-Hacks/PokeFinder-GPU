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

#ifndef LINEAR8A_HPP
#define LINEAR8A_HPP

#include <Core/Global.hpp>
#include <array>
#include <vector>

/**
 * @brief GF(2) tools for recovering a Xoroshiro128+ seed of the form (x, 0x82A2B175229D6A5B) from small outputs.
 *
 * Every state bit t steps after (x, C) is an affine function of the 64 bits of x. An output is s0 + s1, so its low bits
 * are not linear, but they are once the low bits of s0 are guessed: for a 5-bit value o (an IV, rand(32)) guessing the
 * low 4 bits g of s0 fixes the low 4 bits of s1 as (o - g) & 15 and bit 4 of s0 ^ s1 as bit 4 of o ^ the carry, which
 * is 9 linear equations for 4 guessed bits. With enough observations the 64 bits of x follow by elimination, so the
 * seeds that fit are x = base ^ T[0][g0] ^ T[1][g1] ^ ... ^ (free bits), and a search only XORs table entries.
 */
namespace Linear8a
{
    /**
     * @brief One 64-bit word as an affine function of x: bit i = parity(mask[i] & x) ^ bit i of constant
     */
    struct Word
    {
        std::array<u64, 64> mask;
        u64 constant;

        u64 apply(u64 x) const;
        u64 applyLinear(u64 x) const;
    };

    /**
     * @brief State words (s0, s1) of Xoroshiro(x, C) after 0..steps steps
     */
    struct Symbolic
    {
        std::vector<Word> s0;
        std::vector<Word> s1;

        Symbolic(u32 steps);
    };

    /**
     * @brief A 5-bit output (rand(32)) of the draw at \p step: draw k reads the state after k steps
     */
    struct Obs5
    {
        u32 step;
        u8 value;
    };

    /**
     * @brief Bit 0 of the draw at \p step (rand(2))
     */
    struct Obs1
    {
        u32 step;
        u8 value;
    };

    /**
     * @brief Search plan: digits whose table entries XOR together into every candidate
     *
     * Each candidate is described by \ref words u64 values: word 0 is x, then (s0, s1) for each requested target step.
     */
    struct Plan
    {
        bool feasible = false;
        u32 words = 0;
        std::vector<u64> base; ///< words values with every digit at 0
        std::vector<u32> radix; ///< 16 for a guessed observation, 2 for a free bit
        std::vector<u32> offset; ///< first entry of each digit in \ref table
        std::vector<u64> table; ///< entries * words, entry 0 of every digit is the digit's own value 0
        std::vector<int> used; ///< indices of the Obs5 the plan solved with (the rest can be checked)
        u32 bits = 0; ///< log2 of the number of candidates

        u64 count() const
        {
            return 1ULL << bits;
        }
    };

    /**
     * @brief Builds a plan. Observations are taken in order; one is used when it adds more than 4 to the rank, so
     * the plan ends with at most 4 free bits less than it would gain
     *
     * @param sym Symbolic states, long enough for every step
     * @param obs5 5-bit observations in priority order
     * @param obs1 1-bit observations (always used)
     * @param targets steps whose (s0, s1) each candidate should carry
     */
    Plan makePlan(const Symbolic &sym, const std::vector<Obs5> &obs5, const std::vector<Obs1> &obs1, const std::vector<u32> &targets);
}

#endif // LINEAR8A_HPP
