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

#include "Encounters6.hpp"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Shiny.hpp>
#include <Core/Gen6/StaticTemplate6.hpp>
#include <array>

// Delays are MT frames from the button press (or encounter step) to the 60-frame skip before the EC.
// Live-measured on X 1.0 / Y 1.5: Mewtwo 3, Zygarde 2, Xerneas 161, Yveltal 210, roaming birds 0 (grass step), fossil draw order.
// Taken from 3DSRNGTool (not measured): fossil 80, Kalos starters 280/330, Kanto starters 4, Snorlax 0, Pidgey 14.
constexpr std::array<StaticTemplate6, 7> legends = {
    StaticTemplate6(Game::X, 716, 0, Shiny::Never, 0, 255, 3, 50, 161, false), // Xerneas
    StaticTemplate6(Game::Y, 717, 0, Shiny::Never, 0, 255, 3, 50, 210, false), // Yveltal
    StaticTemplate6(Game::XY, 718, 0, Shiny::Never, 0, 255, 3, 70, 2, false), // Zygarde
    StaticTemplate6(Game::XY, 150, 0, Shiny::Never, 0, 255, 3, 70, 3, false), // Mewtwo
    StaticTemplate6(Game::XY, 144, 0, Shiny::Never, 255, 255, 3, 70, 0, false), // Articuno (roamer, from the grass step)
    StaticTemplate6(Game::XY, 145, 0, Shiny::Never, 255, 255, 3, 70, 0, false), // Zapdos (roamer, from the grass step)
    StaticTemplate6(Game::XY, 146, 0, Shiny::Never, 255, 255, 3, 70, 0, false) // Moltres (roamer, from the grass step)
};

constexpr std::array<StaticTemplate6, 6> starters = {
    StaticTemplate6(Game::XY, 650, 0, Shiny::Random, 255, 255, 0, 5, 280, true), // Chespin
    StaticTemplate6(Game::XY, 653, 0, Shiny::Random, 255, 255, 0, 5, 280, true), // Fennekin
    StaticTemplate6(Game::XY, 656, 0, Shiny::Random, 255, 255, 0, 5, 330, true), // Froakie
    StaticTemplate6(Game::XY, 1, 0, Shiny::Random, 255, 255, 0, 10, 4, true), // Bulbasaur
    StaticTemplate6(Game::XY, 4, 0, Shiny::Random, 255, 255, 0, 10, 4, true), // Charmander
    StaticTemplate6(Game::XY, 7, 0, Shiny::Random, 255, 255, 0, 10, 4, true) // Squirtle
};

constexpr std::array<StaticTemplate6, 11> fossils = {
    StaticTemplate6(Game::XY, 138, 0, Shiny::Random, 255, 255, 0, 20, 80, true), // Omanyte
    StaticTemplate6(Game::XY, 140, 0, Shiny::Random, 255, 255, 0, 20, 80, true), // Kabuto
    StaticTemplate6(Game::XY, 142, 0, Shiny::Random, 255, 255, 0, 20, 80, true), // Aerodactyl
    StaticTemplate6(Game::XY, 345, 0, Shiny::Random, 255, 255, 0, 20, 80, true), // Lileep
    StaticTemplate6(Game::XY, 347, 0, Shiny::Random, 255, 255, 0, 20, 80, true), // Anorith
    StaticTemplate6(Game::XY, 408, 0, Shiny::Random, 255, 255, 0, 20, 80, true), // Cranidos
    StaticTemplate6(Game::XY, 410, 0, Shiny::Random, 255, 255, 0, 20, 80, true), // Shieldon
    StaticTemplate6(Game::XY, 564, 0, Shiny::Random, 255, 255, 0, 20, 80, true), // Tirtouga
    StaticTemplate6(Game::XY, 566, 0, Shiny::Random, 255, 255, 0, 20, 80, true), // Archen
    StaticTemplate6(Game::XY, 696, 0, Shiny::Random, 255, 255, 0, 20, 80, true), // Tyrunt
    StaticTemplate6(Game::XY, 698, 0, Shiny::Random, 255, 255, 0, 20, 80, true) // Amaura
};

constexpr std::array<StaticTemplate6, 2> stationary = {
    StaticTemplate6(Game::XY, 143, 0, Shiny::Never, 255, 255, 0, 15, 0, false), // Snorlax
    StaticTemplate6(Game::XY, 16, 0, Shiny::Never, 0, 255, 0, 3, 14, false) // Pidgey
};

namespace Encounters6
{
    const StaticTemplate6 *getStaticEncounters(int index, int *size)
    {
        if (index == 0)
        {
            if (size)
            {
                *size = legends.size();
            }
            return legends.data();
        }
        else if (index == 1)
        {
            if (size)
            {
                *size = starters.size();
            }
            return starters.data();
        }
        else if (index == 2)
        {
            if (size)
            {
                *size = fossils.size();
            }
            return fossils.data();
        }
        else
        {
            if (size)
            {
                *size = stationary.size();
            }
            return stationary.data();
        }
    }

    const StaticTemplate6 *getStaticEncounter(int type, int index)
    {
        const StaticTemplate6 *templates = getStaticEncounters(type);
        return &templates[index];
    }
}
