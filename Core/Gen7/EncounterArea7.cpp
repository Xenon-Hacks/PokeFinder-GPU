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

#include "EncounterArea7.hpp"
#include <Core/Enum/Game.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/Resources/EncounterData7.hpp>

u8 EncounterArea7::getEncounterSlot(u8 roll) const
{
    u8 sum = 0;
    for (u8 i = 0; i < 10; i++)
    {
        sum += rates[i];
        if (roll < sum)
        {
            return i;
        }
    }
    return 9;
}

namespace Encounters7
{
    std::vector<EncounterArea7> getEncounters(Game version, u16 zone, u8 table, bool night)
    {
        const u8 *data;
        if ((version & Game::Sun) != Game::None)
        {
            data = ENCOUNTERS_SUN.data();
        }
        else if ((version & Game::Moon) != Game::None)
        {
            data = ENCOUNTERS_MOON.data();
        }
        else if ((version & Game::US) != Game::None)
        {
            data = ENCOUNTERS_US.data();
        }
        else
        {
            data = ENCOUNTERS_UM.data();
        }
        auto u16le = [data](size_t offset) { return static_cast<u16>(data[offset] | (data[offset + 1] << 8)); };

        std::vector<EncounterArea7> areas;
        size_t offset = 2;
        u16 count = u16le(0);
        for (u16 i = 0; i < count; i++)
        {
            offset += 2; // area index
            u8 zoneCount = data[offset++];
            bool match = false;
            for (u8 j = 0; j < zoneCount; j++)
            {
                if (u16le(offset) == zone)
                {
                    match = true;
                }
                offset += 2;
            }

            u8 tableCount = data[offset++];
            for (u8 j = 0; j < tableCount; j++)
            {
                u8 index = data[offset++];
                for (int time = 0; time < 2; time++)
                {
                    u8 min = data[offset];
                    u8 max = data[offset + 1];
                    if (match && index == table && time == (night ? 1 : 0))
                    {
                        std::array<Slot, 10> slots;
                        std::array<u8, 10> rates;
                        for (int k = 0; k < 10; k++)
                        {
                            u16 value = u16le(offset + 2 + k * 3);
                            u16 specie = value & 0x7ff;
                            u8 form = value >> 11;
                            rates[k] = data[offset + 2 + k * 3 + 2];
                            slots[k] = Slot(specie, form, min, max, specie == 0 ? nullptr : PersonalLoader::getPersonal(version, specie, form));
                        }
                        areas.emplace_back(zone, table, night, slots, rates);
                    }
                    offset += 2 + 10 * 3;
                }
            }
        }
        return areas;
    }
}
