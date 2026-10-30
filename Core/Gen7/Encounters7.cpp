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

#include "Encounters7.hpp"
#include <Core/Enum/Game.hpp>
#include <Core/Gen7/StaticTemplate7.hpp>
#include <array>

using K = StaticKind7;
constexpr std::array<u8, 6> R6 = { 255, 255, 255, 255, 255, 255 };

// Handoff 4.8. Levels from the games (3DSRNGTool's list where the research did not note them). "measured" = the block was read back live.
constexpr std::array<StaticTemplate7, 14> LEGENDS = {
    StaticTemplate7(Game::Gen7, 785, 0, K::Legend, 60, 3, false, 0, 255, 2), // Tapu Koko (SM + US live)
    StaticTemplate7(Game::Gen7, 786, 0, K::Legend, 60, 3, false, 0, 255, 2), // Tapu Lele
    StaticTemplate7(Game::Gen7, 787, 0, K::Legend, 60, 3, false, 0, 255, 2), // Tapu Bulu (SM + US live)
    StaticTemplate7(Game::Gen7, 788, 0, K::Legend, 60, 3, false, 0, 255, 2), // Tapu Fini (SM + US 1.2 live)
    StaticTemplate7(Game::Sun, 791, 0, K::Legend, 55, 3, false, 0, 255, 2), // Solgaleo, Altar of the Sunne
    StaticTemplate7(Game::Moon, 792, 0, K::Legend, 55, 3, false, 0, 255, 2), // Lunala, Altar of the Moone
    StaticTemplate7(Game::US, 791, 0, K::Legend, 60, 3, false, 0, 255, 2, R6, 0, false), // Solgaleo (USUM, not run)
    StaticTemplate7(Game::UM, 792, 0, K::Legend, 60, 3, false, 0, 255, 2, R6, 0, false), // Lunala (USUM, not run)
    StaticTemplate7(Game::USUM, 800, 0, K::Legend, 75, 3, false, 0, 255, 2), // Necrozma, Mount Lanakila summit (block live)
    StaticTemplate7(Game::USUM, 718, 0, K::Legend, 60, 3, false, 0, 255, 2), // Zygarde, Resolution Cave (block live)
    StaticTemplate7(Game::USUM, 793, 0, K::Legend, 27, 4, false, 0, 255, 2), // Nihilego, Aether Paradise story (4 guaranteed IVs, live)
    StaticTemplate7(Game::Sun, 735, 1, K::Totem, 12, 0, false, 0, 15, 255, { 0, 255, 0, 0, 0, 0 }), // Totem Gumshoos, Verdant Cavern (Atk random, Modest)
    StaticTemplate7(Game::Moon, 20, 2, K::Totem, 12, 0, false, 0, 255, 255, { 0, 255, 0, 0, 0, 0 }, 0, false), // Totem Alolan Raticate (nature not noted)
    StaticTemplate7(Game::SM, 800, 0, K::Legend, 75, 3, false, 0, 255, 2), // Necrozma, Ten Carat Hill special table (Honey; use the Honey window)
};

