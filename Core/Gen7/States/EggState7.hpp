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

#ifndef EGGSTATE7_HPP
#define EGGSTATE7_HPP

#include <Core/Parents/States/EggState.hpp>

/**
 * @brief Gen 7 egg: the traits come from the daycare TinyMT stored in the save, the PID from it too (Masuda / Shiny Charm rolls) or
 * from the main RNG when the egg is accepted
 */
class EggState7 : public EggGeneratorState
{
public:
    /**
     * @brief Construct a new EggState7 object
     *
     * @param advances Egg number in the chain, or the main RNG advance of the PID draw
     * @param ec Pokemon EC
     * @param pid Pokemon PID
     * @param ivs Pokemon IVs
     * @param ability Pokemon ability
     * @param gender Pokemon gender
     * @param nature Pokemon nature
     * @param shiny Pokemon shininess
     * @param inheritance Pokemon IV inheritance
     * @param state Daycare TinyMT state after the egg is accepted
     * @param draws TinyMT draws the egg costs (the accept adds 2)
     * @param mainPID Whether the PID comes from the main RNG at the accept instead of the egg TinyMT
     * @param info Pokemon personal information
     */
    EggState7(u32 advances, u32 ec, u32 pid, const std::array<u8, 6> &ivs, u8 ability, u8 gender, u8 nature, u8 shiny,
              const std::array<u8, 6> &inheritance, const std::array<u32, 4> &state, u8 draws, bool mainPID, const PersonalInfo *info) :
        EggGeneratorState(advances, ec, pid, ivs, ability, gender, 1, nature, shiny, inheritance, info),
        state(state),
        draws(draws),
        mainPID(mainPID)
    {
    }

    /**
     * @brief Returns how many TinyMT draws the egg itself costs; accepting it costs 2 more, rejecting it costs 1
     *
     * @return Draw count
     */
    u8 getDraws() const
    {
        return draws;
    }

    /**
     * @brief Returns whether the PID comes from the main RNG
     *
     * @return true PID is one main RNG draw when the egg is accepted
     * @return false PID comes from the egg TinyMT
     */
    bool getMainPID() const
    {
        return mainPID;
    }

    /**
     * @brief Returns the daycare TinyMT state after the egg is accepted (the next egg's starting state)
     *
     * @return TinyMT state, low word first
     */
    const std::array<u32, 4> &getState() const
    {
        return state;
    }

private:
    std::array<u32, 4> state;
    u8 draws;
    bool mainPID;
};

#endif // EGGSTATE7_HPP
