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

#ifndef STATICGENERATOR7_HPP
#define STATICGENERATOR7_HPP

#include <Core/Gen7/Profile7.hpp>
#include <Core/Gen7/StaticTemplate7.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <Core/Parents/Generators/StaticGenerator.hpp>
#include <Core/Parents/States/State.hpp>

/**
 * @brief Gen 7 static, gift and trade generator (handoff 4.8, 4.9, 4.11). Advances are the main-RNG advance of the first draw: the
 * lead roll for statics (the script's final A press), the EC for gifts, the first IV draw for trades.
 */
class StaticGenerator7 : public StaticGenerator<StaticTemplate7, Profile7, StateFilter>
{
public:
    /**
     * @brief Construct a new StaticGenerator7 object
     *
     * @param initialAdvances Initial number of advances
     * @param maxAdvances Maximum number of advances
     * @param blinkDraws Draws made by the on-screen models' blinks between the lead roll and the throwaways: 3 with only the player
     * on screen (3 frames), 12-14 at the Lunala altar
     * @param lead Lead ability (Synchronize fixes the nature on a roll >= 50; gifts take it with no roll)
     * @param staticTemplate Template
     * @param profile Profile
     * @param filter State filter
     */
    StaticGenerator7(u32 initialAdvances, u32 maxAdvances, u8 blinkDraws, Lead lead, const StaticTemplate7 &staticTemplate,
                     const Profile7 &profile, const StateFilter &filter);

    /**
     * @brief Generates states from the boot seed
     */
    std::vector<GeneratorState> generate(u32 seed) const;

private:
    u8 blinkDraws;
};

#endif // STATICGENERATOR7_HPP