// Ultra Space Wilds legendaries (USUM), 3DSRNGTool's list with version exclusives; 15 of them read back live by the research (block exact)
constexpr std::array<StaticTemplate7, 48> ULTRA_LEGENDS = {
    StaticTemplate7(Game::USUM, 144, 0, K::Legend, 60, 3, false, 0, 255, 2), // Articuno
    StaticTemplate7(Game::USUM, 145, 0, K::Legend, 60, 3, false, 0, 255, 2), // Zapdos
    StaticTemplate7(Game::USUM, 146, 0, K::Legend, 60, 3, false, 0, 255, 2), // Moltres
    StaticTemplate7(Game::USUM, 150, 0, K::Legend, 60, 3, false, 0, 255, 2), // Mewtwo
    StaticTemplate7(Game::US, 243, 0, K::Legend, 60, 3, false, 0, 255, 2), // Raikou
    StaticTemplate7(Game::UM, 244, 0, K::Legend, 60, 3, false, 0, 255, 2), // Entei
    StaticTemplate7(Game::USUM, 245, 0, K::Legend, 60, 3, false, 0, 255, 2), // Suicune
    StaticTemplate7(Game::UM, 249, 0, K::Legend, 60, 3, false, 0, 255, 2), // Lugia
    StaticTemplate7(Game::US, 250, 0, K::Legend, 60, 3, false, 0, 255, 2), // Ho-Oh
    StaticTemplate7(Game::USUM, 377, 0, K::Legend, 60, 3, false, 0, 255, 2), // Regirock
    StaticTemplate7(Game::USUM, 378, 0, K::Legend, 60, 3, false, 0, 255, 2), // Regice
    StaticTemplate7(Game::USUM, 379, 0, K::Legend, 60, 3, false, 0, 255, 2), // Registeel
    StaticTemplate7(Game::UM, 380, 0, K::Legend, 60, 3, false, 0, 255, 1), // Latias
    StaticTemplate7(Game::US, 381, 0, K::Legend, 60, 3, false, 0, 255, 0), // Latios
    StaticTemplate7(Game::UM, 382, 0, K::Legend, 60, 3, false, 0, 255, 2), // Kyogre
    StaticTemplate7(Game::US, 383, 0, K::Legend, 60, 3, false, 0, 255, 2), // Groudon
    StaticTemplate7(Game::USUM, 384, 0, K::Legend, 60, 3, false, 0, 255, 2), // Rayquaza
    StaticTemplate7(Game::USUM, 480, 0, K::Legend, 60, 3, false, 0, 255, 2), // Uxie
    StaticTemplate7(Game::USUM, 481, 0, K::Legend, 60, 3, false, 0, 255, 2), // Mesprit
    StaticTemplate7(Game::USUM, 482, 0, K::Legend, 60, 3, false, 0, 255, 2), // Azelf
    StaticTemplate7(Game::US, 483, 0, K::Legend, 60, 3, false, 0, 255, 2), // Dialga
    StaticTemplate7(Game::UM, 484, 0, K::Legend, 60, 3, false, 0, 255, 2), // Palkia
    StaticTemplate7(Game::US, 485, 0, K::Legend, 60, 3, false, 0, 255, 255), // Heatran
    StaticTemplate7(Game::UM, 486, 0, K::Legend, 60, 3, false, 0, 255, 2), // Regigigas
    StaticTemplate7(Game::USUM, 487, 0, K::Legend, 60, 3, false, 0, 255, 2), // Giratina
    StaticTemplate7(Game::USUM, 488, 0, K::Legend, 60, 3, false, 0, 255, 1), // Cresselia
    StaticTemplate7(Game::USUM, 638, 0, K::Legend, 60, 3, false, 0, 255, 2), // Cobalion
    StaticTemplate7(Game::USUM, 639, 0, K::Legend, 60, 3, false, 0, 255, 2), // Terrakion
    StaticTemplate7(Game::USUM, 640, 0, K::Legend, 60, 3, false, 0, 255, 2), // Virizion
    StaticTemplate7(Game::US, 641, 0, K::Legend, 60, 3, false, 0, 255, 0), // Tornadus
    StaticTemplate7(Game::UM, 642, 0, K::Legend, 60, 3, false, 0, 255, 0), // Thundurus
    StaticTemplate7(Game::USUM, 645, 0, K::Legend, 60, 3, false, 0, 255, 0), // Landorus
    StaticTemplate7(Game::US, 643, 0, K::Legend, 60, 3, false, 0, 255, 2), // Reshiram
    StaticTemplate7(Game::UM, 644, 0, K::Legend, 60, 3, false, 0, 255, 2), // Zekrom
    StaticTemplate7(Game::USUM, 646, 0, K::Legend, 60, 3, false, 0, 255, 2), // Kyurem
    StaticTemplate7(Game::US, 716, 0, K::Legend, 60, 3, false, 0, 255, 2), // Xerneas
    StaticTemplate7(Game::UM, 717, 0, K::Legend, 60, 3, false, 0, 255, 2), // Yveltal
    StaticTemplate7(Game::USUM, 793, 0, K::Legend, 60, 3, false, 0, 255, 2), // Nihilego
    StaticTemplate7(Game::US, 794, 0, K::Legend, 60, 3, false, 0, 255, 2), // Buzzwole
    StaticTemplate7(Game::UM, 795, 0, K::Legend, 60, 3, false, 0, 255, 2), // Pheromosa
    StaticTemplate7(Game::USUM, 796, 0, K::Legend, 60, 3, false, 0, 255, 2), // Xurkitree
    StaticTemplate7(Game::UM, 797, 0, K::Legend, 60, 3, false, 0, 255, 2), // Celesteela
    StaticTemplate7(Game::US, 798, 0, K::Legend, 60, 3, false, 0, 255, 2), // Kartana
    StaticTemplate7(Game::USUM, 799, 0, K::Legend, 60, 3, false, 0, 255, 2), // Guzzlord
    StaticTemplate7(Game::US, 805, 0, K::Legend, 60, 3, false, 0, 255, 2), // Stakataka (Poni Grove special table)
    StaticTemplate7(Game::UM, 806, 0, K::Legend, 60, 3, false, 0, 255, 2), // Blacephalon
    StaticTemplate7(Game::USUM, 800, 0, K::Legend, 60, 3, false, 0, 255, 2, R6, 0, false), // Necrozma (Ultra Space, not run)
    StaticTemplate7(Game::USUM, 718, 0, K::Legend, 60, 3, false, 0, 255, 2, R6, 0, false), // Zygarde (Ultra Space, not run)
};

