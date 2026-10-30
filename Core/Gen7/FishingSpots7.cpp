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

#include "FishingSpot7.hpp"
#include <Core/Enum/Game.hpp>

// Fishing spot records (ED section 12 "FS": 66 in Sun/Moon, 62 in Ultra Sun/Ultra Moon) with the standing positions and the
// cast -> bite constants the Gen 7 RNG research verified (63 of 66 Moon spots, 112 casts; 48 USUM spots, 77 casts; Appendix B9.7
// of the handoff). Generated from results/fishspots.txt, results/us_fishspots.txt and fishing_spot_presets.tsv.
// Rare (bubbling) tables: Moon's Hau'oli spot 7 has its own (table 5, rate 100); every USUM record has a rare entry (rate 60)
// whose table is the area's table with the same minimum level and a maximum 5 higher.

constexpr std::array<FishingSpot7, 128> spots = {
    FishingSpot7 { Game::SM, 6, 4, 255, 0, 48, "Melemele Sea", { 33305, 19606, -1320 }, { 33305, 19784, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 10, 3, 7, 60, 48, "Melemele Sea", { 33305, 19606, -1320 }, { 33305, 19784, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 6, 5, 255, 0, 48, "Melemele Sea", { 31769, 21701, -1320 }, { 31769, 21879, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 10, 4, 7, 60, 48, "Melemele Sea", { 31769, 21701, -1320 }, { 31769, 21879, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 6, 6, 255, 0, 48, "Melemele Sea", { 33886, 27460, -1320 }, { 33886, 27638, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 10, 5, 7, 60, 48, "Melemele Sea", { 33886, 27460, -1320 }, { 33886, 27638, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 6, 7, 255, 0, 48, "Melemele Sea", { 31498, 24031, -1320 }, { 31498, 24209, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 10, 6, 7, 60, 48, "Melemele Sea", { 31498, 24031, -1320 }, { 31498, 24209, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 7, 1, 255, 0, 48, "Hau'oli City", { 21019, 23014, -1320 }, { 21019, 23192, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 12, 1, 5, 60, 48, "Hau'oli City", { 21019, 23014, -1320 }, { 21019, 23192, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 7, 2, 255, 0, 48, "Hau'oli City", { 20087, 24489, -1320 }, { 20087, 24667, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 12, 2, 5, 60, 48, "Hau'oli City", { 20087, 24489, -1320 }, { 20087, 24667, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 7, 3, 255, 0, 48, "Hau'oli City", { 22947, 28171, -1320 }, { 22947, 28349, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 12, 3, 5, 60, 48, "Hau'oli City", { 22947, 28171, -1320 }, { 22947, 28349, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 7, 4, 5, 100, 48, "Hau'oli City", { 20756, 26354, -1320 }, { 20756, 26532, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 12, 4, 5, 60, 48, "Hau'oli City", { 20756, 26354, -1320 }, { 20756, 26532, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 2, 4, 255, 0, 48, "Route 3", { 21414, 9427, -1320 }, { 21414, 9605, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 6, 4, 8, 60, 48, "Route 3", { 21414, 9427, -1320 }, { 21414, 9605, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 2, 5, 255, 0, 48, "Route 3", { 25159, 9471, -1320 }, { 25159, 9649, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 6, 5, 8, 60, 48, "Route 3", { 25159, 9471, -1320 }, { 25159, 9649, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 2, 6, 255, 0, 48, "Route 3", { 23820, 11298, -1320 }, { 23820, 11476, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 6, 6, 8, 60, 48, "Route 3", { 23820, 11298, -1320 }, { 23820, 11476, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 2, 7, 255, 0, 48, "Route 3", { 22695, 7704, -1320 }, { 22695, 7882, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 6, 7, 8, 60, 48, "Route 3", { 22695, 7704, -1320 }, { 22695, 7882, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 22, 2, 255, 0, 48, "Seaward Cave", { 2370, 3923, -360 }, { 2370, 4101, -360 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 30, 2, 3, 60, 48, "Seaward Cave", { 2370, 3923, -360 }, { 2370, 4101, -360 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 76, 0, 255, 0, 48, "Paniola Town", { 17186, 23312, 148 }, { 17186, 23052, 148 }, "side S, 260 from the spot, facing it", true, true, true },
    FishingSpot7 { Game::USUM, 89, 0, 1, 60, 48, "Paniola Town", { 17186, 23312, 148 }, { 17186, 23052, 148 }, "side S, 260 from the spot, facing it; not cast in USUM", true, false, true },
    FishingSpot7 { Game::SM, 71, 4, 255, 0, 48, "Route 7", { 30324, 16463, -20 }, { 30324, 16641, -20 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 83, 4, 6, 60, 48, "Route 7", { 30324, 16463, -20 }, { 30324, 16641, -20 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 71, 3, 255, 0, 48, "Route 7", { 31860, 14105, -24 }, { 31860, 14283, -24 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 83, 3, 6, 60, 48, "Route 7", { 31860, 14105, -24 }, { 31860, 14283, -24 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 71, 1, 255, 0, 48, "Route 7", { 30363, 10121, -24 }, { 30363, 10299, -24 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 83, 1, 6, 60, 48, "Route 7", { 30363, 10121, -24 }, { 30363, 10299, -24 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 71, 2, 255, 0, 48, "Route 7", { 32101, 10577, -24 }, { 32101, 10755, -24 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 83, 2, 6, 60, 48, "Route 7", { 32101, 10577, -24 }, { 32101, 10755, -24 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 71, 5, 255, 0, 48, "Route 7", { 33250, 15665, 12 }, { 33250, 15925, 92 }, "side N, 260 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::USUM, 83, 5, 6, 60, 48, "Route 7", { 33250, 15665, 12 }, { 33250, 15925, 92 }, "side N, 260 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::SM, 72, 1, 255, 0, 48, "Route 8", { 28124, 5959, 0 }, { 28124, 6137, 0 }, "no cast from 52-64 positions: unreachable in the data", false, false, false },
    FishingSpot7 { Game::USUM, 84, 1, 4, 60, 48, "Route 8", { 28124, 5959, 0 }, { 28124, 6137, 0 }, "no cast reached in Moon from 52-64 positions; not cast in USUM", false, false, false },
    FishingSpot7 { Game::SM, 72, 2, 255, 0, 48, "Route 8", { 15585, 12750, -29 }, { 15585, 12928, -29 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 84, 2, 4, 60, 48, "Route 8", { 15585, 12750, -20 }, { 15585, 12928, -29 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 72, 3, 255, 0, 48, "Route 8", { 16145, 14489, -29 }, { 16145, 14667, -29 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 84, 3, 4, 60, 48, "Route 8", { 16145, 14489, -20 }, { 16145, 14667, -29 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 73, 0, 255, 0, 48, "Route 9", { 10348, 40263, 0 }, { 10348, 40441, 0 }, "no cast from 52-64 positions: unreachable in the data", false, false, false },
    FishingSpot7 { Game::USUM, 85, 0, 1, 60, 48, "Route 9", { 10348, 40263, 0 }, { 10348, 40441, 0 }, "no cast reached in Moon from 52-64 positions; not cast in USUM", false, false, false },
    FishingSpot7 { Game::SM, 84, 3, 255, 0, 48, "Akala Outskirts", { 7325, 3134, -413 }, { 7325, 3312, -413 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 98, 3, 5, 60, 48, "Akala Outskirts", { 7325, 3134, -413 }, { 7325, 3312, -413 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 84, 4, 255, 0, 48, "Akala Outskirts", { 7251, 2107, -413 }, { 7251, 2285, -413 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 98, 4, 5, 60, 48, "Akala Outskirts", { 7251, 2107, -413 }, { 7251, 2285, -413 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 88, 2, 255, 0, 48, "Brooklet Hill", { 7918, 2121, -331 }, { 7918, 2299, -331 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 102, 2, 255, 60, 48, "Brooklet Hill", { 7918, 2121, -331 }, { 7918, 2299, -331 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 88, 3, 255, 0, 48, "Brooklet Hill", { 7750, 3263, -331 }, { 7750, 3441, -331 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 102, 3, 255, 60, 48, "Brooklet Hill", { 7750, 3263, -331 }, { 7750, 3441, -331 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 88, 4, 255, 0, 48, "Brooklet Hill", { 8926, 3334, -331 }, { 8926, 3512, -331 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 102, 4, 255, 60, 48, "Brooklet Hill", { 8926, 3334, -331 }, { 8926, 3512, -331 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 89, 5, 255, 0, 48, "Brooklet Hill", { 4979, 6181, -105 }, { 4979, 6359, -105 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 103, 5, 255, 60, 48, "Brooklet Hill", { 4979, 6181, -105 }, { 4979, 6359, -105 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 89, 6, 255, 0, 48, "Brooklet Hill", { 4489, 7786, -105 }, { 4489, 7964, -105 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 103, 6, 255, 60, 48, "Brooklet Hill", { 4489, 7786, -105 }, { 4489, 7964, -105 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 89, 7, 255, 0, 48, "Brooklet Hill", { 3534, 6550, -105 }, { 3534, 6728, -105 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 103, 7, 255, 60, 48, "Brooklet Hill", { 3534, 6550, -105 }, { 3534, 6728, -105 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 89, 8, 255, 0, 48, "Brooklet Hill", { 5141, 10420, -505 }, { 5141, 10598, -505 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 103, 8, 255, 60, 48, "Brooklet Hill", { 5141, 10420, -505 }, { 5141, 10598, -505 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 89, 9, 255, 0, 48, "Brooklet Hill", { 5777, 11327, -505 }, { 5777, 11505, -505 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 103, 9, 255, 60, 48, "Brooklet Hill", { 5777, 11327, -505 }, { 5777, 11505, -505 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 89, 10, 255, 0, 48, "Brooklet Hill", { 4342, 11062, -505 }, { 4342, 11240, -505 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 103, 10, 255, 60, 48, "Brooklet Hill", { 4342, 11062, -505 }, { 4342, 11240, -505 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 90, 1, 255, 0, 48, "Brooklet Hill", { 4794, 3343, -31 }, { 4794, 3521, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 104, 1, 7, 60, 48, "Brooklet Hill", { 4794, 3343, -31 }, { 4794, 3521, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 90, 2, 255, 0, 48, "Brooklet Hill", { 3654, 3816, -31 }, { 3654, 3994, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 104, 2, 7, 60, 48, "Brooklet Hill", { 3654, 3816, -31 }, { 3654, 3994, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 90, 3, 255, 0, 48, "Brooklet Hill", { 2261, 3954, -31 }, { 2261, 4132, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 104, 3, 7, 60, 48, "Brooklet Hill", { 2261, 3954, -31 }, { 2261, 4132, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 90, 4, 255, 0, 48, "Brooklet Hill", { 3987, 4925, -31 }, { 3987, 5103, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 104, 4, 7, 60, 48, "Brooklet Hill", { 3987, 4925, -31 }, { 3987, 5103, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 90, 5, 255, 0, 48, "Brooklet Hill", { 2232, 5316, -31 }, { 2232, 5494, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 104, 5, 7, 60, 48, "Brooklet Hill", { 2232, 5316, -31 }, { 2232, 5494, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 90, 6, 255, 0, 48, "Brooklet Hill", { 3069, 4678, -31 }, { 3069, 4856, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 104, 6, 7, 60, 48, "Brooklet Hill", { 3069, 4678, -31 }, { 3069, 4856, -31 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 142, 2, 255, 0, 48, "Ula'ula Beach", { 34118, 32507, 20 }, { 34118, 32685, 20 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 143, 0, 255, 0, 67, "Route 13", { 19767, 35855, 0 }, { 19945, 35855, 0 }, "side W, 178 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::USUM, 167, 0, 2, 60, 59, "Route 13", { 19767, 35855, 0 }, { 19945, 35855, 0 }, "side W, 178 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::SM, 143, 1, 255, 0, 67, "Route 13", { 21284, 35735, 0 }, { 21284, 35557, 0 }, "side S, 178 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::USUM, 167, 1, 2, 60, 59, "Route 13", { 21284, 35735, 0 }, { 21284, 35557, 0 }, "side S, 178 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::SM, 145, 1, 255, 0, 48, "Route 14", { 10142, 37822, -25 }, { 10142, 38000, -25 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 169, 1, 6, 60, 48, "Route 14", { 10142, 37822, -25 }, { 10142, 38000, -25 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 145, 2, 255, 0, 48, "Route 14", { 11779, 37799, -25 }, { 11779, 37977, -25 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 169, 2, 6, 60, 48, "Route 14", { 11779, 37799, -25 }, { 11779, 37977, -25 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 145, 3, 255, 0, 48, "Route 14", { 12381, 39689, -25 }, { 12381, 39867, -25 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 169, 3, 6, 60, 48, "Route 14", { 12381, 39689, -25 }, { 12381, 39867, -25 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 145, 4, 255, 0, 48, "Route 14", { 14743, 41237, -25 }, { 14743, 41415, -25 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 169, 4, 6, 60, 48, "Route 14", { 14743, 41237, -25 }, { 14743, 41415, -25 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 145, 5, 255, 0, 48, "Route 14", { 10221, 39947, -20 }, { 10221, 40125, -20 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 169, 5, 6, 60, 48, "Route 14", { 10221, 39947, -20 }, { 10221, 40125, -20 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 146, 1, 255, 0, 48, "Route 15", { 2749, 26287, -4 }, { 2749, 26465, -4 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 170, 1, 3, 60, 48, "Route 15", { 2749, 26287, -4 }, { 2749, 26465, -4 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 146, 2, 255, 0, 48, "Route 15", { 7510, 31015, -4 }, { 7510, 31193, -4 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 170, 2, 3, 60, 48, "Route 15", { 7510, 31015, -4 }, { 7510, 31193, -4 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 162, 1, 255, 0, 48, "Malie Garden", { 3675, 4065, -75 }, { 3675, 4415, 5 }, "side N, 350 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::USUM, 186, 1, 7, 60, 48, "Malie Garden", { 3675, 4065, -75 }, { 3675, 4415, 5 }, "side N, 350 from the spot, facing it; not cast in USUM", true, false, false },
    FishingSpot7 { Game::SM, 162, 2, 255, 0, 48, "Malie Garden", { 6738, 2880, -75 }, { 7088, 2880, 5 }, "side W, 350 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::USUM, 186, 2, 7, 60, 48, "Malie Garden", { 6738, 2880, -75 }, { 7088, 2880, 5 }, "side W, 350 from the spot, facing it; not cast in USUM", true, false, false },
    FishingSpot7 { Game::SM, 162, 3, 255, 0, 48, "Malie Garden", { 5553, 6157, -75 }, { 5553, 6507, 5 }, "side N, 350 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::USUM, 186, 3, 7, 60, 48, "Malie Garden", { 5553, 6157, -75 }, { 5553, 6507, 5 }, "side N, 350 from the spot, facing it; not cast in USUM", true, false, false },
    FishingSpot7 { Game::SM, 162, 6, 255, 0, 48, "Malie Garden", { 3992, 2220, -75 }, { 3992, 2480, 5 }, "side N, 260 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::USUM, 186, 6, 7, 60, 48, "Malie Garden", { 3992, 2220, -75 }, { 3992, 2480, 5 }, "side N, 260 from the spot, facing it; not cast in USUM", true, false, false },
    FishingSpot7 { Game::SM, 238, 0, 255, 0, 67, "Seafolk Village", { 20348, 20281, -1307 }, { 20348, 20103, -1307 }, "side S, 178 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::USUM, 272, 0, 1, 60, 59, "Seafolk Village", { 20348, 20281, -1307 }, { 20348, 20103, -1307 }, "side S, 178 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::SM, 231, 5, 255, 0, 48, "Poni Wilds", { 14283, 12831, -1320 }, { 14283, 13009, -1320 }, "side N, 178 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::SM, 231, 6, 255, 0, 48, "Poni Wilds", { 13926, 14696, -1320 }, { 13926, 14874, -1320 }, "side N, 178 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::SM, 231, 7, 255, 0, 48, "Poni Wilds", { 14308, 15632, -1320 }, { 14308, 15810, -1320 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 233, 1, 255, 0, 48, "Poni Breaker Coast", { 25888, 19458, -1317 }, { 25888, 19280, -1317 }, "side S, 178 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::SM, 233, 2, 255, 0, 48, "Poni Breaker Coast", { 28097, 19937, -1317 }, { 28097, 19677, -1317 }, "side S, 260 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::SM, 233, 3, 255, 0, 48, "Poni Breaker Coast", { 29988, 19963, -1317 }, { 29728, 19963, -1317 }, "side E, 260 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::SM, 237, 1, 255, 0, 48, "Poni Gauntlet", { 28126, 4884, -1282 }, { 28126, 4624, -1282 }, "side S, 260 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::USUM, 270, 1, 3, 60, 48, "Poni Gauntlet", { 28126, 4884, -1282 }, { 28126, 4624, -1282 }, "side S, 260 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::SM, 237, 2, 255, 0, 67, "Poni Gauntlet", { 29810, 5204, -1296 }, { 30070, 5204, -1296 }, "side W, 260 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::USUM, 270, 2, 3, 60, 59, "Poni Gauntlet", { 29810, 5204, -1296 }, { 30070, 5204, -1296 }, "side W, 260 from the spot, facing it", true, true, false },
    FishingSpot7 { Game::SM, 239, 1, 255, 0, 48, "Poni Meadow", { 3098, 6950, -116 }, { 3098, 7128, -116 }, "no cast from 52-64 positions: unreachable in the data", false, false, false },
    FishingSpot7 { Game::USUM, 273, 1, 2, 60, 48, "Poni Meadow", { 3098, 6950, -116 }, { 3098, 7128, -116 }, "no cast reached in Moon from 52-64 positions; not cast in USUM", false, false, false },
    FishingSpot7 { Game::SM, 244, 3, 255, 0, 48, "Vast Poni Canyon", { 6061, 3075, -20 }, { 6061, 3253, -20 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 278, 3, 5, 60, 48, "Vast Poni Canyon", { 6061, 3075, -20 }, { 6061, 3253, -20 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::SM, 244, 4, 255, 0, 48, "Vast Poni Canyon", { 3393, 2438, -20 }, { 3393, 2616, -20 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 278, 4, 5, 60, 48, "Vast Poni Canyon", { 3393, 2438, -20 }, { 3393, 2616, -20 }, "178 south, facing north", true, true, false },
    FishingSpot7 { Game::USUM, 266, 4, 7, 60, 48, "Poni Breaker Coast", { 25891, 19435, -1317 }, { 25891, 19613, -1317 }, "USUM-only record (moved spot); standing position assumed 178 south, not cast", true, false, false },
    FishingSpot7 { Game::USUM, 266, 5, 7, 60, 48, "Poni Breaker Coast", { 28111, 19934, -1317 }, { 28111, 20112, -1317 }, "USUM-only record (moved spot); standing position assumed 178 south, not cast", true, false, false },
    FishingSpot7 { Game::USUM, 266, 6, 7, 60, 48, "Poni Breaker Coast", { 30017, 19912, -1317 }, { 30017, 20090, -1317 }, "USUM-only record (moved spot); standing position assumed 178 south, not cast", true, false, false },
};

namespace FishingSpots7
{
    std::vector<const FishingSpot7 *> getSpots(Game version)
    {
        std::vector<const FishingSpot7 *> result;
        for (const FishingSpot7 &spot : spots)
        {
            if ((spot.games & version) != Game::None)
            {
                result.emplace_back(&spot);
            }
        }
        return result;
    }
}
