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

#ifndef SAVE7_HPP
#define SAVE7_HPP

#include <Core/Enum/Game.hpp>
#include <Core/Global.hpp>
#include <array>
#include <cstddef>

/**
 * @brief Fields read from a decrypted Gen 7 `main` save: the trainer, the daycare TinyMT and the Poké Pelago TinyMT
 */
struct Save7
{
    static constexpr size_t size = 0x6be00; ///< Sun / Moon
    static constexpr size_t sizeUSUM = 0x6cc00; ///< Ultra Sun / Ultra Moon

    std::array<u32, 4> eggState = { 0, 0, 0, 0 }; ///< Daycare TinyMT state (Sodateya + 0x1DC), low word first as PKHeX shows it
    std::array<u32, 4> pelagoState = { 0, 0, 0, 0 }; ///< Poké Pelago TinyMT state (ResortSave + 0)
    Game version = Game::None;
    u16 sid = 0;
    u16 tid = 0;
    bool eggReady = false; ///< Sodateya + 0x1D8
    bool valid = false;

    /**
     * @brief Reads the fields from \p data
     *
     * @param data Save data
     * @param length Save size
     *
     * @return Parsed save, valid is false when the size is not a Gen 7 save
     */
    static Save7 parse(const u8 *data, size_t length)
    {
        Save7 save;
        if (length != size && length != sizeUSUM)
        {
            return save;
        }

        bool usum = length == sizeUSUM;
        auto u16le = [data](size_t offset) { return static_cast<u16>(data[offset] | (data[offset + 1] << 8)); };
        auto u32le = [data](size_t offset) {
            return static_cast<u32>(data[offset]) | (static_cast<u32>(data[offset + 1]) << 8) | (static_cast<u32>(data[offset + 2]) << 16)
                | (static_cast<u32>(data[offset + 3]) << 24);
        };

        // Every block from Situation on sits 0x200 to 0x600 bytes later on USUM
        size_t myStatus = usum ? 0x1400 : 0x1200;
        size_t sodateya = usum ? 0x6ba00 : 0x6b400;
        size_t resort = usum ? 0x3b800 : 0x3b400;

        save.tid = u16le(myStatus);
        save.sid = u16le(myStatus + 2);
        u8 game = data[myStatus + 4];
        switch (game)
        {
        case 30:
            save.version = Game::Sun;
            break;
        case 31:
            save.version = Game::Moon;
            break;
        case 32:
            save.version = Game::US;
            break;
        case 33:
            save.version = Game::UM;
            break;
        default:
            save.version = usum ? Game::US : Game::Moon;
            break;
        }

        save.eggReady = data[sodateya + 0x1d8] != 0;
        for (size_t i = 0; i < 4; i++)
        {
            save.eggState[i] = u32le(sodateya + 0x1dc + 4 * i);
            save.pelagoState[i] = u32le(resort + 4 * i);
        }

        save.valid = true;
        return save;
    }
};

#endif // SAVE7_HPP
