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

#ifndef EGGGENERATOR7_HPP
#define EGGGENERATOR7_HPP

#include <Core/Gen7/Profile7.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <Core/Parents/Generators/EggGenerator.hpp>
#include <array>

class EggState7;

/**
 * @brief Gen 7 egg generator: replays the daycare TinyMT stored in the save (Sodateya + 0x1DC)
 *
 * Parent 0 is the father role (the male, or the Ditto paired with a female), parent 1 the mother role (the female, or the Ditto paired
 * with a male or genderless Pokemon), the order EggSettings::reorderParents leaves them in. Every parent pick rand(2) gives 0 for the
 * father and 1 for the mother; the ability and the species follow the mother role.
 */
class EggGenerator7 : public EggGenerator<Profile7, StateFilter>
{
public:
    /**
     * @brief Construct a new EggGenerator7 object
     *
     * @param daycare Daycare parent information
     * @param sameSpecies Whether both parents are the same species (adds the rand(100) ball roll)
     * @param profile Profile Information
     * @param filter State filter
     */
    EggGenerator7(const Daycare &daycare, bool sameSpecies, const Profile7 &profile, const StateFilter &filter);

    /**
     * @brief Generates the egg the daycare TinyMT \p state produces
     *
     * @param state TinyMT state, low word first
     *
     * @return Generated egg (not filtered); its state is the state after accepting it
     */
    EggState7 generate(const std::array<u32, 4> &state) const;

    /**
     * @brief Generates \p count consecutive eggs, each accepted, after \p rejects rejected eggs
     *
     * @param state TinyMT state, low word first
     * @param rejects Eggs rejected before the first one (one draw each)
     * @param count Number of eggs
     *
     * @return Vector of computed states (filtered)
     */
    std::vector<EggState7> generateChain(std::array<u32, 4> state, u32 rejects, u32 count) const;

    /**
     * @brief Generates the egg of \p state with the PID drawn from the main RNG at each advance; only meaningful without Masuda or the
     * Shiny Charm, when the PID is one main RNG draw at the accept
     *
     * @param state TinyMT state, low word first
     * @param seed Main RNG seed
     * @param initialAdvances Initial number of advances
     * @param maxAdvances Maximum number of advances
     *
     * @return Vector of computed states (filtered)
     */
    std::vector<EggState7> generateMainPID(const std::array<u32, 4> &state, u32 seed, u32 initialAdvances, u32 maxAdvances) const;

    /**
     * @brief Returns how many PID rolls the egg TinyMT makes: 6 with Masuda, +2 with the Shiny Charm; 0 means the PID comes from the
     * main RNG
     */
    u8 getPIDRolls() const
    {
        return pidRolls;
    }

    /**
     * @brief Walks the daycare TinyMT past \p count rejected eggs: EggClear draws once per reject and generates nothing
     *
     * @param state TinyMT state, low word first
     * @param count Rejected eggs
     *
     * @return TinyMT state after the rejects
     */
    static std::array<u32, 4> reject(std::array<u32, 4> state, u32 count);

private:
    u8 pidRolls;
    bool sameSpecies;

    EggState7 generate(const std::array<u32, 4> &state, u32 advances, u32 mainPID) const;
};

#endif // EGGGENERATOR7_HPP
