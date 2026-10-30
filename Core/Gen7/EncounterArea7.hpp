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

#ifndef ENCOUNTERAREA7_HPP
#define ENCOUNTERAREA7_HPP

#include <Core/Global.hpp>
#include <Core/Parents/Slot.hpp>
#include <array>
#include <vector>

enum class Game : u32;

/**
 * @brief One Gen 7 wild encounter table (one tile type of an area, day or night): 10 slots with their rates
 */
class EncounterArea7
{
public:
    /**
     * @brief Construct a new EncounterArea7 object
     *
     * @param zone Zone id the table was looked up for
     * @param table Encounter table index within the area
     * @param night Whether this is the night table
     * @param pokemon Slots
     * @param rates Slot rates, in percent
     */
    EncounterArea7(u16 zone, u8 table, bool night, const std::array<Slot, 10> &pokemon, const std::array<u8, 10> &rates) :
        pokemon(pokemon), rates(rates), zone(zone), table(table), night(night)
    {
    }

    /**
     * @brief Returns the slot a rand(100) roll picks: the cumulative rates in slot order
     */
    u8 getEncounterSlot(u8 roll) const;

    bool getNight() const
    {
        return night;
    }

    const std::array<Slot, 10> &getPokemon() const
    {
        return pokemon;
    }

    const Slot &getPokemon(size_t index) const
    {
        return pokemon[index];
    }

    const std::array<u8, 10> &getRates() const
    {
        return rates;
    }

    u8 getTable() const
    {
        return table;
    }

    u16 getZone() const
    {
        return zone;
    }

private:
    std::array<Slot, 10> pokemon;
    std::array<u8, 10> rates;
    u16 zone;
    u8 table;
    bool night;
};

namespace Encounters7
{
    /**
     * @brief Returns the encounter table of \p zone / \p table in \p version, or nullopt-like empty vector when the zone has none
     *
     * @param version Game version
     * @param zone Zone id
     * @param table Encounter table index of the tile
     * @param night Night table instead of day
     */
    std::vector<EncounterArea7> getEncounters(Game version, u16 zone, u8 table, bool night);
}

#endif // ENCOUNTERAREA7_HPP
