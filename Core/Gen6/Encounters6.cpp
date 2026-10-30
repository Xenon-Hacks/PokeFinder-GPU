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
// (fossils, starters, Lapras, Lucario) and trades have no container, so their delay runs straight to the EC.
//
// X/Y, measured on Azahar (X 1.0 / Y 1.5): Mewtwo 3, Zygarde 2, Xerneas 161, Yveltal 209 (208-211 in 6 runs), roaming birds 0
// (grass step). Every X/Y static is shiny locked and, apart from Snorlax, has a fixed ability, which skips the ability draw.
//
// OR/AS, measured on Azahar (base 1.0): Groudon talk A + 3831, Kyogre + 3843, Deoxys tablet A + 301, Regice talk + 186, Rayquaza
// second A + 99 with the EC one output later (stored as 100), Southern Island Latias second A + 2, fossils A + 81, starter pick
// A + 140 (no container), soaring sky encounters 0 from the trigger. OR/AS statics draw the ability (rand(2) + 1) even for
// single-ability legends and skip the gender draw only when genderless or fixed, so their ability is 255 (drawn) here. Locks come
// from the DllField static and gift tables: Groudon, Kyogre, Rayquaza, Deoxys and the Cosplay Pikachu.
//
// Anything made with guess() has a delay from 3DSRNGTool, another game or an estimate, and is flagged as not measured.

namespace
{
    constexpr std::array<u8, 6> randomIVs = { 255, 255, 255, 255, 255, 255 };

    /**
     * @brief Template whose delay was not measured in the game
     */
    constexpr StaticTemplate6 guess(Game version, u16 specie, u8 form, Shiny shiny, u8 ability, u8 gender, u8 ivCount, u8 level,
                                    u16 delay, bool gift, u8 nature = 255, const std::array<u8, 6> &ivs = randomIVs)
    {
        return StaticTemplate6(version, specie, form, shiny, ability, gender, ivCount, level, delay, gift, nature, ivs, false);
    }

    /**
     * @brief In-game trade: one PID roll locked against the trade OT (secret ID 0), fixed ability, gender and nature, generated on the
     * A press that closes "Let's start our Pokemon trade!" with no container (live X Steelix: 7 draws = EC, PID, 5 random IVs)
     */
    constexpr StaticTemplate6 trade(Game version, u16 specie, u8 level, u8 ability, u8 gender, u8 nature, const std::array<u8, 6> &ivs,
                                    u16 tid)
    {
        return StaticTemplate6(version, specie, 0, Shiny::Never, ability, gender, 0, level, 0, true, nature, ivs, false, tid);
    }
}

