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

#ifndef GEN7_HPP
#define GEN7_HPP

#include <Core/Enum/Game.hpp>
#include <Core/Global.hpp>

/**
 * @brief Constants of the Gen 7 main RNG (SFMT-19937). One advance is one 64-bit draw (two SFMT words, low word first);
 * rand(n) = u64 % n with the full 64-bit modulo; 32-bit fields (EC, PID, ID) take the low word. From the Gen 7 RNG research
 * (handoff 2026-10-30, sections 1 and 2), live on the emulator for all four games and on a New 3DS for Moon 1.2.
 */
namespace Gen7
{
    /**
     * @brief Returns the advance the main RNG sits at on the title screen: 417 on Sun/Moon, 477 on Ultra Sun/Ultra Moon
     * (constant per game, no draws while the title is shown)
     */
    constexpr u32 titleAdvance(Game version)
    {
        return (version & Game::USUM) != Game::None ? 477 : 417;
    }

    /**
     * @brief Returns the advance the trainer ID is drawn at when the player name is confirmed: 1012 on Sun/Moon (live, 2 runs);
     * 1132 on Ultra Sun/Ultra Moon is 3DSRNGTool's value and is not verified
     */
    constexpr u32 idAdvance(Game version)
    {
        return (version & Game::USUM) != Game::None ? 1132 : 1012;
    }

    /**
     * @brief Returns whether the ID advance of \p version was measured live
     */
    constexpr bool idAdvanceVerified(Game version)
    {
        return (version & Game::USUM) == Game::None;
    }

    /**
     * @brief Returns the Continue card's loading-clock needle of a draw: u64 % 17
     */
    constexpr u8 needle(u64 value)
    {
        return static_cast<u8>(value % 17);
    }

    /**
     * @brief Returns the 64-bit draw reduced to [0, n): the full modulo, never the low word first
     */
    constexpr u64 rand(u64 value, u64 n)
    {
        return value % n;
    }
}

#endif // GEN7_HPP
