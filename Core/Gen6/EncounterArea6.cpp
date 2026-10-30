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

#include "EncounterArea6.hpp"
#include <Core/Enum/Game.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/Resources/EncounterData6.hpp>
#include <Core/Util/Translator.hpp>
#include <algorithm>

bool hasEncounter(Encounter6 encounter, Game version)
{
    bool oras = (version & Game::ORAS) != Game::None;
    switch (encounter)
    {
    case Encounter6::YellowFlowers:
    case Encounter6::PurpleFlowers:
    case Encounter6::RedFlowers:
    case Encounter6::RoughTerrain:
    case Encounter6::Ambush:
    case Encounter6::FriendSafari:
        return !oras;
    case Encounter6::TallGrass:
        return oras;
    default:
        return true;
    }
}

u32 EncounterArea6::getDelay() const
{
    bool oras = (version & Game::ORAS) != Game::None;
    switch (encounter)
    {
    case Encounter6::RockSmash:
        return 274; // Yes press -> TinyMT decision and container, live X (OR 272..275)
    case Encounter6::HordeHoney:
        // Honey press -> container: first EC at press + 256..260 on live X, 254..257 (mean 255.3, n = 25) on live Y Route 7;
        // OR Route 103 trigger at press + 177 (3 runs) and AS + 177; 3DSRNGTool uses 174
        return oras ? 177 : 195;
    case Encounter6::HordeSweetScent:
        // Sweet Scent press -> container: X Route 7 + 339 (first EC at press + 399), OR Route 103 + 321, AS + 317
        if (oras)
        {
            return (version & Game::AS) != Game::None ? 317 : 321;
        }
        return 339;
    case Encounter6::Ambush:
        // Cave shadows and drops: Reflection Cave, Victory Road, Glittering Cave, Terminus Cave (shadow walked onto: press + 32).
        // Bushes on Routes 6 and 18: press + 62 (Route 6 bush, live X). Berry trees on Route 7 were not timed.
        switch (location)
        {
        case 56:
        case 104:
        case 132:
        case 140:
            return 32;
        case 38:
            return 0;
        default:
            return 62;
        }
    default:
        return 0;
    }
}

std::vector<std::string> EncounterArea6::getNames() const
{
    std::vector<std::string> names;
    if (isHorde(encounter))
    {
        for (u8 horde = 0; horde < getHordeCount(); horde++)
        {
            auto members = getHorde(horde);
            std::string name = Translator::getSpecie(members[0].getSpecie(), members[0].getForm());
            std::vector<u16> others;
            for (const auto &member : members)
            {
                if (member.getSpecie() != members[0].getSpecie())
                {
                    others.emplace_back(member.getSpecie());
                }
            }
            name += " ×" + std::to_string(5 - others.size());
            for (u16 other : others)
            {
                name += ", " + Translator::getSpecie(other);
            }
            names.emplace_back(name);
        }
    }
    else
    {
        for (const auto &slot : pokemon)
        {
            names.emplace_back(Translator::getSpecie(slot.getSpecie(), slot.getForm()));
        }
    }
    return names;
}

namespace Encounters6
{
    std::vector<EncounterArea6> getEncounters(Encounter6 encounter, Game version)
    {
        const u8 *data;
        if ((version & Game::AS) != Game::None)
        {
            data = ENCOUNTERS_AS.data();
        }
        else if ((version & Game::OR) != Game::None)
        {
            data = ENCOUNTERS_OR.data();
        }
        else
        {
            data = (version & Game::Y) != Game::None ? ENCOUNTERS_Y.data() : ENCOUNTERS_X.data();
        }
        auto u16le = [data](size_t offset) { return static_cast<u16>(data[offset] | (data[offset + 1] << 8)); };

        // Both horde triggers share the horde table
        u8 type = toInt(encounter == Encounter6::HordeSweetScent ? Encounter6::HordeHoney : encounter);

        std::vector<EncounterArea6> areas;
        size_t offset = 2;
        u16 count = u16le(0);
        for (u16 i = 0; i < count; i++)
        {
            u16 location = u16le(offset);
            u8 areaType = data[offset + 2];
            u8 slotCount = data[offset + 3];
            offset += 4;

            if (areaType == type)
            {
                std::vector<Slot> slots;
                slots.reserve(slotCount);
                for (u8 j = 0; j < slotCount; j++)
                {
                    u16 value = u16le(offset + j * 4);
                    u16 specie = value & 0x3ff;
                    u8 form = value >> 11;

                    // Form 30 marks a random form (Vivillon, Spewpa): use the base personal data
                    const PersonalInfo *base = PersonalLoader::getPersonal(version, specie);
                    if (form >= base->getFormCount())
                    {
                        form = 0;
                    }
                    slots.emplace_back(specie, form, data[offset + j * 4 + 2], data[offset + j * 4 + 3],
                                       PersonalLoader::getPersonal(version, specie, form));
                }
                areas.emplace_back(location, encounter, slots, version);
            }
            offset += static_cast<size_t>(slotCount) * 4;
        }

        std::ranges::sort(areas, [](const EncounterArea6 &left, const EncounterArea6 &right) {
            return left.getLocation() < right.getLocation();
        });
        return areas;
    }
}
