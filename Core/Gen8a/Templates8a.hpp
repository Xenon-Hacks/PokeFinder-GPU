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

#ifndef TEMPLATES8A_HPP
#define TEMPLATES8A_HPP

#include <Core/Gen8a/Generator8a.hpp>
#include <string>
#include <vector>

/**
 * @brief One row of romfs poke_event_encount.bin (static) or poke_add.bin (gift)
 */
struct RawTemplate8a
{
    const char *name; ///< internal name of the row
    u16 species;
    u16 form;
    u8 level;
    u8 gender; ///< 0xFF = random
    s8 nature; ///< -1 = random
    u8 shinyLocked;
    u16 height;
    u16 weight;
    s8 ivs[6];
    u8 flawless;
    u8 genderRatio;
};

/**
 * @brief Pokémon the game makes on the shared RNG: Unown on an area load, statics and gifts
 */
namespace Templates8a
{
    /**
     * @brief Unown spec as logged live: form fixed, Lv 25, shiny-locked, EC/trainer id/IVs/ability/nature random,
     * genderless, height 128, weight random
     */
    Spec8a unown(u16 form);

    /**
     * @brief Static rows (romfs poke_event_encount.bin)
     */
    const std::vector<RawTemplate8a> &statics();

    /**
     * @brief Gift rows (romfs poke_add.bin)
     */
    const std::vector<RawTemplate8a> &gifts();

    /**
     * @brief Generator spec of a static or gift row. Confirmed live for the Chimchar static (hikozaru_01), the Alolan
     * Vulpix gift (event_092) and the kept starter (shoki3_*); every other row is built the same way
     */
    Spec8a spec(const RawTemplate8a &row);

    /**
     * @brief English species name, including the Legends: Arceus additions 899-905
     */
    std::string speciesName(u16 species);

    /**
     * @brief Unown letter of a form (A-Z, !, ?)
     */
    char unownLetter(u16 form);
}

#endif // TEMPLATES8A_HPP
