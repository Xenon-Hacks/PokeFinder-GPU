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

#ifndef HONEYSTATE7_HPP
#define HONEYSTATE7_HPP

#include <Core/Parents/States/WildState.hpp>

/**
 * @brief A Gen 7 Honey result: the wild Pokemon one bag frame gives. Advances is the bag advance (the main RNG when the bag froze).
 */
class HoneyState7 : public WildGeneratorState
{
public:
    /**
     * @brief Construct a new HoneyState7 object
     *
     * @param bagAdvance Advance when the bag opened
     * @param leadAdvance Advance of the lead-ability roll
     * @param ec Encryption constant
     * @param pid PID
     * @param ivs IVs
     * @param ability Ability slot
     * @param gender Gender
     * @param level Level
     * @param nature Nature
     * @param shiny Shiny type
     * @param encounterSlot Encounter slot
     * @param item Held item
     * @param specie Species
     * @param form Form
     * @param info Personal data
     */
    HoneyState7(u32 bagAdvance, u32 leadAdvance, u32 ec, u32 pid, const std::array<u8, 6> &ivs, u8 ability, u8 gender, u8 level, u8 nature,
                u8 shiny, u8 encounterSlot, u16 item, u16 specie, u8 form, const PersonalInfo *info) :
        WildGeneratorState(bagAdvance, ec, pid, ivs, ability, gender, level, nature, shiny, encounterSlot, item, specie, form, info),
        leadAdvance(leadAdvance),
        frame(0),
        lastFrame(0),
        leadRoll(0),
        special(false)
    {
    }

    /**
     * @brief Returns the first bag frame that gives this result
     */
    u32 getFrame() const
    {
        return frame;
    }

    /**
     * @brief Returns the last bag frame that gives this result (same bag advance)
     */
    u32 getLastFrame() const
    {
        return lastFrame;
    }

    u32 getLeadAdvance() const
    {
        return leadAdvance;
    }

    u8 getLeadRoll() const
    {
        return leadRoll;
    }

    /**
     * @brief Whether the special-table roll (Island Scan / Ultra Beast) passed and this is that target
     */
    bool getSpecial() const
    {
        return special;
    }

    void setFrames(u32 first, u32 last)
    {
        frame = first;
        lastFrame = last;
    }

    void setLeadRoll(u8 roll)
    {
        leadRoll = roll;
    }

    void setSpecial(bool flag)
    {
        special = flag;
    }

private:
    u32 leadAdvance;
    u32 frame;
    u32 lastFrame;
    u8 leadRoll;
    bool special;
};

#endif // HONEYSTATE7_HPP
