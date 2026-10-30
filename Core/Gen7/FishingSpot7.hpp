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

#ifndef FISHINGSPOT7_HPP
#define FISHINGSPOT7_HPP

#include <Core/Global.hpp>
#include <array>
#include <vector>

enum class Game : u32;

/**
 * @brief One fishing spot (ED section 12 "FS" record) with the standing position and cast constant the research verified
 */
struct FishingSpot7
{
    Game games; ///< Games the record applies to (Sun/Moon share one set, Ultra Sun/Ultra Moon another)
    u16 zone; ///< Zone id the spot's record belongs to
    u8 table; ///< Encounter table index of the spot (record + 0x34)
    u8 rareTable; ///< Encounter table used while the spot is in its rare (bubbling) state, 255 when the record has none
    u8 rareRate; ///< Rare-state lottery rate rolled at the first load after a zone change (rand(100) < rate), 0 when none
    u8 castDelay; ///< C: frames from the cast frame to the start of the bite wait (48 at normal tiles, 67 SM / 59 USUM where the game first walks the player)
    const char *location; ///< Location name
    std::array<s32, 3> spot; ///< Spot position (x, z, height)
    std::array<s32, 3> stand; ///< Standing position the casts were verified from (x, z, height)
    const char *standNote; ///< Where to stand, in words
    bool reachable; ///< False for the three records no cast ever reached (leave them out of a plan)
    bool verified; ///< Whether casts at this spot were logged and fitted in this game
    bool lapras; ///< Casts only from Lapras
};

namespace FishingSpots7
{
    /**
     * @brief Returns the fishing spots of \p version
     */
    std::vector<const FishingSpot7 *> getSpots(Game version);
}

#endif // FISHINGSPOT7_HPP