constexpr std::array<StaticTemplate6, 39> legends = {
    // X/Y
    StaticTemplate6(Game::X, 716, 0, Shiny::Never, 0, 255, 3, 50, 161, false), // Xerneas
    StaticTemplate6(Game::Y, 717, 0, Shiny::Never, 0, 255, 3, 50, 209, false), // Yveltal
    StaticTemplate6(Game::XY, 718, 0, Shiny::Never, 0, 255, 3, 70, 2, false), // Zygarde
    StaticTemplate6(Game::XY, 150, 0, Shiny::Never, 0, 255, 3, 70, 3, false), // Mewtwo
    StaticTemplate6(Game::XY, 144, 0, Shiny::Never, 0, 255, 3, 70, 0, false), // Articuno (roamer, from the grass step)
    StaticTemplate6(Game::XY, 145, 0, Shiny::Never, 0, 255, 3, 70, 0, false), // Zapdos (roamer, from the grass step)
    StaticTemplate6(Game::XY, 146, 0, Shiny::Never, 0, 255, 3, 70, 0, false), // Moltres (roamer, from the grass step)

    // OR/AS, measured
    StaticTemplate6(Game::OR, 383, 0, Shiny::Never, 255, 255, 3, 45, 3831, false), // Groudon (Cave of Origin, talk A)
    StaticTemplate6(Game::AS, 382, 0, Shiny::Never, 255, 255, 3, 45, 3843, false), // Kyogre (Cave of Origin, talk A)
    StaticTemplate6(Game::ORAS, 384, 0, Shiny::Never, 255, 255, 3, 70, 100, false), // Rayquaza (Sky Pillar, second A)
    StaticTemplate6(Game::ORAS, 386, 0, Shiny::Never, 255, 255, 3, 80, 301, false), // Deoxys (A closing the tablet's message)
    StaticTemplate6(Game::ORAS, 378, 0, Shiny::Random, 255, 255, 3, 40, 186, false), // Regice (talk A)
    StaticTemplate6(Game::OR, 380, 0, Shiny::Random, 255, 255, 3, 30, 2, false), // Latias (Southern Island, second A)

    // OR/AS, not measured
    guess(Game::AS, 381, 0, Shiny::Random, 255, 255, 3, 30, 2, false), // Latios (Southern Island, OR Latias's delay)
    guess(Game::ORAS, 377, 0, Shiny::Random, 255, 255, 3, 40, 186, false), // Regirock (Regice's delay)
    guess(Game::ORAS, 379, 0, Shiny::Random, 255, 255, 3, 40, 186, false), // Registeel (Regice's delay)
    guess(Game::ORAS, 486, 0, Shiny::Random, 255, 255, 3, 50, 0, false), // Regigigas
    guess(Game::OR, 250, 0, Shiny::Random, 255, 255, 3, 50, 20, false), // Ho-Oh (Sea Mauville, 3DSRNGTool)
    guess(Game::AS, 249, 0, Shiny::Random, 255, 255, 3, 50, 20, false), // Lugia (Sea Mauville, 3DSRNGTool)
    guess(Game::OR, 484, 0, Shiny::Random, 255, 255, 3, 50, 88, false), // Palkia (soaring, 3DSRNGTool)
    guess(Game::AS, 483, 0, Shiny::Random, 255, 255, 3, 50, 88, false), // Dialga (soaring, 3DSRNGTool)
    guess(Game::ORAS, 487, 0, Shiny::Random, 255, 255, 3, 50, 88, false), // Giratina (soaring, 3DSRNGTool)
    guess(Game::OR, 641, 0, Shiny::Random, 255, 255, 3, 50, 88, false), // Tornadus (soaring, 3DSRNGTool)
    guess(Game::AS, 642, 0, Shiny::Random, 255, 255, 3, 50, 88, false), // Thundurus (soaring, 3DSRNGTool)
    guess(Game::ORAS, 645, 0, Shiny::Random, 255, 255, 3, 50, 88, false), // Landorus (soaring, 3DSRNGTool)
    guess(Game::ORAS, 243, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Raikou (Hoopa ring, 3DSRNGTool)
    guess(Game::ORAS, 244, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Entei (Hoopa ring, 3DSRNGTool)
    guess(Game::ORAS, 245, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Suicune (Hoopa ring, 3DSRNGTool)
    guess(Game::ORAS, 480, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Uxie (Hoopa ring, 3DSRNGTool)
    guess(Game::ORAS, 481, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Mesprit (Hoopa ring, 3DSRNGTool)
    guess(Game::ORAS, 482, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Azelf (Hoopa ring, 3DSRNGTool)
    guess(Game::ORAS, 485, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Heatran (Hoopa ring, 3DSRNGTool)
    guess(Game::ORAS, 488, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Cresselia (Hoopa ring, 3DSRNGTool)
    guess(Game::ORAS, 638, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Cobalion (Hoopa ring, 3DSRNGTool)
    guess(Game::ORAS, 639, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Terrakion (Hoopa ring, 3DSRNGTool)
    guess(Game::ORAS, 640, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Virizion (Hoopa ring, 3DSRNGTool)
    guess(Game::OR, 643, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Reshiram (Hoopa ring, 3DSRNGTool)
    guess(Game::AS, 644, 0, Shiny::Random, 255, 255, 3, 50, 16, false), // Zekrom (Hoopa ring, 3DSRNGTool)
    guess(Game::ORAS, 646, 0, Shiny::Random, 255, 255, 3, 50, 16, false) // Kyurem (Hoopa ring, 3DSRNGTool)
};

// OR/AS starters: one 11-output block (EC, PID, 6 IVs, ability, nature, gender) at the pick A + 140 (live OR Torchic, 3 runs)
constexpr std::array<StaticTemplate6, 18> starters = {
    guess(Game::XY, 650, 0, Shiny::Random, 255, 255, 0, 5, 280, true), // Chespin (3DSRNGTool)
    guess(Game::XY, 653, 0, Shiny::Random, 255, 255, 0, 5, 280, true), // Fennekin (3DSRNGTool)
    guess(Game::XY, 656, 0, Shiny::Random, 255, 255, 0, 5, 330, true), // Froakie (3DSRNGTool)
    guess(Game::XY, 1, 0, Shiny::Random, 255, 255, 0, 10, 4, true), // Bulbasaur (3DSRNGTool)
    guess(Game::XY, 4, 0, Shiny::Random, 255, 255, 0, 10, 4, true), // Charmander (3DSRNGTool)
    guess(Game::XY, 7, 0, Shiny::Random, 255, 255, 0, 10, 4, true), // Squirtle (3DSRNGTool)

    StaticTemplate6(Game::ORAS, 255, 0, Shiny::Random, 255, 255, 0, 5, 140, true), // Torchic
    guess(Game::ORAS, 258, 0, Shiny::Random, 255, 255, 0, 5, 140, true), // Mudkip (Torchic's delay; 3DSRNGTool has them equal)
    guess(Game::ORAS, 252, 0, Shiny::Random, 255, 255, 0, 5, 188, true), // Treecko (3DSRNGTool 186 + the 2 Torchic measured over it)
    guess(Game::ORAS, 152, 0, Shiny::Random, 255, 255, 0, 5, 0, true), // Chikorita
    guess(Game::ORAS, 155, 0, Shiny::Random, 255, 255, 0, 5, 0, true), // Cyndaquil
    guess(Game::ORAS, 158, 0, Shiny::Random, 255, 255, 0, 5, 0, true), // Totodile
    guess(Game::ORAS, 387, 0, Shiny::Random, 255, 255, 0, 5, 0, true), // Turtwig
    guess(Game::ORAS, 390, 0, Shiny::Random, 255, 255, 0, 5, 0, true), // Chimchar
    guess(Game::ORAS, 393, 0, Shiny::Random, 255, 255, 0, 5, 0, true), // Piplup
    guess(Game::ORAS, 495, 0, Shiny::Random, 255, 255, 0, 5, 0, true), // Snivy
    guess(Game::ORAS, 498, 0, Shiny::Random, 255, 255, 0, 5, 0, true), // Tepig
    guess(Game::ORAS, 501, 0, Shiny::Random, 255, 255, 0, 5, 0, true) // Oshawott
};

// One 11-output block with no container and one PID roll even with the Shiny Charm. A + 81 measured on OR; X/Y not timed.
constexpr std::array<StaticTemplate6, 22> fossils = {
    guess(Game::XY, 138, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Omanyte
    guess(Game::XY, 140, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Kabuto
    guess(Game::XY, 142, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Aerodactyl
    guess(Game::XY, 345, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Lileep
    guess(Game::XY, 347, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Anorith
    guess(Game::XY, 408, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Cranidos
    guess(Game::XY, 410, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Shieldon
    guess(Game::XY, 564, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Tirtouga
    guess(Game::XY, 566, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Archen
    guess(Game::XY, 696, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Tyrunt
    guess(Game::XY, 698, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Amaura

    StaticTemplate6(Game::ORAS, 138, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Omanyte
    StaticTemplate6(Game::ORAS, 140, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Kabuto
    StaticTemplate6(Game::ORAS, 142, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Aerodactyl
    StaticTemplate6(Game::ORAS, 345, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Lileep
    StaticTemplate6(Game::ORAS, 347, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Anorith
    StaticTemplate6(Game::ORAS, 408, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Cranidos
    StaticTemplate6(Game::ORAS, 410, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Shieldon
    StaticTemplate6(Game::ORAS, 564, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Tirtouga
    StaticTemplate6(Game::ORAS, 566, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Archen
    StaticTemplate6(Game::ORAS, 696, 0, Shiny::Random, 255, 255, 0, 20, 81, true), // Tyrunt
    StaticTemplate6(Game::ORAS, 698, 0, Shiny::Random, 255, 255, 0, 20, 81, true) // Amaura
};

// Every fixed field skips its draw (live X Lapras = EC, PID, gender; Lucario = EC, PID; live OR/AS through the fossil scientist:
// Cosplay Pikachu = EC, PID, 3 IV picks, 3 IVs, nature; Beldum = EC, PID, 4 IVs, nature; Castform = EC, PID, 5 IVs; Latios = EC, PID,
// 3 picks, 3 IVs, nature; Camerupt = EC, PID, 6 IVs). The real OR/AS gift scenes are story-gated, so their delays are not measured.
constexpr std::array<StaticTemplate6, 9> gifts = {
    guess(Game::XY, 131, 0, Shiny::Random, 0, 255, 0, 30, 18, true, 6, { 31, 20, 20, 20, 20, 20 }), // Lapras (Docile, 3DSRNGTool)
    guess(Game::XY, 448, 0, Shiny::Never, 0, 0, 0, 32, 20, true, 11, { 6, 25, 16, 25, 19, 31 }), // Lucario (Hasty, male, 3DSRNGTool)

    guess(Game::ORAS, 25, 1, Shiny::Never, 2, 1, 3, 20, 0, true), // Cosplay Pikachu (hidden ability, female)
    guess(Game::ORAS, 374, 0, Shiny::Random, 0, 255, 0, 1, 0, true, 255, { 255, 255, 31, 255, 31, 255 }), // Beldum (Def, SpD 31)
    guess(Game::ORAS, 351, 0, Shiny::Random, 0, 1, 0, 30, 0, true, 9, { 255, 255, 255, 31, 255, 255 }), // Castform (Lax, SpA 31)
    guess(Game::OR, 381, 0, Shiny::Random, 0, 255, 3, 30, 0, true), // Latios (Eon Flute gift)
    guess(Game::AS, 380, 0, Shiny::Random, 0, 255, 3, 30, 0, true), // Latias (Eon Flute gift)
    guess(Game::ORAS, 323, 0, Shiny::Random, 0, 1, 0, 40, 0, true, 17), // Camerupt (Quiet, female)
    guess(Game::ORAS, 319, 0, Shiny::Random, 0, 1, 0, 40, 0, true, 3) // Sharpedo (Adamant, female)
};

// OR/AS: Kecleon (Devon Scope), Voltorb, Electrode, Spiritomb and the soaring sky encounters (EC = trigger + 60, 11 outputs, live OR)
constexpr std::array<StaticTemplate6, 18> stationary = {
    guess(Game::XY, 143, 0, Shiny::Never, 255, 255, 0, 15, 0, false), // Snorlax (random ability)
    guess(Game::XY, 460, 0, Shiny::Never, 0, 255, 0, 45, 0, false), // Abomasnow (Frost Cavern)
    guess(Game::XY, 16, 0, Shiny::Never, 0, 255, 0, 3, 14, false), // Pidgey (tutorial, 3DSRNGTool)

    guess(Game::ORAS, 352, 0, Shiny::Random, 255, 255, 0, 30, 0, false), // Kecleon (Routes 119 and 120)
    guess(Game::ORAS, 352, 0, Shiny::Random, 255, 1, 0, 40, 0, false), // Kecleon (Lavaridge Town, female)
    guess(Game::ORAS, 352, 0, Shiny::Random, 2, 255, 0, 45, 0, false), // Kecleon (Mossdeep City, hidden ability)
    guess(Game::ORAS, 100, 0, Shiny::Random, 255, 255, 0, 20, 120, false), // Voltorb (Route 119, 3DSRNGTool)
    guess(Game::ORAS, 101, 0, Shiny::Random, 255, 255, 0, 40, 122, false), // Electrode (Magma / Aqua Hideout, 3DSRNGTool)
    guess(Game::ORAS, 442, 0, Shiny::Random, 255, 255, 0, 50, 0, false), // Spiritomb (Route 120)
    StaticTemplate6(Game::ORAS, 276, 0, Shiny::Random, 255, 255, 0, 40, 0, false), // Taillow (soaring)
    StaticTemplate6(Game::ORAS, 278, 0, Shiny::Random, 255, 255, 0, 40, 0, false), // Wingull (soaring)
    StaticTemplate6(Game::ORAS, 279, 0, Shiny::Random, 255, 255, 0, 40, 0, false), // Pelipper (soaring)
    StaticTemplate6(Game::ORAS, 333, 0, Shiny::Random, 255, 255, 0, 40, 0, false), // Swablu (soaring)
    StaticTemplate6(Game::ORAS, 628, 0, Shiny::Random, 255, 255, 0, 45, 0, false), // Braviary (soaring)
    StaticTemplate6(Game::ORAS, 425, 0, Shiny::Random, 255, 255, 0, 45, 0, false), // Drifloon (soaring)
    StaticTemplate6(Game::ORAS, 198, 0, Shiny::Random, 255, 255, 0, 45, 0, false), // Murkrow (soaring)
    guess(Game::ORAS, 265, 0, Shiny::Never, 255, 255, 0, 3, 14, false), // Wurmple (Route 101 rescue, 3DSRNGTool)
    guess(Game::ORAS, 261, 0, Shiny::Random, 255, 255, 0, 5, 8, false) // Poochyena (DexNav tutorial, 3DSRNGTool)
};

// IVs are HP, Atk, Def, SpA, SpD, Spe (PKHeX trade data reordered); gender 0 male, 1 female
constexpr std::array<StaticTemplate6, 12> trades = {
    trade(Game::XY, 129, 5, 0, 0, 3, { 255, 31, 255, 31, 255, 255 }, 44285), // Magikarp (Adamant, Atk/SpA 31)
    trade(Game::XY, 133, 5, 0, 1, 6, { 255, 255, 255, 255, 255, 255 }, 29294), // Eevee (Docile)
    trade(Game::XY, 83, 10, 0, 0, 13, { 255, 255, 255, 255, 255, 31 }, 185), // Farfetch'd (Jolly, Spe 31)
    trade(Game::XY, 208, 20, 0, 1, 8, { 255, 255, 31, 255, 255, 255 }, 19250), // Steelix (Impish, Def 31; live X)
    trade(Game::XY, 625, 50, 0, 0, 3, { 255, 31, 255, 255, 255, 255 }, 3447), // Bisharp (Adamant, Atk 31)
    trade(Game::XY, 656, 5, 0, 0, 13, { 20, 20, 20, 20, 20, 31 }, 37), // Froakie (Jolly)
    trade(Game::XY, 650, 5, 0, 0, 3, { 20, 31, 20, 20, 20, 20 }, 37), // Chespin (Adamant)
    trade(Game::XY, 653, 5, 0, 0, 15, { 20, 20, 20, 31, 20, 20 }, 37), // Fennekin (Modest)
    trade(Game::XY, 280, 5, 0, 1, 15, { 20, 20, 20, 31, 20, 31 }, 37110), // Ralts (Modest)
    trade(Game::ORAS, 296, 9, 1, 0, 2, { 255, 31, 255, 255, 255, 255 }, 30724), // Makuhita (Brave, Atk 31)
    trade(Game::ORAS, 300, 30, 0, 1, 4, { 255, 255, 255, 255, 255, 31 }, 3239), // Skitty (Naughty, Spe 31)
    trade(Game::ORAS, 222, 50, 2, 1, 20, { 31, 255, 255, 255, 31, 255 }, 325) // Corsola (Calm, hidden ability, HP/SpD 31)
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
        case 5:
            return pick(trades);
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
