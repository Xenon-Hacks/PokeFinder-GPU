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

#ifndef ENCOUNTERS6_HPP
#define ENCOUNTERS6_HPP

#include <Core/Global.hpp>

class StaticTemplate6;

namespace Encounters6
{
    /**
     * @brief Gets static encounters from the \p index category
     *
     * @param index Category index (0 legends, 1 starters, 2 fossils, 3 gifts, 4 stationary, 5 trades)
     * @param size Number of templates
     *
     * @return Array of static encounters
     */
    const StaticTemplate6 *getStaticEncounters(int index, int *size = nullptr);

    /**
     * @brief Gets static encounter from the \p type category at \p index
     *
     * @param type Category index
     * @param index Pokemon index
     *
     * @return Static encounter
     */
    const StaticTemplate6 *getStaticEncounter(int type, int index);
}

#endif // ENCOUNTERS6_HPP
