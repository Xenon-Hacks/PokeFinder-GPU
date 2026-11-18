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

#ifndef GENERATOR8A_HPP
#define GENERATOR8A_HPP

#include <Core/Gen8a/Rng8a.hpp>
#include <array>

/**
 * @brief Input of the Legends: Arceus Pokémon generator (main text 0x2B8BAC0), the "initial spec" the game fills in
 * before it calls the generator. Unset values are drawn, in the order of \ref Generator8a::generate.
 */
struct Spec8a
{
    static constexpr u64 neverShiny = 0x1FFFFFFFFULL; ///< one PID draw, flipped with 0x10000000 if it happens to be shiny
    static constexpr u64 alwaysShiny = 0x2FFFFFFFFULL; ///< one PID draw, top half rewritten to be shiny
    static constexpr u64 randomPID = 0x3FFFFFFFFULL; ///< up to \ref rolls PID draws, stops at the first shiny one

    s64 ec = -1; ///< fixed encryption constant, -1 = draw
    s64 tid = -1; ///< trainer id used for the shiny test, -1 = draw (the real trainer is not used)
    u64 pid = randomPID; ///< PID code (above) or a fixed PID
    u16 species = 0;
    u16 form = 0; ///< 0xFF = rand(formCount)
    u8 level = 1;
    u8 gender = 0xFF; ///< 0xFF = draw by ratio, else fixed (then forced by the species ratio)
    s8 nature = -1; ///< -1 = rand(25)
    u8 ability = 0xFF; ///< 0xFF = rand(2), 0xFE = rand(3), else fixed slot
    u8 rolls = 1; ///< PID draws for \ref randomPID
    std::array<s8, 6> ivs = { -1, -1, -1, -1, -1, -1 }; ///< fixed IVs, -1 = draw
    u8 flawless = 0; ///< guaranteed 31s, picked with rand(6); 6 or more means none
    u16 height = 0x100; ///< size code (below 0x100 = fixed value, 0x100 = rand(129) + rand(128), 0x101-0x105 = bands)
    u16 weight = 0x100; ///< size code, as \ref height
    u8 genderRatio = 127; ///< species ratio: 0 male, 254 female, 255 genderless, else threshold
    u8 formCount = 1; ///< forms of the species, used when \ref form is 0xFF
};

/**
 * @brief One generated Pokémon
 */
struct State8a
{
    u32 ec;
    u32 tid; ///< the trainer id the shiny test used (random unless the spec fixes it)
    u32 pid;
    bool shiny;
    u8 pidDraws;
    std::array<u8, 6> ivs;
    u8 ability; ///< slot: 0, 1, or 2 = hidden
    u16 form;
    u8 gender; ///< 0 male, 1 female, 2 genderless
    u8 nature;
    u8 height;
    u8 weight;
    u32 draws; ///< RNG draws the generator used
};

namespace Generator8a
{
    /**
     * @brief Runs the generator on \p rng. \p seeded selects the seeded path (an entity seed: 32-bit draws redraw
     * 0xFFFFFFFF, rand(1) draws) instead of the shared path (Unown, statics, gifts)
     */
    State8a generate(Xoro8a &rng, const Spec8a &spec, bool seeded);

    /**
     * @brief Shiny test of the game: (pid ^ tid) folded to 16 bits is below 16
     */
    constexpr bool isShiny(u32 pid, u32 tid)
    {
        u32 x = pid ^ tid;
        return ((x >> 16) ^ (x & 0xFFFF)) < 16;
    }
}

#endif // GENERATOR8A_HPP
