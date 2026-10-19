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

#include <Core/Gen6/EncounterArea6.hpp>
#include <Core/Gen6/Profile6.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <Core/Parents/Generators/Generator.hpp>
#include <Core/Parents/Slot.hpp>

class State6;
enum class Lead : u8;

/**
 * @brief Wild encounter generator for X/Y (main MT side)
 *
 * Which slot appears, the lead check and held items come from the TinyMT and are not modelled here: the user picks the
 * target slot and the lead result. Advances are the MT frame of the trigger; the EC is MT output advance + delay + 60 (the
 * 60-output container of 6 dummy Pokemon).
 *
 * Hordes generate 5 members back to back. Normal members draw a 2-ability slot (>> 31); the hidden-ability member, chosen on
 * the TinyMT, has its ability fixed and skips that draw, so every later member starts one output earlier.
 */
class WildGenerator6 : public Generator<Profile6, StateFilter>
{
public:
    /**
     * @brief Construct a new WildGenerator6 object
     *
     * @param initialAdvances Initial number of advances
     * @param maxAdvances Maximum number of advances
     * @param delay MT frames from the trigger to the 60-output container
     * @param encounter Encounter type
     * @param lead Lead result (Synchronize nature or Cute Charm), Lead::None for no lead effect
     * @param targets Target slot, or the 5 horde members
     * @param haMember Horde member (1-5) with the hidden ability, 0 for none
     * @param ivCount Guaranteed perfect IVs
     * @param shinyLocked Whether the Pokemon is shiny locked (roaming birds)
     * @param profile Profile Information
     * @param filter State filter
     */
    WildGenerator6(u32 initialAdvances, u32 maxAdvances, u32 delay, Encounter6 encounter, Lead lead, const std::vector<Slot> &targets,
                   u8 haMember, u8 ivCount, bool shinyLocked, const Profile6 &profile, const StateFilter &filter);

    /**
     * @brief Generates states
     *
     * @param seed Initial MT seed
     *
     * @return Vector of computed states
     */
    std::vector<State6> generate(u32 seed) const;

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
    std::vector<Slot> targets;
    Encounter6 encounter;
    Lead lead;
    u8 haMember;
    u8 ivCount;
    bool shinyLocked;
};

#endif // WILDGENERATOR6_HPP
