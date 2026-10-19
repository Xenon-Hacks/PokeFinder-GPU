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

// Delays are MT frames from the button press. Encounters (gift = false) then run the 60-output container before the EC; gifts
// (fossils, starters, Lapras, Lucario) have no container, so their delay runs straight to the EC.
// Live on Azahar (X 1.0 / Y 1.5): Mewtwo 3, Zygarde 2, Xerneas 161, Yveltal 210, roaming birds 0 (grass step), fossils 81 on OR
// (X/Y not timed). From 3DSRNGTool, not measured: Kalos starters 280/330, Kanto starters 4, Lapras 18, Lucario 20, Pidgey 14,
// Snorlax 0, Abomasnow 0.
// Every X/Y static is shiny locked and, apart from Snorlax, has a fixed ability, which skips the ability draw.
constexpr std::array<StaticTemplate6, 7> legends = {
    StaticTemplate6(Game::X, 716, 0, Shiny::Never, 0, 255, 3, 50, 161, false), // Xerneas
    StaticTemplate6(Game::Y, 717, 0, Shiny::Never, 0, 255, 3, 50, 210, false), // Yveltal
    StaticTemplate6(Game::XY, 718, 0, Shiny::Never, 0, 255, 3, 70, 2, false), // Zygarde
    StaticTemplate6(Game::XY, 150, 0, Shiny::Never, 0, 255, 3, 70, 3, false), // Mewtwo
    StaticTemplate6(Game::XY, 144, 0, Shiny::Never, 0, 255, 3, 70, 0, false), // Articuno (roamer, from the grass step)
    StaticTemplate6(Game::XY, 145, 0, Shiny::Never, 0, 255, 3, 70, 0, false), // Zapdos (roamer, from the grass step)
    StaticTemplate6(Game::XY, 146, 0, Shiny::Never, 0, 255, 3, 70, 0, false) // Moltres (roamer, from the grass step)
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
    StaticTemplate6(Game::XY, 138, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Omanyte
    StaticTemplate6(Game::XY, 140, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Kabuto
    StaticTemplate6(Game::XY, 142, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Aerodactyl
    StaticTemplate6(Game::XY, 345, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Lileep
    StaticTemplate6(Game::XY, 347, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Anorith
    StaticTemplate6(Game::XY, 408, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Cranidos
    StaticTemplate6(Game::XY, 410, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Shieldon
    StaticTemplate6(Game::XY, 564, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Tirtouga
    StaticTemplate6(Game::XY, 566, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Archen
    StaticTemplate6(Game::XY, 696, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Tyrunt
    StaticTemplate6(Game::XY, 698, 0, Shiny::Random, 255, 255, 0, 20, 81, true) // Amaura
};

// Gift table rows 15 and 16: every fixed field skips its draw (Lapras = EC, PID, gender; Lucario = EC, PID), live X
constexpr std::array<StaticTemplate6, 2> gifts = {
    StaticTemplate6(Game::XY, 131, 0, Shiny::Random, 0, 255, 0, 30, 18, true, 6, { 31, 20, 20, 20, 20, 20 }), // Lapras (Docile)
    StaticTemplate6(Game::XY, 448, 0, Shiny::Never, 0, 0, 0, 32, 20, true, 11, { 6, 25, 16, 25, 19, 31 }) // Lucario (Hasty, male)
};

constexpr std::array<StaticTemplate6, 3> stationary = {
    StaticTemplate6(Game::XY, 143, 0, Shiny::Never, 255, 255, 0, 15, 0, false), // Snorlax (random ability)
    StaticTemplate6(Game::XY, 460, 0, Shiny::Never, 0, 255, 0, 45, 0, false), // Abomasnow (Frost Cavern)
    StaticTemplate6(Game::XY, 16, 0, Shiny::Never, 0, 255, 0, 3, 14, false) // Pidgey (tutorial)
};

namespace Encounters6
{
    const StaticTemplate6 *getStaticEncounters(int index, int *size)
    {
        auto pick = [size](const auto &table) {
            if (size)
            {
                *size = static_cast<int>(table.size());
            }
            return table.data();
        };

        switch (index)
        {
        case 0:
            return pick(legends);
        case 1:
            return pick(starters);
        case 2:
            return pick(fossils);
        case 3:
            return pick(gifts);
        default:
            return pick(stationary);
        }
    }

    const StaticTemplate6 *getStaticEncounter(int type, int index)
    {
        const StaticTemplate6 *templates = getStaticEncounters(type);
        return &templates[index];
    }
}
