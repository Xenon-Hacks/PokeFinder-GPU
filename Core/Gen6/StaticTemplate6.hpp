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
#include <array>

/**
 * @brief Contains additional information for Gen6 static encounters, gifts and in-game trades
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
     * @param ability Fixed ability slot (0, 1, 2) or 255 for a random slot (drawn)
     * @param gender Fixed gender (0, 1, 2) or 255 for the species ratio
     * @param ivCount Guaranteed perfect IVs
     * @param level Pokemon level
     * @param delay MT frames from the button press to the 60-output container (encounters) or to the EC (gifts and trades)
     * @param gift Whether the Pokemon is a gift or trade: one PID roll even with the Shiny Charm, and no 60-output container
     * @param nature Fixed nature or 255 for a random one
     * @param ivs Fixed IVs (HP, Atk, Def, SpA, SpD, Spe), 255 for random
     * @param measured Whether the delay was measured in the game (false: 3DSRNGTool value or an estimate, calibrate it)
     * @param tradeTID Trainer ID of the in-game trade's OT (secret ID 0), or -1 when the Pokemon is not a trade
     */
    constexpr StaticTemplate6(Game version, u16 specie, u8 form, Shiny shiny, u8 ability, u8 gender, u8 ivCount, u8 level, u16 delay,
                              bool gift, u8 nature = 255, const std::array<u8, 6> &ivs = { 255, 255, 255, 255, 255, 255 },
                              bool measured = true, s32 tradeTID = -1) :
        StaticTemplate(version, specie, form, shiny, ability, gender, ivCount, level),
        ivs(ivs),
        tradeTID(tradeTID),
        delay(delay),
        nature(nature),
        gift(gift),
        measured(measured)
    {
    }

    /**
     * @brief Returns the delay from the button press to the container (encounters) or the EC (gifts and trades)
     *
     * @return Delay in MT frames
     */
    u16 getDelay() const
    {
        return delay;
    }

    /**
     * @brief Returns whether the Pokemon is a gift or a trade
     *
     * @return true Gift or trade (one PID roll, no 60-output container)
     * @return false Encounter (Shiny Charm adds two rolls, 60-output container before the EC)
     */
    bool getGift() const
    {
        return gift;
    }

    /**
     * @brief Returns the fixed IVs
     *
     * @return IVs, 255 for random
     */
    std::array<u8, 6> getIVs() const
    {
        return ivs;
    }

    /**
     * @brief Returns whether the delay was measured in the game
     *
     * @return true Measured (Azahar)
     * @return false Taken from 3DSRNGTool or estimated
     */
    bool getMeasured() const
    {
        return measured;
    }

    /**
     * @brief Returns the fixed nature
     *
     * @return Nature, 255 for random
     */
    u8 getNature() const
    {
        return nature;
    }

    /**
     * @brief Returns whether the Pokemon is an in-game trade
     *
     * @return true Trade: shiny-locked against the trade OT and generated with its ID
     * @return false Not a trade
     */
    bool getTrade() const
    {
        return tradeTID >= 0;
    }

    /**
     * @brief Returns the trainer ID of the trade's OT (its secret ID is 0)
     *
     * @return Trainer ID
     */
    u16 getTradeTID() const
    {
        return static_cast<u16>(tradeTID);
    }

private:
    std::array<u8, 6> ivs;
    s32 tradeTID;
    u16 delay;
    u8 nature;
    bool gift;
    bool measured;
};

#endif // STATICTEMPLATE6_HPP
