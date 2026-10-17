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

#ifndef STATICTEMPLATE6_HPP
#define STATICTEMPLATE6_HPP

#include <Core/Parents/StaticTemplate.hpp>

/**
 * @brief Contains additional information for Gen6 static encounters
 */
class StaticTemplate6 : public StaticTemplate
{
public:
    /**
     * @brief Construct a new StaticTemplate6 object
     *
     * @param version Game the template appears in
     * @param specie Pokemon specie
     * @param form Pokemon form
     * @param shiny Never for shiny-locked Pokemon, Random otherwise
     * @param ability Fixed ability slot (0, 1, 2) or 255 for a random slot
     * @param gender Fixed gender (0, 1, 2) or 255 for the species ratio
     * @param ivCount Guaranteed perfect IVs
     * @param level Pokemon level
     * @param delay MT frames from the button press (or step) to the 60-frame skip before the EC
     * @param gift Whether the Pokemon is a gift (one PID roll even with the Shiny Charm)
     */
    constexpr StaticTemplate6(Game version, u16 specie, u8 form, Shiny shiny, u8 ability, u8 gender, u8 ivCount, u8 level, u16 delay,
                              bool gift) :
        StaticTemplate(version, specie, form, shiny, ability, gender, ivCount, level), delay(delay), gift(gift)
    {
    }

    /**
     * @brief Returns the delay from the button press to the 60-frame skip
     *
     * @return Delay in MT frames
     */
    u16 getDelay() const
    {
        return delay;
    }

    /**
     * @brief Returns whether the Pokemon is a gift
     *
     * @return true Gift (one PID roll)
     * @return false Encounter (Shiny Charm adds two rolls)
     */
    bool getGift() const
    {
        return gift;
    }

private:
    u16 delay;
    bool gift;
};

#endif // STATICTEMPLATE6_HPP
