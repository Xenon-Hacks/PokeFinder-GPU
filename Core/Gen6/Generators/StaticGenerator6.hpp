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

#ifndef STATICGENERATOR6_HPP
#define STATICGENERATOR6_HPP

#include <Core/Gen6/Pokemon6.hpp>
#include <Core/Gen6/Profile6.hpp>
#include <Core/Gen6/StaticTemplate6.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <Core/Parents/Generators/StaticGenerator.hpp>

class State6;

/**
 * @brief Static encounter generator for X/Y
 *
 * Advances are the MT frame of the button press. The EC is MT output advance + delay + 60 for encounters, which run the
 * 60-output container, and advance + delay for gifts and fossils, which do not.
 */
class StaticGenerator6 : public StaticGenerator<StaticTemplate6, Profile6, StateFilter>
{
public:
    /**
     * @brief Construct a new StaticGenerator6 object
     *
     * @param initialAdvances Initial number of advances
     * @param maxAdvances Maximum number of advances
     * @param delay MT frames from the button press to the 60-frame skip
     * @param lead Encounter lead
     * @param staticTemplate Pokemon template
     * @param profile Profile Information
     * @param filter State filter
     */
    StaticGenerator6(u32 initialAdvances, u32 maxAdvances, u32 delay, Lead lead, const StaticTemplate6 &staticTemplate,
                     const Profile6 &profile, const StateFilter &filter);

    /**
     * @brief Generates states
     *
     * @param seed Initial MT seed
     *
     * @return Vector of computed states
     */
    std::vector<State6> generate(u32 seed) const;

    /**
     * @brief Builds the generation request a template makes
     *
     * @param staticTemplate Pokemon template
     * @param profile Profile information
     * @param lead Encounter lead
     *
     * @return Generation request
     */
    static Gen6::Request6 getRequest(const StaticTemplate6 &staticTemplate, const Profile6 &profile, Lead lead);
};

#endif // STATICGENERATOR6_HPP
