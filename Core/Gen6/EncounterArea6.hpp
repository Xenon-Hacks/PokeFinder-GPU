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

#ifndef ENCOUNTERAREA6_HPP
#define ENCOUNTERAREA6_HPP

#include <Core/Global.hpp>
#include <Core/Parents/Slot.hpp>
#include <string>
#include <vector>

enum class Game : u32;

/**
 * @brief Gen 6 wild encounter types. Flowers, rough terrain, ambush spots and the Friend Safari are X/Y only; long grass is OR/AS only.
 */
enum class Encounter6 : u8
{
    Grass, ///< Tall grass and caves (EC = step + 60)
    YellowFlowers,
    PurpleFlowers,
    RedFlowers,
    RoughTerrain,
    Surf,
    RockSmash, ///< Advances count from the Yes press (decision at Yes + 274 on X, + 272..275 on OR)
    OldRod, ///< Advances count from the bite (the Pokemon is generated even if the bite is missed)
    GoodRod,
    SuperRod,
    HordeHoney, ///< 5 Pokemon back to back; advances count from the Honey press (trigger ≈ +195 on X/Y, +177 on OR/AS)
    HordeSweetScent, ///< 5 Pokemon back to back; advances count from the Sweet Scent press (trigger +339 on X, +321 OR, +317 AS)
    Ambush, ///< Bushes, berry trees, cave shadows and drops (bush trigger = press + 62, shadow + 32)
    FriendSafari, ///< 5 PID rolls without the Shiny Charm, 2 perfect IVs
    TallGrass, ///< OR/AS long grass (same generation as grass)
    HordeNatural, ///< Horde triggered by a step (same table as Honey hordes, no pre-advance; advances count from the step)
    DexNav ///< OR/AS DexNav-only slots; the Pokemon is generated at the patch trigger + 60 and its potential IVs overwrite the drawn ones
};

/**
 * @brief Whether \p encounter exists in \p version
 */
bool hasEncounter(Encounter6 encounter, Game version);

/**
 * @brief Converts enum to number
 */
constexpr u8 toInt(Encounter6 encounter)
{
    return static_cast<u8>(encounter);
}

/**
 * @brief Whether \p encounter generates a horde of 5
 */
constexpr bool isHorde(Encounter6 encounter)
{
    return encounter == Encounter6::HordeHoney || encounter == Encounter6::HordeSweetScent || encounter == Encounter6::HordeNatural;
}

/**
 * @brief One Gen 6 location's slots for one encounter type. Species and slot come from the TinyMT, which the Gen 6 tab does not
 * model, so the slots only tell the user what can appear and give the generator the species' personal data.
 */
class EncounterArea6
{
public:
    /**
     * @brief Construct a new EncounterArea6 object
     *
     * @param location Met location
     * @param encounter Encounter type
     * @param pokemon Slots; hordes hold 5 slots per horde
     * @param version Game the area belongs to
     */
    EncounterArea6(u16 location, Encounter6 encounter, const std::vector<Slot> &pokemon, Game version) :
        pokemon(pokemon), version(version), location(location), encounter(encounter)
    {
    }

    /**
     * @brief Returns the default MT frames from the trigger press to the 60-output container for this area
     *
     * @return Delay in MT frames
     */
    u32 getDelay() const;

    Encounter6 getEncounter() const
    {
        return encounter;
    }

    /**
     * @brief Returns the members of horde \p index
     *
     * @param index Horde index
     *
     * @return 5 slots
     */
    std::vector<Slot> getHorde(u8 index) const
    {
        return std::vector<Slot>(pokemon.begin() + index * 5, pokemon.begin() + index * 5 + 5);
    }

    /**
     * @brief Returns the number of hordes in the area
     */
    u8 getHordeCount() const
    {
        return static_cast<u8>(pokemon.size() / 5);
    }

    /**
     * @brief Returns the names of the selectable targets: species for single encounters, "A ×5" / "A ×4, B" for hordes
     */
    std::vector<std::string> getNames() const;

    u16 getLocation() const
    {
        return location;
    }

    const std::vector<Slot> &getPokemon() const
    {
        return pokemon;
    }

    const Slot &getPokemon(size_t index) const
    {
        return pokemon[index];
    }

private:
    std::vector<Slot> pokemon;
    Game version;
    u16 location;
    Encounter6 encounter;
};

namespace Encounters6
{
    /**
     * @brief Gets the areas of \p version that have \p encounter, sorted by location
     *
     * @param encounter Encounter type
     * @param version Game version
     *
     * @return Encounter areas
     */
    std::vector<EncounterArea6> getEncounters(Encounter6 encounter, Game version);
}

#endif // ENCOUNTERAREA6_HPP
