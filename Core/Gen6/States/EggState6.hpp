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

#ifndef EGGSTATE6_HPP
#define EGGSTATE6_HPP

#include <Core/Parents/States/EggState.hpp>

/**
 * @brief State class for X/Y eggs
 */
class EggState6 : public EggGeneratorState
{
public:
    /**
     * @brief Construct a new EggState6 object
     *
     * @param advances MT frame of the accept/reject that produced \p seed
     * @param ec Pokemon EC
     * @param pid Pokemon PID
     * @param ivs Pokemon IVs
     * @param ability Pokemon ability
     * @param gender Pokemon gender
     * @param nature Pokemon nature
     * @param shiny Pokemon shininess
     * @param inheritance Pokemon IV inheritance
     * @param seed 64bit egg seed (high word = key 1, low word = key 0)
     * @param mainPID Whether the PID comes from the main MT at the accept frame instead of the egg MT
     * @param info Pokemon personal information
     */
    EggState6(u32 advances, u32 ec, u32 pid, const std::array<u8, 6> &ivs, u8 ability, u8 gender, u8 nature, u8 shiny,
              const std::array<u8, 6> &inheritance, u64 seed, bool mainPID, const PersonalInfo *info) :
        EggGeneratorState(advances, ec, pid, ivs, ability, gender, 1, nature, shiny, inheritance, info), seed(seed), mainPID(mainPID)
    {
    }

    /**
     * @brief Returns whether the PID comes from the main MT
     *
     * @return true PID is drawn from the main MT when the egg is accepted
     * @return false PID comes from the egg MT
     */
    bool getMainPID() const
    {
        return mainPID;
    }

    /**
     * @brief Returns the egg seed
     *
     * @return 64bit egg seed
     */
    u64 getSeed() const
    {
        return seed;
    }

private:
    u64 seed;
    bool mainPID;
};

#endif // EGGSTATE6_HPP
