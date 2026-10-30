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

#ifndef ENCOUNTERS7_HPP
#define ENCOUNTERS7_HPP

#include <Core/Global.hpp>

class StaticTemplate7;

namespace Encounters7
{
    /**
     * @brief Static, gift and trade categories
     */
    enum class StaticCategory7 : u8
    {
        Legends, ///< Tapus, Solgaleo/Lunala, Necrozma, Zygarde, Nihilego (Aether), Totem Gumshoos
        UltraSpaceLegends, ///< Ultra Space Wilds legendaries (USUM)
        UltraSpaceWilds, ///< Ultra Space Wilds ordinary Pokemon (USUM)
        LegendGifts, ///< Cosmog, Type: Null, Poipole, Magearna, Zygarde, Surf Pikachu
        WildGifts, ///< Fossils, Porygon, Aerodactyl, Eevee egg, USUM starters
        Trades ///< In-game trades
    };

    /**
     * @brief Returns the templates of a category
     *
     * @param category Category index (StaticCategory7)
     * @param size Number of templates
     */
    const StaticTemplate7 *getStaticEncounters(int category, int *size = nullptr);

    /**
     * @brief Returns one template of a category
     */
    const StaticTemplate7 *getStaticEncounter(int category, int index);
}

#endif // ENCOUNTERS7_HPP