// Ultra Space Wilds ordinary Pokemon (USUM): 3 guaranteed IVs, shiny-locked, ability drawn (3DSRNGTool; blocks not run by the research)
constexpr std::array<StaticTemplate7, 20> ULTRA_WILDS = {
    StaticTemplate7(Game::USUM, 334, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Altaria
    StaticTemplate7(Game::USUM, 469, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Yanmega
    StaticTemplate7(Game::USUM, 561, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Sigilyph
    StaticTemplate7(Game::USUM, 581, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Swanna
    StaticTemplate7(Game::USUM, 277, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Swellow
    StaticTemplate7(Game::USUM, 452, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Drapion
    StaticTemplate7(Game::USUM, 531, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Audino
    StaticTemplate7(Game::USUM, 695, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Heliolisk
    StaticTemplate7(Game::USUM, 274, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Nuzleaf
    StaticTemplate7(Game::USUM, 326, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Grumpig
    StaticTemplate7(Game::USUM, 460, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Abomasnow
    StaticTemplate7(Game::USUM, 308, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Medicham
    StaticTemplate7(Game::USUM, 450, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Hippowdon
    StaticTemplate7(Game::USUM, 558, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Crustle
    StaticTemplate7(Game::USUM, 219, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Magcargo
    StaticTemplate7(Game::USUM, 689, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Barbaracle
    StaticTemplate7(Game::USUM, 271, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Lombre
    StaticTemplate7(Game::USUM, 618, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Stunfisk
    StaticTemplate7(Game::USUM, 419, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Floatzel
    StaticTemplate7(Game::USUM, 195, 0, K::Legend, 60, 3, true, 0, 255, 255, R6, 0, false), // Quagsire
};

// Handoff 4.9: legend gifts. The first draw is the EC (no lead, no throwaways); Synchronize fixes the nature with no roll
constexpr std::array<StaticTemplate7, 10> LEGEND_GIFTS = {
    StaticTemplate7(Game::SM, 789, 0, K::LegendGift, 5, 3, false, 0, 255, 2), // Cosmog, Lake of the Sunne/Moone (one ability, no draw)
    StaticTemplate7(Game::USUM, 789, 0, K::LegendGift, 5, 3, false, 0, 255, 2), // Cosmog (USUM, after "You received Cosmog!")
    StaticTemplate7(Game::SM, 772, 0, K::LegendGift, 40, 3, true, 0, 255, 2), // Type: Null, Aether Paradise (Gladion)
    StaticTemplate7(Game::USUM, 772, 0, K::LegendGift, 40, 3, true, 0, 255, 2), // Type: Null (Wicke)
    StaticTemplate7(Game::USUM, 803, 0, K::LegendGift, 40, 3, true, 0, 255, 2), // Poipole
    StaticTemplate7(Game::Gen7, 801, 0, K::LegendGift, 50, 3, false, 0, 255, 2), // Magearna (QR), no ability draw
    StaticTemplate7(Game::SM, 718, 0, K::LegendGift, 30, 3, true, 0, 255, 2), // Zygarde 50 %, Reassembly Unit (Route 16)
    StaticTemplate7(Game::USUM, 25, 0, K::LegendGift, 40, 3, true), // Surf Pikachu (Mantine Surf), ability drawn, gendered
    StaticTemplate7(Game::USUM, 718, 1, K::LegendGift, 63, 3, true, 0, 255, 2, R6, 0, false), // Zygarde 10 % (USUM, not run)
    StaticTemplate7(Game::USUM, 718, 0, K::LegendGift, 50, 3, true, 0, 255, 2, R6, 0, false), // Zygarde 50 % (USUM, not run)
};

// Handoff 4.9: wild-style gifts (EC, PID with 1 or 3 rolls, 6 IVs, ability, nature, gender); fossils are revived at the Route 8 trailer
constexpr std::array<StaticTemplate7, 18> WILD_GIFTS = {
    StaticTemplate7(Game::SM, 137, 0, K::WildGift, 30, 0, true, 0, 255, 2), // Porygon, Aether House
    StaticTemplate7(Game::SM, 142, 0, K::WildGift, 40, 0, true), // Aerodactyl, Seafolk Village
    StaticTemplate7(Game::Gen7, 133, 0, K::WildGift, 1, 0, true), // Eevee egg, Paniola Nursery (main RNG when taken)
    StaticTemplate7(Game::USUM, 722, 0, K::WildGift, 5, 0, true), // Rowlet (Kukui)
    StaticTemplate7(Game::USUM, 725, 0, K::WildGift, 5, 0, true), // Litten
    StaticTemplate7(Game::USUM, 728, 0, K::WildGift, 5, 0, true), // Popplio
    StaticTemplate7(Game::Gen7, 408, 0, K::WildGift, 15, 0, true), // Cranidos
    StaticTemplate7(Game::Gen7, 410, 0, K::WildGift, 15, 0, true), // Shieldon
    StaticTemplate7(Game::Gen7, 564, 0, K::WildGift, 15, 0, true), // Tirtouga
    StaticTemplate7(Game::Gen7, 566, 0, K::WildGift, 15, 0, true), // Archen
    StaticTemplate7(Game::Gen7, 138, 0, K::WildGift, 15, 0, true), // Omanyte
    StaticTemplate7(Game::Gen7, 140, 0, K::WildGift, 15, 0, true), // Kabuto
    StaticTemplate7(Game::Gen7, 142, 0, K::WildGift, 15, 0, true), // Aerodactyl (fossil)
    StaticTemplate7(Game::Gen7, 345, 0, K::WildGift, 15, 0, true), // Lileep
    StaticTemplate7(Game::Gen7, 347, 0, K::WildGift, 15, 0, true), // Anorith
    StaticTemplate7(Game::Gen7, 696, 0, K::WildGift, 15, 0, true), // Tyrunt
    StaticTemplate7(Game::Gen7, 698, 0, K::WildGift, 15, 0, true), // Amaura
    StaticTemplate7(Game::USUM, 25, 0, K::WildGift, 21, 0, false, 1, 0, 1, R6, 1009, false), // Movie Pikachu (OT TSV 1009, Hardy, female; not run)
};

// Handoff 4.11: in-game trades. SM from the RomFS table (OT ID -> TSV), USUM from 3DSRNGTool's list (OT TSV). Every trade fixes one IV.
constexpr std::array<StaticTemplate7, 14> TRADES = {
    StaticTemplate7(Game::SM, 66, 0, K::Trade, 9, 0, false, 1, 2, 0, { 255, 31, 255, 255, 255, 255 }, 410 ^ 0), // Machop, Route 2 PC (OT 410/0)
    StaticTemplate7(Game::SM, 761, 0, K::Trade, 16, 0, false, 0, 3, 1, { 255, 31, 255, 255, 255, 255 }, 20683 ^ 9), // Bounsweet, Heahea City
    StaticTemplate7(Game::SM, 61, 0, K::Trade, 22, 0, false, 1, 4, 1, { 31, 255, 255, 255, 255, 255 }, 1092 ^ 9), // Poliwhirl, Route 5 PC
    StaticTemplate7(Game::SM, 440, 0, K::Trade, 27, 0, false, 1, 20, 1, { 255, 255, 255, 31, 255, 255 }, 10913 ^ 0), // Happiny, Route 8
    StaticTemplate7(Game::SM, 75, 1, K::Trade, 32, 0, false, 0, 8, 0, { 255, 255, 31, 255, 255, 255 }, 20778 ^ 9), // Alolan Graveler, Konikoni PC
    StaticTemplate7(Game::SM, 762, 0, K::Trade, 43, 0, false, 0, 23, 1, { 255, 255, 255, 255, 31, 255 }, 20679 ^ 9), // Steenee, Malie City
    StaticTemplate7(Game::SM, 663, 0, K::Trade, 59, 0, false, 2, 13, 0, { 255, 255, 255, 255, 255, 31 }, 56734 ^ 8), // Talonflame, Route 16 PC
    StaticTemplate7(Game::USUM, 701, 0, K::Trade, 8, 0, false, 1, 2, 1, { 255, 31, 255, 255, 255, 255 }, 25), // Hawlucha, Route 2 PC (live)
    StaticTemplate7(Game::USUM, 714, 0, K::Trade, 19, 0, false, 0, 15, 1, { 255, 255, 255, 31, 255, 255 }, 1292, false), // Noibat
    StaticTemplate7(Game::USUM, 339, 0, K::Trade, 21, 0, false, 1, 4, 0, { 31, 255, 255, 255, 255, 255 }, 68, false), // Barboach
    StaticTemplate7(Game::USUM, 24, 0, K::Trade, 22, 0, false, 0, 8, 0, { 255, 255, 31, 255, 255, 255 }, 682, false), // Arbok
    StaticTemplate7(Game::USUM, 709, 0, K::Trade, 33, 0, false, 0, 20, 1, { 255, 255, 255, 255, 31, 255 }, 1298, false), // Phantump
    StaticTemplate7(Game::USUM, 422, 0, K::Trade, 44, 0, false, 1, 17, 0, { 255, 255, 31, 255, 255, 255 }, 1292, false), // Shellos
    StaticTemplate7(Game::USUM, 128, 0, K::Trade, 59, 0, false, 0, 13, 1, { 255, 255, 255, 255, 255, 31 }, 3545, false), // Tauros
};

namespace Encounters7
{
    const StaticTemplate7 *getStaticEncounters(int category, int *size)
    {
        switch (static_cast<StaticCategory7>(category))
        {
        case StaticCategory7::Legends:
            if (size)
            {
                *size = LEGENDS.size();
            }
            return LEGENDS.data();
        case StaticCategory7::UltraSpaceLegends:
            if (size)
            {
                *size = ULTRA_LEGENDS.size();
            }
            return ULTRA_LEGENDS.data();
        case StaticCategory7::UltraSpaceWilds:
            if (size)
            {
                *size = ULTRA_WILDS.size();
            }
            return ULTRA_WILDS.data();
        case StaticCategory7::LegendGifts:
            if (size)
            {
                *size = LEGEND_GIFTS.size();
            }
            return LEGEND_GIFTS.data();
        case StaticCategory7::WildGifts:
            if (size)
            {
                *size = WILD_GIFTS.size();
            }
            return WILD_GIFTS.data();
        case StaticCategory7::Trades:
        default:
            if (size)
            {
                *size = TRADES.size();
            }
            return TRADES.data();
        }
    }

    const StaticTemplate7 *getStaticEncounter(int category, int index)
    {
        return &getStaticEncounters(category)[index];
    }
}
