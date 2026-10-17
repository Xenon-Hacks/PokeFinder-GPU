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

#ifndef WILDGENERATOR6_HPP
#define WILDGENERATOR6_HPP

#include <Core/Gen6/Profile6.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <Core/Parents/Generators/Generator.hpp>

class PersonalInfo;
class State6;
enum class Lead : u8;

/**
 * @brief X/Y wild encounter types
 */
enum class Encounter6 : u8
{
    Grass, ///< Grass/cave step, also roaming birds (EC = step + 60)
    RockSmash, ///< Advances count from the Yes press (decision at Yes + 274)
    Fishing, ///< Advances count from the bite (the Pokemon is generated even if the bite is missed)
    FriendSafari, ///< 5 PID rolls without the Shiny Charm, 2 perfect IVs
    Horde ///< 5 Pokemon back to back, ability rand(3); advances count from the Honey press (+200)
};

/**
 * @brief Wild encounter generator for X/Y (main MT side)
 *
 * Species, slot, lead checks and held items come from the TinyMT and are not modelled here: the user picks the species and the
 * lead result. Advances are the MT frame of the trigger; the EC is MT output advance + delay + 60.
 */
class WildGenerator6 : public Generator<Profile6, StateFilter>
{
public:
    /**
     * @brief Construct a new WildGenerator6 object
     *
     * @param initialAdvances Initial number of advances
     * @param maxAdvances Maximum number of advances
     * @param delay MT frames from the trigger to the 60-frame skip
     * @param encounter Encounter type
     * @param lead Lead result (Synchronize nature or Cute Charm), Lead::None for no lead effect
     * @param specie Pokemon specie
     * @param level Pokemon level
     * @param ivCount Guaranteed perfect IVs
     * @param shinyLocked Whether the Pokemon is shiny locked (roaming birds)
     * @param profile Profile Information
     * @param filter State filter
     */
    WildGenerator6(u32 initialAdvances, u32 maxAdvances, u32 delay, Encounter6 encounter, Lead lead, u16 specie, u8 level, u8 ivCount,
                   bool shinyLocked, const Profile6 &profile, const StateFilter &filter);

    /**
     * @brief Generates states
     *
     * @param seed Initial MT seed
     *
     * @return Vector of computed states
     */
    std::vector<State6> generate(u32 seed) const;

    /**
     * @brief Returns the default delay for \p encounter
     *
     * @param encounter Encounter type
     *
     * @return Delay in MT frames
     */
    static u32 getDefaultDelay(Encounter6 encounter);

    /**
     * @brief Returns the number of PID rolls for \p encounter
     *
     * @param encounter Encounter type
     * @param shinyCharm Whether the Shiny Charm is obtained
     * @param shinyLocked Whether the Pokemon is shiny locked
     *
     * @return Number of PID rolls
     */
    static u8 getPIDRolls(Encounter6 encounter, bool shinyCharm, bool shinyLocked);

private:
    const PersonalInfo *info;
    Encounter6 encounter;
    Lead lead;
    u8 ivCount;
    u8 level;
    bool shinyLocked;
};

#endif // WILDGENERATOR6_HPP
