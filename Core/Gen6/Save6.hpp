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

#ifndef SAVE6_HPP
#define SAVE6_HPP

#include <Core/Enum/Game.hpp>
#include <Core/Global.hpp>
#include <cstddef>

/**
 * @brief RNG-relevant fields of a decrypted X/Y save ("main", 0x65600 bytes)
 */
struct Save6
{
    static constexpr size_t size = 0x65600;

    u64 eggSeed = 0; ///< Daycare egg seed (Daycare 0x1B200 + 0x1E8)
    u32 saveSeed = 0; ///< Misc[0x00] at 0x4200, the save term of the MT seed
    Game version = Game::None;
    u16 sid = 0;
    u16 tid = 0;
    bool eggReady = false; ///< Daycare + 0x1E0
    bool shinyCharm = false; ///< Key item 632
    bool valid = false;

    /**
     * @brief Reads the fields from \p data
     *
     * @param data Save data
     * @param length Save size
     *
     * @return Parsed save, valid is false when the size is not an X/Y save
     */
    static Save6 parse(const u8 *data, size_t length)
    {
        Save6 save;
        if (length != size)
        {
            return save;
        }

        auto u16le = [data](size_t offset) { return static_cast<u16>(data[offset] | (data[offset + 1] << 8)); };
        auto u32le = [data](size_t offset) {
            return static_cast<u32>(data[offset]) | (static_cast<u32>(data[offset + 1]) << 8) | (static_cast<u32>(data[offset + 2]) << 16)
                | (static_cast<u32>(data[offset + 3]) << 24);
        };

        constexpr size_t myStatus = 0x14000;
        constexpr size_t daycare = 0x1b200;

        save.tid = u16le(myStatus);
        save.sid = u16le(myStatus + 2);
        save.version = data[myStatus + 4] == 25 ? Game::Y : Game::X;
        save.saveSeed = u32le(0x4200);
        save.eggReady = data[daycare + 0x1e0] != 0;
        save.eggSeed = (static_cast<u64>(u32le(daycare + 0x1ec)) << 32) | u32le(daycare + 0x1e8);
        for (size_t offset = 0x400; offset < 0x400 + 0xb90; offset += 4)
        {
            u16 count = u16le(offset + 2);
            if (u16le(offset) == 632 && count > 0 && count < 1000)
            {
                save.shinyCharm = true;
                break;
            }
        }
        save.valid = true;
        return save;
    }
};

#endif // SAVE6_HPP
