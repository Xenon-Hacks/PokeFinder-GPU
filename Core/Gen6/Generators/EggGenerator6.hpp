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

#ifndef EGGGENERATOR6_HPP
#define EGGGENERATOR6_HPP

#include <Core/Gen6/Profile6.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <Core/Parents/Generators/EggGenerator.hpp>

class EggState6;

/**
 * @brief Egg generator for X/Y
 *
 * Every egg is a fresh MT19937 seeded with init_by_array({low, high}) of the 64bit daycare seed (save Daycare + 0x1E8). Accepting or
 * rejecting an egg replaces the seed with the next two main MT outputs, low word first. Without the Shiny Charm and the Masuda method
 * the PID is one main MT output at the accept frame instead.
 */
class EggGenerator6 : public EggGenerator<Profile6, StateFilter>
{
public:
    /**
     * @brief Construct a new EggGenerator6 object
     *
     * @param initialAdvances Initial number of advances
     * @param maxAdvances Maximum number of advances
     * @param daycare Daycare parent information (parent 0 = male or non-Ditto, parent 1 = female or Ditto)
     * @param profile Profile Information
     * @param filter State filter
     */
    EggGenerator6(u32 initialAdvances, u32 maxAdvances, const Daycare &daycare, const Profile6 &profile, const StateFilter &filter);

    /**
     * @brief Generates the egg for an egg seed
     *
     * @param seed 64bit egg seed
     *
     * @return Generated egg (not filtered)
     */
    EggState6 generate(u64 seed) const;

    /**
     * @brief Generates the next egg for every accept/reject frame of the main MT
     *
     * @param seed Initial main MT seed
     *
     * @return Vector of computed states
     */
    std::vector<EggState6> generateChain(u32 seed) const;

private:
    u8 pidRolls;

    /**
     * @brief Generates the egg for an egg seed
     *
     * @param seed 64bit egg seed
     * @param advances Main MT frame the seed was drawn at
     * @param mainPID PID drawn from the main MT, used when there are no egg MT PID rolls
     *
     * @return Generated egg
     */
    EggState6 generate(u64 seed, u32 advances, u32 mainPID) const;
};

#endif // EGGGENERATOR6_HPP
