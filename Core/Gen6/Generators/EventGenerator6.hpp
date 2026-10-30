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

#ifndef EVENTGENERATOR6_HPP
#define EVENTGENERATOR6_HPP

#include <Core/Gen6/Profile6.hpp>
#include <Core/Gen6/WC6.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <Core/Parents/Generators/Generator.hpp>

class State6;

/**
 * @brief Wonder Card generator for X/Y
 *
 * X/Y generate the gift once, with no 60-frame skip: EC, PID, perfect IV slots, IVs, [ability], nature, [gender]. A field the card
 * fixes skips its draw. Verified live on X 1.0 and Y 1.5 (Celebi, Garchomp).
 */
class EventGenerator6 : public Generator<Profile6, StateFilter>
{
public:
    /**
     * @brief Construct a new EventGenerator6 object
     *
     * @param initialAdvances Initial number of advances
     * @param maxAdvances Maximum number of advances
     * @param delay MT frames between the advance and the EC
     * @param wc6 Wonder Card
     * @param profile Profile Information
     * @param filter State filter
     */
    EventGenerator6(u32 initialAdvances, u32 maxAdvances, u32 delay, const WC6 &wc6, const Profile6 &profile, const StateFilter &filter,
                    bool generateTwice = false);

    /**
     * @brief Generates states
     *
     * @param seed Initial MT seed
     *
     * @return Vector of computed states
     */
    std::vector<State6> generate(u32 seed) const;

private:
    WC6 wc6;
    bool generateTwice; ///< OR/AS generate the gift twice and keep the second (3DSRNGTool, not run live)
};

#endif // EVENTGENERATOR6_HPP
