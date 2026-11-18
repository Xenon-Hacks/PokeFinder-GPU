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

#ifndef SHAREDGENERATOR8A_HPP
#define SHAREDGENERATOR8A_HPP

#include <Core/Gen8a/Generator8a.hpp>
#include <vector>

/**
 * @brief Filter for Pokémon made on the shared RNG
 */
struct SharedFilter8a
{
    std::array<u8, 6> ivMin = { 0, 0, 0, 0, 0, 0 };
    std::array<u8, 6> ivMax = { 31, 31, 31, 31, 31, 31 };
    std::array<bool, 25> natures;
    s8 ability = -1;
    s8 gender = -1;
    bool shinyOnly = false;
    u8 heightMin = 0;
    u8 heightMax = 255;
    u8 weightMin = 0;
    u8 weightMax = 255;

    SharedFilter8a()
    {
        natures.fill(true);
    }

    bool compare(const State8a &state) const;
};

/**
 * @brief One Pokémon of a batch made at a start advance
 */
struct SharedState8a
{
    u32 advance; ///< draws from boot to the batch's first draw
    u32 index; ///< position in the batch
    u32 end; ///< draws from boot after this Pokémon
    State8a state;
};

/**
 * @brief Lists what the shared RNG makes from a known boot seed: a batch of specs made back to back (an area's Unown,
 * or one static/gift) at every start advance of a range
 */
class SharedGenerator8a
{
public:
    /**
     * @brief Construct a new SharedGenerator8a object
     *
     * @param seed Boot seed (s0 at boot)
     * @param initialAdvance First start advance, in draws from boot
     * @param maxAdvances Number of start advances
     * @param specs Batch made back to back
     * @param filter Filter applied to each Pokémon of the batch
     */
    SharedGenerator8a(u64 seed, u32 initialAdvance, u32 maxAdvances, const std::vector<Spec8a> &specs, const SharedFilter8a &filter);

    std::vector<SharedState8a> generate() const;

private:
    std::vector<Spec8a> specs;
    SharedFilter8a filter;
    u64 seed;
    u32 initialAdvance;
    u32 maxAdvances;
};

#endif // SHAREDGENERATOR8A_HPP
