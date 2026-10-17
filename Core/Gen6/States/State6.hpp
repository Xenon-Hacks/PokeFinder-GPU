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
        member(member)
    {
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
    u8 member;
};

#endif // STATE6_HPP
