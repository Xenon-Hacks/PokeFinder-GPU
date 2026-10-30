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

#ifndef FISHINGSTATE7_HPP
#define FISHINGSTATE7_HPP

#include <Core/Parents/States/WildState.hpp>

/**
 * @brief Result of one cast: nothing bites, an item is hooked, or a Pokemon is hooked and generated
 */
enum class FishingResult7 : u8
{
    NoBite,
    Item,
    Pokemon
};

/**
 * @brief One cast frame's outcome: the advances of the cast, bite, hook and lead draws and the Pokemon or item they lead to
 */
class FishingState7 : public WildGeneratorState
{
public:
    /**
     * @brief Construct a new FishingState7 object for a cast that hooks a Pokemon
     *
     * @param castAdvance Advance of the cast's rand(60) draw
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
    FishingState7(u32 castAdvance, u32 leadAdvance, u32 ec, u32 pid, const std::array<u8, 6> &ivs, u8 ability, u8 gender, u8 level, u8 nature,
                  u8 shiny, u8 encounterSlot, u16 item, u16 specie, u8 form, const PersonalInfo *info) :
        WildGeneratorState(castAdvance, ec, pid, ivs, ability, gender, level, nature, shiny, encounterSlot, item, specie, form, info),
        leadAdvance(leadAdvance),
        result(FishingResult7::Pokemon)
    {
    }

    /**
     * @brief Construct a new FishingState7 object for a cast that gets no bite or hooks an item
     *
     * @param castAdvance Advance of the cast's rand(60) draw
     * @param result NoBite or Item
     * @param hookedItem Item id of the hooked item (0 when nothing bites)
     * @param info Any personal data (the state computes nothing meaningful from it)
     */
    FishingState7(u32 castAdvance, FishingResult7 result, u16 hookedItem, const PersonalInfo *info) :
        WildGeneratorState(castAdvance, 0, 0, { 0, 0, 0, 0, 0, 0 }, 0, 2, 1, 0, 0, 0, hookedItem, 0, 0, info),
        leadAdvance(0),
        result(result)
    {
    }

    u32 getBiteAdvance() const
    {
        return biteAdvance;
    }

    u32 getBiteFrame() const
    {
        return biteFrame;
    }

    u8 getBiteRoll() const
    {
        return biteRoll;
    }

    /**
     * @brief Returns the first cast frame that gives this result
     */
    u32 getFrame() const
    {
        return frame;
    }

    u32 getHookAdvance() const
    {
        return hookAdvance;
    }

    u32 getHookFrame() const
    {
        return hookFrame;
    }

    u8 getHookRoll() const
    {
        return hookRoll;
    }

    /**
     * @brief Returns the last cast frame that gives this result (same cast advance)
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
     * @brief Returns the last frame the reel press may land on: the bite frame + rand(15) + 15
     */
    u32 getReelFrame() const
    {
        return reelFrame;
    }

    FishingResult7 getResult() const
    {
        return result;
    }

    /**
     * @brief Returns the bite wait in frames (rand(60) + 30)
     */
    u8 getWait() const
    {
        return wait;
    }

    void setBite(u32 advance, u32 f, u8 roll)
    {
        biteAdvance = advance;
        biteFrame = f;
        biteRoll = roll;
    }

    void setFrames(u32 first, u32 last)
    {
        frame = first;
        lastFrame = last;
    }

    void setHook(u32 advance, u32 f, u8 roll)
    {
        hookAdvance = advance;
        hookFrame = f;
        hookRoll = roll;
    }

    void setLeadRoll(u8 roll)
    {
        leadRoll = roll;
    }

    void setWait(u8 frames, u32 reel)
    {
        wait = frames;
        reelFrame = reel;
    }

private:
    u32 leadAdvance;
    u32 biteAdvance = 0;
    u32 hookAdvance = 0;
    u32 frame = 0;
    u32 lastFrame = 0;
    u32 biteFrame = 0;
    u32 hookFrame = 0;
    u32 reelFrame = 0;
    FishingResult7 result;
    u8 wait = 0;
    u8 biteRoll = 0;
    u8 hookRoll = 0;
    u8 leadRoll = 0;
};

#endif // FISHINGSTATE7_HPP
