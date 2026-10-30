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

#include "Spot7.hpp"
#include <Core/Enum/Game.hpp>
#include <algorithm>

// Honey spot presets from the Gen 7 RNG research (handoff 2026-10-30, Appendix B9): PRE-Moon (61 tiles, clean saves, Sun = Moon),
// PRE-US and PRE-UM (52 usable tiles each) and the USUM-only areas (measured in Ultra Sun). Generated from the handoff tables.
// Fields: games, zone, encounter table, location, tile (x, z, height), Continue L / persistent mask / load draws, Honey F /
// persistent mask / pre draws / D, first fidget frame, flags, hardware timer offset + fps (New 3DS Moon 1.2 at 268 MHz, where
// measured), Azahar timer offset (where measured), note.

constexpr std::array<Spot7, 200> spots = {
    Spot7 { Game::SM, 0, 3, "Route 1", { 29632, 19399, -1192 }, 23, 0x00600001u, 0, 23, 0x00600001u, 0, 93, 4, 0, false, 0.0f, 29.913f, true, -89.3f, "SM: 3 trials" },
    Spot7 { Game::SM, 2, 1, "Route 3", { 21840, 11488, -1538 }, 15, 0x00000001u, 0, 1, 0x00000001u, 0, 93, 4, 0, true, -123.2f, 30.04f, true, -72.4f, "SM: 3/3 logged Honeys" },
    Spot7 { Game::SM, 2, 14, "Route 3", { 27967, 9128, -1415 }, 15, 0x00000001u, 0, 1, 0x00000001u, 0, 93, 4, 0, true, -123.2f, 30.04f, true, -72.4f, "SM: 3/3 logged Honeys" },
    Spot7 { Game::SM, 4, 0, "Kala'e Bay", { 17563, 3527, 232 }, 1, 0x00000001u, 1, 15, 0x00000101u, 0, 93, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "SM: moving NPC): calibrate per attempt" },
    Spot7 { Game::SM, 4, 13, "Kala'e Bay", { 21086, 11528, -26 }, 1, 0x00000001u, 1, 15, 0x00000401u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 3/3 logged Honeys" },
    Spot7 { Game::SM, 5, 0, "Melemele Sea", { 17700, 22803, -1316 }, 4, 0x00000001u, 1, 16, 0x00000237u, 0, 93, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "SM: moving-model spot, like Haina Desert): calibrate per attempt or avoid" },
    Spot7 { Game::SM, 18, 0, "Ten Carat Hill", { 6001, 6348, 0 }, 2, 0x00000001u, 0, 2, 0x00000001u, 0, 93, 4, 0, true, -100.7f, 29.9f, true, -62.2f, "SM: 3 trials" },
    Spot7 { Game::SM, 18, 1, "Ten Carat Hill", { 6714, 6865, -48 }, 2, 0x00000001u, 0, 2, 0x00000001u, 0, 93, 4, 0, true, -100.7f, 29.9f, true, -62.2f, "SM: 1 trials" },
    Spot7 { Game::SM, 19, 0, "Ten Carat Hill", { 4050, 7503, 0 }, 1, 0x00000001u, 0, 1, 0x00000001u, 0, 93, 4, 0, true, -100.7f, 29.9f, true, -62.2f, "SM: 3 trials" },
    Spot7 { Game::SM, 20, 0, "Hau'oli Cemetery", { 1270, 2591, 4 }, 6, 0x0000002fu, 0, 6, 0x0000002fu, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 3 trials" },
    Spot7 { Game::SM, 21, 0, "Melemele Meadow", { 2032, 2079, -96 }, 5, 0x0000001bu, 0, 5, 0x0000001bu, 0, 93, 4, 0, true, -108.2f, 29.9f, true, -63.8f, "SM: 3 trials" },
    Spot7 { Game::SM, 22, 0, "Seaward Cave", { 4151, 4357, -608 }, 1, 0x00000001u, 1, 1, 0x00000001u, 0, 93, 4, 0, true, -101.5f, 29.95f, true, -62.1f, "SM: 3 trials" },
    Spot7 { Game::SM, 22, 1, "Seaward Cave", { 2038, 3535, -547 }, 1, 0x00000001u, 1, 1, 0x00000001u, 0, 93, 4, 0, true, -101.5f, 29.95f, true, -62.1f, "SM: 2 trials" },
    Spot7 { Game::SM, 53, 0, "Route 1", { 929, 2562, -217 }, 9, 0x00000001u, 0, 9, 0x00000001u, 0, 93, 4, 0, true, -111.5f, 29.86f, true, -67.8f, "SM: 3 trials" },
    Spot7 { Game::SM, 68, 0, "Route 4", { 17895, 26038, 214 }, 6, 0x00000031u, 0, 6, 0x00000021u, 0, 93, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "SM: 3/3 logged Honeys" },
    Spot7 { Game::SM, 70, 0, "Route 6", { 20323, 23370, 214 }, 11, 0x00000601u, 0, 11, 0x00000601u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 3 trials" },
    Spot7 { Game::SM, 70, 1, "Route 6", { 20220, 27124, 214 }, 11, 0x00000181u, 0, 11, 0x00000181u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2 trials" },
    Spot7 { Game::SM, 72, 6, "Route 8", { 15475, 13383, -16 }, 16, 0x00005801u, 1, 16, 0x00005801u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 3 trials" },
    Spot7 { Game::SM, 72, 7, "Route 8", { 25406, 6512, 193 }, 16, 0x00000081u, 1, 16, 0x00000001u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2/2 logged Honeys" },
    Spot7 { Game::SM, 83, 0, "Memorial Hill", { 1461, 2518, -128 }, 12, 0x00000801u, 0, 12, 0x00000801u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2 trials" },
    Spot7 { Game::SM, 84, 2, "Akala Outskirts", { 5978, 4170, -188 }, 7, 0x00000061u, 1, 7, 0x00000061u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 3 trials" },
    Spot7 { Game::SM, 85, 1, "Diglett's Tunnel", { 5116, 3743, 4 }, 13, 0x00000009u, 0, 13, 0x00000009u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2 trials" },
    Spot7 { Game::SM, 86, 0, "Wela Volcano Park", { 7041, 11332, -950 }, 9, 0x00000001u, 0, 9, 0x00000001u, 0, 93, 4, Spot7::FidgetSingle, false, 0.0f, 29.913f, false, 0.0f, "SM: 3 trials" },
    Spot7 { Game::SM, 88, 0, "Brooklet Hill", { 6094, 4382, -144 }, 7, 0x00000021u, 1, 7, 0x00000021u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 88, 1, "Brooklet Hill", { 8062, 2750, -327 }, 7, 0x0000007fu, 1, 7, 0x0000007fu, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2 trials" },
    Spot7 { Game::SM, 89, 14, "Brooklet Hill", { 5081, 8614, -577 }, 5, 0x0000000du, 1, 5, 0x0000000du, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2 trials" },
    Spot7 { Game::SM, 90, 0, "Brooklet Hill", { 3472, 4487, -27 }, 2, 0x00000001u, 1, 2, 0x00000001u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 91, 0, "Lush Jungle", { 2530, 2856, 4 }, 7, 0x00000001u, 0, 7, 0x00000001u, 0, 93, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "SM: 3 trials" },
    Spot7 { Game::SM, 95, 0, "Lush Jungle", { 2691, 1794, 4 }, 2, 0x00000003u, 0, 2, 0x00000003u, 0, 93, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 131, 0, "Paniola Ranch", { 3546, 5585, 4 }, 23, 0x00030ec5u, 2, 23, 0x00030ec5u, 2, 93, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "SM: 3/3 logged Honeys" },
    Spot7 { Game::SM, 140, 0, "Route 10", { 15354, 19961, 350 }, 19, 0x00012001u, 0, 19, 0x00012001u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 141, 0, "Route 11", { 19665, 20079, 174 }, 10, 0x00000015u, 0, 10, 0x00000015u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2 trials" },
    Spot7 { Game::SM, 142, 1, "Ula'ula Beach", { 32861, 32237, -16 }, 11, 0x0000057fu, 1, 11, 0x0000057fu, 0, 93, 4, 0, true, -129.5f, 29.09f, true, -86.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 144, 0, "Tapu Village", { 10846, 36001, 257 }, 11, 0x00000001u, 0, 11, 0x00000001u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 145, 7, "Route 14", { 12978, 39966, -16 }, 11, 0x00000301u, 1, 11, 0x00000301u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1/1 logged Honeys" },
    Spot7 { Game::SM, 146, 0, "Route 15", { 10256, 32605, 253 }, 20, 0x00081003u, 1, 20, 0x00081003u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 3 trials" },
    Spot7 { Game::SM, 146, 8, "Route 15", { 2041, 32324, -16 }, 20, 0x00000001u, 1, 20, 0x00000001u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2 trials" },
    Spot7 { Game::SM, 147, 6, "Route 16", { 3998, 24294, 246 }, 10, 0x00000181u, 0, 10, 0x00000181u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 3 trials" },
    Spot7 { Game::SM, 148, 0, "Route 17", { 9117, 14750, 354 }, 10, 0x000000c1u, 2, 10, 0x000000c1u, 2, 93, 4, Spot7::Rain, false, 0.0f, 29.913f, false, 0.0f, "SM: 1/1 logged Honeys" },
    Spot7 { Game::SM, 148, 3, "Route 17", { 11415, 13435, 589 }, 10, 0x00000201u, 2, 10, 0x00000201u, 2, 93, 4, Spot7::Rain, false, 0.0f, 29.913f, false, 0.0f, "SM: 1/1 logged Honeys" },
    Spot7 { Game::SM, 149, 0, "Route 12", { 30227, 22785, 354 }, 11, 0x00000003u, 0, 11, 0x00000003u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2 trials" },
    Spot7 { Game::SM, 151, 0, "Malie City", { 19933, 8916, 347 }, 9, 0x00000043u, 2, 9, 0x00000143u, 0, 93, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "SM: 1/1 logged Honeys" },
    Spot7 { Game::SM, 153, 0, "Haina Desert", { 2121, 2193, 0 }, 4, 0x0000000fu, 0, 4, 0x0000000fu, 0, 93, 4, Spot7::Unsafe | Spot7::FidgetSingle, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 161, 0, "Ula'ula Meadow", { 907, 3648, 260 }, 6, 0x00000011u, 2, 6, 0x00000001u, 2, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1/1 logged Honeys" },
    Spot7 { Game::SM, 162, 0, "Malie Garden", { 5526, 1087, 0 }, 26, 0x00000001u, 1, 26, 0x00000001u, 0, 93, 4, 0, true, -123.8f, 30.01f, true, -75.8f, "SM: 1 trials" },
    Spot7 { Game::SM, 163, 0, "Mount Hokulani", { 6310, 5570, -333 }, 16, 0x00000001u, 0, 16, 0x00000001u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 164, 0, "Blush Mountain", { 2015, 4659, 6 }, 3, 0x00000003u, 0, 3, 0x00000003u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2 trials" },
    Spot7 { Game::SM, 164, 1, "Blush Mountain", { 3528, 4500, 22 }, 3, 0x00000005u, 0, 3, 0x00000005u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 177, 0, "Mount Lanakila", { 6291, 1829, 1 }, 1, 0x00000001u, 0, 1, 0x00000001u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 180, 0, "Mount Lanakila", { 3177, 6036, 606 }, 1, 0x00000001u, 0, 1, 0x00000001u, 0, 93, 4, Spot7::FidgetSingle, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 194, 0, "Thrifty Megamart", { 1654, 2420, 5 }, 1, 0x00000001u, 0, 1, 0x00000001u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 231, 0, "Poni Wilds", { 16677, 12068, -1310 }, 12, 0x00000081u, 1, 12, 0x00000081u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 3 trials" },
    Spot7 { Game::SM, 231, 1, "Poni Wilds", { 14412, 14099, -1316 }, 12, 0x00000c81u, 1, 12, 0x00000c81u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2 trials); tile moved: old tile 14072 14099 -1316 starts an ambush battle at Continue" },
    Spot7 { Game::SM, 232, 5, "Ancient Poni Path", { 24178, 18817, -1196 }, 20, 0x00001831u, 0, 20, 0x00001831u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 234, 0, "Poni Grove", { 26913, 15897, -1460 }, 4, 0x00000009u, 0, 4, 0x00000009u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: X menu blocked" },
    Spot7 { Game::SM, 235, 0, "Poni Plains", { 33693, 15964, -1485 }, 9, 0x000000b5u, 0, 9, 0x00000015u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1/1 logged Honeys" },
    Spot7 { Game::SM, 235, 8, "Poni Plains", { 31108, 15365, -1196 }, 9, 0x000001f9u, 0, 9, 0x000001b9u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2/2 logged Honeys" },
    Spot7 { Game::SM, 237, 0, "Poni Gauntlet", { 31914, 6081, -1357 }, 11, 0x00000331u, 1, 11, 0x00000331u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 3 trials" },
    Spot7 { Game::SM, 240, 0, "Vast Poni Canyon", { 6887, 2153, 1 }, 15, 0x00000001u, 0, 15, 0x00000001u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 244, 0, "Vast Poni Canyon", { 4889, 2053, -37 }, 8, 0x00000005u, 1, 8, 0x00000005u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 1 trials" },
    Spot7 { Game::SM, 244, 6, "Vast Poni Canyon", { 4570, 4388, 1504 }, 8, 0x00000007u, 1, 8, 0x00000007u, 0, 93, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "SM: 2 trials" },
    Spot7 { Game::US, 0, 2, "Route 1", { 29632, 19399, -1192 }, 18, 0x00030001u, 0, 18, 0x00030001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 6, 1, "Route 3", { 21840, 11488, -1538 }, 19, 0x00000001u, 0, 2, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 6, 14, "Route 3", { 27967, 9128, -1415 }, 19, 0x00000001u, 0, 2, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "2/3 trials exact, 1 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::US, 8, 0, "Kala'e Bay", { 17563, 3527, 232 }, 2, 0x00000003u, 0, 19, 0x00000011u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 8, 13, "Kala'e Bay", { 21086, 11528, -26 }, 2, 0x00000001u, 0, 19, 0x00004001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 9, 0, "Melemele Sea", { 17700, 22803, -1316 }, 3, 0x00000001u, 0, 17, 0x00000001u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 26, 0, "Ten Carat Hill", { 6001, 6348, 0 }, 1, 0x00000001u, 0, 1, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 26, 1, "Ten Carat Hill", { 6714, 6865, -48 }, 1, 0x00000001u, 0, 1, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "1/3 trials exact, 2 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::US, 27, 0, "Ten Carat Hill", { 4050, 7503, 0 }, 2, 0x00000001u, 0, 2, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 28, 0, "Hau'oli Cemetery", { 1270, 2591, 4 }, 11, 0x00000601u, 0, 11, 0x00000601u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 29, 0, "Melemele Meadow", { 2032, 2079, -96 }, 6, 0x00000001u, 0, 6, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 30, 0, "Seaward Cave", { 4151, 4357, -608 }, 6, 0x00000001u, 0, 6, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "2/3 trials exact, 1 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::US, 30, 1, "Seaward Cave", { 2038, 3535, -547 }, 6, 0x00000001u, 0, 6, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "2/3 trials exact, 1 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::US, 80, 0, "Route 4", { 17895, 26038, 214 }, 8, 0x00000061u, 0, 8, 0x00000061u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 82, 0, "Route 6", { 20323, 23370, 214 }, 12, 0x00000e01u, 0, 12, 0x00000e01u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 82, 1, "Route 6", { 20220, 27124, 214 }, 12, 0x00000181u, 0, 12, 0x00000181u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 84, 6, "Route 8", { 15475, 13383, -16 }, 26, 0x00b00001u, 0, 26, 0x00300001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 84, 7, "Route 8", { 25406, 6512, 193 }, 26, 0x00000021u, 0, 26, 0x00000021u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 97, 0, "Memorial Hill", { 1461, 2518, -128 }, 12, 0x00000a01u, 0, 12, 0x00000a01u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 98, 2, "Akala Outskirts", { 5978, 4170, -188 }, 10, 0x00000301u, 0, 10, 0x00000301u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 99, 1, "Diglett's Tunnel", { 5116, 3743, 4 }, 13, 0x00000001u, 0, 13, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "2/3 trials exact, 1 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::US, 100, 0, "Wela Volcano Park", { 7041, 11332, -950 }, 11, 0x00000101u, 0, 11, 0x00000101u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 102, 0, "Brooklet Hill", { 6094, 4382, -144 }, 8, 0x00000041u, 0, 8, 0x00000041u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 102, 1, "Brooklet Hill", { 8062, 2750, -327 }, 8, 0x000000c1u, 0, 8, 0x000000c1u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 103, 13, "Brooklet Hill", { 5081, 8614, -577 }, 6, 0x00000019u, 0, 6, 0x00000019u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 104, 0, "Brooklet Hill", { 3472, 4487, -27 }, 2, 0x00000001u, 0, 2, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 105, 0, "Lush Jungle", { 2530, 2856, 4 }, 10, 0x00000001u, 0, 10, 0x00000001u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 109, 0, "Lush Jungle", { 2691, 1794, 4 }, 2, 0x00000003u, 0, 2, 0x00000003u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 150, 0, "Paniola Ranch", { 3546, 5585, 4 }, 24, 0x000c0001u, 0, 24, 0x000c0001u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 164, 0, "Route 10", { 15354, 19961, 350 }, 19, 0x00010001u, 0, 19, 0x00010001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 165, 0, "Route 11", { 19665, 20079, 174 }, 12, 0x00000001u, 0, 12, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "1/1 trials exact" },
    Spot7 { Game::US, 168, 0, "Tapu Village", { 10846, 36001, 257 }, 17, 0x00000001u, 0, 17, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "2/3 trials exact, 1 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::US, 169, 7, "Route 14", { 12978, 39966, -16 }, 21, 0x00160001u, 0, 21, 0x00060001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 172, 0, "Route 17", { 9117, 14750, 354 }, 9, 0x000001c1u, 0, 9, 0x000000c1u, 3, 63, 3, Spot7::Rain, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact), rain" },
    Spot7 { Game::US, 172, 2, "Route 17", { 11415, 13435, 589 }, 9, 0x00000101u, 0, 9, 0x00000101u, 3, 63, 3, Spot7::Rain, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact), rain" },
    Spot7 { Game::US, 173, 10, "Route 12", { 30227, 22785, 354 }, 13, 0x00000001u, 0, 13, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 177, 0, "Haina Desert", { 2121, 2193, 0 }, 6, 0x0000003bu, 0, 6, 0x0000002bu, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 185, 0, "Ula'ula Meadow", { 907, 3648, 260 }, 8, 0x0000004bu, 0, 8, 0x0000004bu, 3, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 186, 0, "Malie Garden", { 5526, 1087, 0 }, 32, 0x00000001u, 0, 32, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 187, 0, "Mount Hokulani", { 6310, 5570, -333 }, 28, 0x08000001u, 0, 28, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 188, 0, "Blush Mountain", { 2015, 4659, 6 }, 6, 0x00000031u, 0, 6, 0x00000011u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 188, 5, "Blush Mountain", { 3528, 4500, 22 }, 6, 0x00000031u, 0, 6, 0x00000031u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 222, 0, "Thrifty Megamart", { 1654, 2420, 5 }, 2, 0x00000001u, 0, 2, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 264, 0, "Poni Wilds", { 16677, 12068, -1310 }, 13, 0x00001401u, 0, 13, 0x00001401u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 265, 8, "Ancient Poni Path", { 24178, 18817, -1196 }, 19, 0x00000001u, 0, 19, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 267, 0, "Poni Grove", { 26913, 15897, -1460 }, 9, 0x00000101u, 0, 9, 0x00000101u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "2/3 trials exact, 1 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::US, 268, 0, "Poni Plains", { 33693, 15964, -1485 }, 14, 0x00001541u, 0, 14, 0x00000541u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 268, 8, "Poni Plains", { 31108, 15365, -1196 }, 14, 0x00003d81u, 0, 14, 0x00003581u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 270, 0, "Poni Gauntlet", { 31914, 6081, -1357 }, 11, 0x00000199u, 0, 11, 0x00000199u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 274, 0, "Vast Poni Canyon", { 6887, 2153, 1 }, 20, 0x00080001u, 0, 20, 0x00080001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 278, 0, "Vast Poni Canyon", { 4889, 2053, -37 }, 9, 0x00000005u, 0, 9, 0x00000005u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::US, 278, 6, "Vast Poni Canyon", { 4570, 4388, 1504 }, 9, 0x0000000fu, 0, 9, 0x0000000fu, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 0, 2, "Route 1", { 29632, 19399, -1192 }, 18, 0x00030001u, 0, 18, 0x00030001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 6, 1, "Route 3", { 21840, 11488, -1538 }, 15, 0x00000001u, 0, 2, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 6, 14, "Route 3", { 27967, 9128, -1415 }, 15, 0x00000001u, 0, 2, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 8, 0, "Kala'e Bay", { 17563, 3527, 232 }, 2, 0x00000001u, 0, 19, 0x00001001u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 8, 13, "Kala'e Bay", { 21086, 11528, -26 }, 2, 0x00000003u, 0, 19, 0x00004001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 9, 0, "Melemele Sea", { 17700, 22803, -1316 }, 3, 0x00000003u, 0, 17, 0x000079c3u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 26, 0, "Ten Carat Hill", { 6001, 6348, 0 }, 1, 0x00000001u, 0, 1, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "1/3 trials exact, 2 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::UM, 26, 1, "Ten Carat Hill", { 6714, 6865, -48 }, 1, 0x00000001u, 0, 1, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 27, 0, "Ten Carat Hill", { 4050, 7503, 0 }, 2, 0x00000001u, 0, 2, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 28, 0, "Hau'oli Cemetery", { 1270, 2591, 4 }, 11, 0x00000687u, 0, 11, 0x00000687u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 29, 0, "Melemele Meadow", { 2032, 2079, -96 }, 6, 0x0000001bu, 0, 6, 0x0000001bu, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 30, 0, "Seaward Cave", { 4151, 4357, -608 }, 6, 0x00000001u, 0, 6, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "2/3 trials exact, 1 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::UM, 30, 1, "Seaward Cave", { 2038, 3535, -547 }, 6, 0x00000035u, 0, 6, 0x00000035u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 80, 0, "Route 4", { 17895, 26038, 214 }, 8, 0x00000061u, 0, 8, 0x00000061u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 82, 0, "Route 6", { 20323, 23370, 214 }, 12, 0x00000e01u, 0, 12, 0x00000e01u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 82, 1, "Route 6", { 20220, 27124, 214 }, 12, 0x00000181u, 0, 12, 0x00000181u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 84, 6, "Route 8", { 15475, 13383, -16 }, 24, 0x00b00001u, 0, 24, 0x00300001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 84, 7, "Route 8", { 25406, 6512, 193 }, 24, 0x00000021u, 0, 24, 0x00000021u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 97, 0, "Memorial Hill", { 1461, 2518, -128 }, 12, 0x00000a01u, 0, 12, 0x00000a01u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 98, 2, "Akala Outskirts", { 5978, 4170, -188 }, 10, 0x00000301u, 0, 10, 0x00000301u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 99, 1, "Diglett's Tunnel", { 5116, 3743, 4 }, 12, 0x00000009u, 0, 12, 0x00000009u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 100, 0, "Wela Volcano Park", { 7041, 11332, -950 }, 11, 0x00000101u, 0, 11, 0x00000101u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "2/3 trials exact, 1 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::UM, 102, 0, "Brooklet Hill", { 6094, 4382, -144 }, 8, 0x00000041u, 0, 8, 0x00000041u, 3, 63, 4, Spot7::Rain, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact), rain" },
    Spot7 { Game::UM, 102, 1, "Brooklet Hill", { 8062, 2750, -327 }, 8, 0x000000dfu, 0, 8, 0x000000dfu, 3, 63, 4, Spot7::Rain, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact), rain" },
    Spot7 { Game::UM, 103, 13, "Brooklet Hill", { 5081, 8614, -577 }, 6, 0x0000001du, 0, 6, 0x0000001du, 3, 63, 4, Spot7::Rain, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact), rain" },
    Spot7 { Game::UM, 104, 0, "Brooklet Hill", { 3472, 4487, -27 }, 2, 0x00000001u, 0, 2, 0x00000001u, 3, 63, 4, Spot7::Rain, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact), rain" },
    Spot7 { Game::UM, 105, 0, "Lush Jungle", { 2530, 2856, 4 }, 10, 0x00000001u, 0, 10, 0x00000001u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "2/3 trials exact, 1 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::UM, 109, 0, "Lush Jungle", { 2691, 1794, 4 }, 2, 0x00000003u, 0, 2, 0x00000003u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 150, 0, "Paniola Ranch", { 3546, 5585, 4 }, 24, 0x000c0c03u, 0, 24, 0x000c0c03u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 164, 0, "Route 10", { 15354, 19961, 350 }, 19, 0x00014001u, 0, 19, 0x00014001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 165, 0, "Route 11", { 19665, 20079, 174 }, 12, 0x00000015u, 0, 12, 0x00000015u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 168, 0, "Tapu Village", { 10846, 36001, 257 }, 17, 0x00000001u, 0, 17, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 169, 7, "Route 14", { 12978, 39966, -16 }, 21, 0x00160001u, 0, 21, 0x00060001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 172, 0, "Route 17", { 9117, 14750, 354 }, 9, 0x000001c1u, 0, 9, 0x000000c1u, 3, 63, 3, Spot7::Rain, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact), rain" },
    Spot7 { Game::UM, 172, 2, "Route 17", { 11415, 13435, 589 }, 9, 0x00000101u, 0, 9, 0x00000101u, 3, 63, 3, Spot7::Rain, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact), rain" },
    Spot7 { Game::UM, 173, 10, "Route 12", { 30227, 22785, 354 }, 13, 0x00000001u, 0, 13, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 177, 0, "Haina Desert", { 2121, 2193, 0 }, 3, 0x00000001u, 0, 3, 0x00000001u, 1, 63, 4, Spot7::Unsafe, false, 0.0f, 29.913f, false, 0.0f, "1/3 trials exact, 2 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::UM, 185, 0, "Ula'ula Meadow", { 907, 3648, 260 }, 8, 0x00000041u, 0, 8, 0x00000041u, 3, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 186, 0, "Malie Garden", { 5526, 1087, 0 }, 32, 0x00000001u, 0, 32, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "2/3 trials exact, 1 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::UM, 187, 0, "Mount Hokulani", { 6310, 5570, -333 }, 28, 0x08000001u, 0, 28, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 188, 0, "Blush Mountain", { 2015, 4659, 6 }, 6, 0x0000003fu, 0, 6, 0x0000001fu, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 188, 5, "Blush Mountain", { 3528, 4500, 22 }, 6, 0x00000031u, 0, 6, 0x00000031u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 222, 0, "Thrifty Megamart", { 1654, 2420, 5 }, 2, 0x00000003u, 0, 2, 0x00000003u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 264, 0, "Poni Wilds", { 16677, 12068, -1310 }, 13, 0x00001401u, 0, 13, 0x00001401u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 265, 8, "Ancient Poni Path", { 24178, 18817, -1196 }, 19, 0x00043021u, 0, 19, 0x00043021u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 267, 0, "Poni Grove", { 26913, 15897, -1460 }, 9, 0x0000017du, 0, 9, 0x0000017du, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 268, 0, "Poni Plains", { 33693, 15964, -1485 }, 10, 0x00000141u, 0, 10, 0x00000141u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 268, 8, "Poni Plains", { 31108, 15365, -1196 }, 10, 0x000001b3u, 0, 10, 0x000001b3u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 270, 0, "Poni Gauntlet", { 31914, 6081, -1357 }, 11, 0x00000199u, 0, 11, 0x00000199u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::UM, 274, 0, "Vast Poni Canyon", { 6887, 2153, 1 }, 20, 0x00080001u, 0, 20, 0x00080001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "1/2 trials exact, 1 more consistent but ambiguous (only the player blinks, the lead fell in a blink pause" },
    Spot7 { Game::UM, 278, 0, "Vast Poni Canyon", { 4889, 2053, -37 }, 7, 0x00000005u, 0, 7, 0x00000005u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "2/2 trials exact" },
    Spot7 { Game::UM, 278, 6, "Vast Poni Canyon", { 4570, 4388, 1504 }, 7, 0x0000000fu, 0, 7, 0x0000000fu, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "3/3 trials exact" },
    Spot7 { Game::USUM, 3, 11, "Route 1", { 17877, 15091, -179 }, 13, 0x00000051u, 0, 13, 0x00000051u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 3, 13, "Route 1", { 23498, 13835, -25 }, 13, 0x00000801u, 0, 13, 0x00000801u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 3, 14, "Route 1", { 23498, 15765, -138 }, 13, 0x00000809u, 0, 13, 0x00000809u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 3, 15, "Route 1", { 21475, 16763, -249 }, 13, 0x00001003u, 0, 13, 0x00001003u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 5, 0, "Route 1", { 23498, 15765, -138 }, 23, 0x0001c1e3u, 0, 23, 0x0001c1e3u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 5, 1, "Route 1", { 20044, 14908, -170 }, 23, 0x00004001u, 0, 23, 0x00004001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 5, 2, "Route 1", { 17877, 15067, -179 }, 23, 0x00000001u, 0, 23, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 32, 0, "Sandy Cave", { 2311, 2603, 198 }, 2, 0x00000001u, 0, 2, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 62, 0, "Route 1", { 929, 2591, 8 }, 15, 0x00000609u, 0, 15, 0x00000609u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 62, 1, "Route 1", { 515, 2487, -83 }, 15, 0x00000409u, 0, 15, 0x00000409u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 62, 2, "Route 1", { 793, 1750, -52 }, 15, 0x00000001u, 0, 15, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 62, 3, "Route 1", { 716, 1072, -123 }, 15, 0x00000001u, 0, 15, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 80, 3, "Route 4", { 17460, 28301, 242 }, 8, 0x00000025u, 0, 8, 0x00000025u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 97, 0, "Memorial Hill", { 1487, 2571, 16 }, 12, 0x00000a01u, 0, 12, 0x00000a01u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 170, 0, "Route 15", { 10256, 32605, 276 }, 26, 0x00800481u, 0, 26, 0x00800481u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 170, 5, "Route 15", { 6268, 32374, 338 }, 26, 0x02000001u, 0, 26, 0x02000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 170, 8, "Route 15", { 9470, 35136, 42 }, 26, 0x00000001u, 0, 26, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 171, 6, "Route 16", { 3998, 24294, 264 }, 12, 0x00000301u, 0, 12, 0x00000301u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 171, 7, "Route 16", { 8322, 25225, 264 }, 12, 0x00000cf1u, 0, 12, 0x00000cf1u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 175, 0, "Malie City", { 19933, 8958, 370 }, 10, 0x000002e1u, 0, 10, 0x000002e1u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 175, 1, "Malie City", { 18958, 10367, 370 }, 10, 0x000001efu, 0, 10, 0x000001efu, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 175, 2, "Malie City", { 16883, 10203, 370 }, 10, 0x0000001du, 0, 10, 0x0000001du, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 200, 0, "Mount Lanakila", { 2689, 2655, 32 }, 7, 0x0000007bu, 0, 7, 0x0000007bu, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 201, 0, "Mount Lanakila", { 6291, 1829, 24 }, 2, 0x00000003u, 0, 2, 0x00000003u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 202, 0, "Mount Lanakila", { 2003, 3385, 172 }, 1, 0x00000001u, 0, 1, 0x00000001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 204, 0, "Mount Lanakila", { 3028, 5854, -232 }, 5, 0x0000001fu, 0, 5, 0x0000001fu, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 205, 0, "Mount Lanakila", { 2927, 5541, 492 }, 7, 0x0000000fu, 0, 7, 0x0000000fu, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 206, 0, "Mount Lanakila", { 4238, 4236, 362 }, 8, 0x0000002du, 0, 8, 0x0000002du, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 266, 0, "Poni Breaker Coast", { 27907, 20194, 17 }, 6, 0x00000031u, 0, 6, 0x00000031u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 267, 2, "Poni Grove", { 28538, 15763, -1249 }, 9, 0x00000101u, 0, 9, 0x00000101u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 267, 3, "Poni Grove", { 28976, 16224, -1195 }, 9, 0x00000081u, 0, 9, 0x00000081u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 274, 1, "Vast Poni Canyon", { 6873, 2654, 22 }, 20, 0x00080001u, 0, 20, 0x00080001u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 287, 0, "Resolution Cave", { 3777, 3376, 415 }, 4, 0x0000000bu, 0, 4, 0x0000000bu, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 288, 0, "Resolution Cave", { 4250, 3780, -56 }, 2, 0x00000003u, 0, 2, 0x00000003u, 1, 63, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
    Spot7 { Game::USUM, 290, 1, "Exeggutor Island", { 2644, 7771, 222 }, 9, 0x000001b9u, 0, 9, 0x000001b9u, 3, 63, 4, Spot7::Rain, false, 0.0f, 29.913f, false, 0.0f, "USUM-only area, measured in Ultra Sun" },
};

namespace Spots7
{
    std::vector<const Spot7 *> getSpots(Game version)
    {
        std::vector<const Spot7 *> result;
        for (const auto &spot : spots)
        {
            if ((spot.games & version) != Game::None)
            {
                result.emplace_back(&spot);
            }
        }
        return result;
    }

    const Spot7 *getSpot(Game version, u16 zone, u8 table)
    {
        for (const auto &spot : spots)
        {
            if ((spot.games & version) != Game::None && spot.zone == zone && spot.table == table)
            {
                return &spot;
            }
        }
        return nullptr;
    }
}
