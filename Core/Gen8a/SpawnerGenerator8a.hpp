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

#ifndef SPAWNERGENERATOR8A_HPP
#define SPAWNERGENERATOR8A_HPP

#include <Core/Gen8a/Generator8a.hpp>
#include <vector>

/**
 * @brief One Pokémon of a spawner fill
 */
struct SpawnerState8a
{
    u32 fill; ///< 0 = the fill made by the given group seed
    u32 slot; ///< position inside the fill
    u64 groupSeed; ///< group seed of the fill
    u64 slotSeed;
    u64 alphaSeed; ///< seeds the alpha move pick
    u64 entitySeed;
    double slotRoll; ///< 0-1: picks the encounter slot by weight
    State8a state;
};

/**
 * @brief Future spawns of a wild spawner from its group seed (save block 0x511622B3, research section 5).
 *
 * Fill: r = Xoroshiro(group seed); per new Pokémon slot seed = r.next(), alpha seed = r.next(); new group seed = r.next().
 * Slot: Xoroshiro(slot seed) gives the slot roll (u64 * 2^-64), the entity seed, then the level roll. The entity seed
 * makes the Pokémon on the seeded path. Species comes from the slot roll and the area's encounter table (not included).
 */
namespace SpawnerGenerator8a
{
    /**
     * @brief \p fills fills of \p fillSize Pokémon each from \p groupSeed
     *
     * @param rolls Shiny rolls (1 base, +1 research 10, +2 perfect, +3 Shiny Charm, +25 outbreak, +12 massive outbreak)
     * @param genderRatio Species gender ratio
     * @param shinyOnly Keep shiny Pokémon only
     */
    std::vector<SpawnerState8a> generate(u64 groupSeed, u32 fills, u32 fillSize, u8 rolls, u8 genderRatio, bool shinyOnly);
}

#endif // SPAWNERGENERATOR8A_HPP
