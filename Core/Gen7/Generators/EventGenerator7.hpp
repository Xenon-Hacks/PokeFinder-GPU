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

#ifndef EVENTGENERATOR7_HPP
#define EVENTGENERATOR7_HPP

#include <Core/Gen7/Profile7.hpp>
#include <Core/Gen7/WC7.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <Core/Parents/Generators/Generator.hpp>

class GeneratorState;

/**
 * @brief Gen 7 Wonder Card generator: the deliveryman builds the Pokemon several times during the dialogue (2 or 3 identical blocks) and
 * hands over the last block, built right after the final Yes
 */
class EventGenerator7 : public Generator<Profile7, StateFilter>
{
public:
    /**
     * @brief Construct a new EventGenerator7 object
     *
     * @param initialAdvances Initial number of advances
     * @param maxAdvances Maximum number of advances
     * @param blocksBefore Discarded blocks built before the kept one (1 for Raikou's card, 2 for Munchlax and shiny Magikarp)
     * @param wc7 Wonder Card
     * @param profile Profile Information
     * @param filter State filter
     */
    EventGenerator7(u32 initialAdvances, u32 maxAdvances, u8 blocksBefore, const WC7 &wc7, const Profile7 &profile, const StateFilter &filter);

    /**
     * @brief Generates states for the \p seed
     *
     * @param seed Starting PRNG state
     *
     * @return Vector of computed states
     */
    std::vector<GeneratorState> generate(u32 seed) const;

private:
    WC7 wc7;
    u8 blocksBefore;
};

#endif // EVENTGENERATOR7_HPP
