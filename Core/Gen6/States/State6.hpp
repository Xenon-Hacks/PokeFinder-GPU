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

#ifndef STATE6_HPP
#define STATE6_HPP

#include <Core/Gen6/Pokemon6.hpp>
#include <Core/Parents/States/State.hpp>
#include <algorithm>
#include <array>

/**
 * @brief State class for Gen6 static, wild and event generator encounters
 */
class State6 : public GeneratorState
{
public:
    /**
     * @brief Construct a new State6 object
     *
     * @param advances Trigger frame (MT index of the button press or step)
     * @param result Generated Pokemon
     * @param level Pokemon level
     * @param member Horde member (1-5), 0 for single encounters
     * @param info Pokemon personal information
     */
    State6(u32 advances, const Gen6::Result6 &result, u8 level, u8 member, const PersonalInfo *info) :
        GeneratorState(advances, result.ec, result.pid, result.ivs, result.ability, result.gender, level, result.nature, result.shiny, info),
        ms(0), seed(0), spotFirst {}, spotLast {}, member(member)
    {
    }

    /**
     * @brief Records that this horde Pokemon also shows as \p member at \p advance
     *
     * @param member Horde member (1-5)
     * @param advance Trigger frame
     */
    void addSpot(u8 member, u32 advance)
    {
        u8 i = member - 1;
        if (spotLast[i] == 0 || advance < spotFirst[i])
        {
            spotFirst[i] = advance;
        }
        spotLast[i] = std::max(spotLast[i], advance + 1);
    }

    /**
     * @brief Whether this horde Pokemon shows as \p member anywhere
     *
     * @param member Horde member (1-5)
     *
     * @return true when the member has a spot
     */
    bool hasSpot(u8 member) const
    {
        return spotLast[member - 1] != 0;
    }

    /**
     * @brief Returns the first trigger frame where this horde Pokemon shows as \p member
     *
     * @param member Horde member (1-5)
     *
     * @return First trigger frame
     */
    u32 getSpotFirst(u8 member) const
    {
        return spotFirst[member - 1];
    }

    /**
     * @brief Returns the last trigger frame where this horde Pokemon shows as \p member
     *
     * @param member Horde member (1-5)
     *
     * @return Last trigger frame
     */
    u32 getSpotLast(u8 member) const
    {
        return spotLast[member - 1] - 1;
    }

    /**
     * @brief Returns the console clock of the seed for seed window results
     *
     * @return Console ms since 2000, 0 when the state was generated from a single seed
     */
    u64 getMs() const
    {
        return ms;
    }

    /**
     * @brief Returns the initial MT seed for seed window results
     *
     * @return Initial MT seed
     */
    u32 getSeed() const
    {
        return seed;
    }

    /**
     * @brief Sets the seed and console clock the state came from
     *
     * @param seed Initial MT seed
     * @param ms Console ms since 2000
     */
    void setSeed(u32 seed, u64 ms)
    {
        this->seed = seed;
        this->ms = ms;
    }

    /**
     * @brief Returns the horde member
     *
     * @return Horde member (1-5), 0 for single encounters
     */
    u8 getMember() const
    {
        return member;
    }

private:
    u64 ms;
    u32 seed;
    std::array<u32, 5> spotFirst;
    std::array<u32, 5> spotLast; ///< One past the last trigger frame, 0 for no spot
    u8 member;
};

#endif // STATE6_HPP
