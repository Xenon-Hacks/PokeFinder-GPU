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

#ifndef FISHINGGENERATOR7_HPP
#define FISHINGGENERATOR7_HPP

#include <Core/Gen7/EncounterArea7.hpp>
#include <Core/Gen7/FishingSpot7.hpp>
#include <Core/Gen7/Profile7.hpp>
#include <Core/Gen7/States/FishingState7.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <vector>

enum class Lead : u8;

/**
 * @brief The field's blink model at the standing position after Continue: the Honey presets do not cover fishing tiles, so the
 * research's fit for the fishing saves is the default (L 4 with only the player persisting, fidget once at frame 4)
 */
struct FieldModel7
{
    u8 continueModels = 4; ///< L: models updated on field frame 1
    u32 persist = 1; ///< Bit i set when model i keeps blinking (bit 0 = player)
    u8 loadDraws = 0; ///< Load-time draws before field frame 1: one rand(100) per fishing spot in the zone after a zone change (LotRare), none when the save was made in the zone
    u8 fidget = 4; ///< Field frame of the first player fidget
    bool fidgetOnce = true; ///< The fidget fired once in every logged fishing run (teleported positions on the water); a shore tile may repeat it after rand(90) + 407/412 frames
};

/**
 * @brief Gen 7 fishing generator: the whole chain hangs on one input, the cast frame. Cast: rand(60) + 30 wait and rand(15) + 15 reel
 * window; bite rand(100) < 50 at cast + C + wait; hook rand(100) < 80 at bite + 16 (21 in the rare state) when the reel comes early;
 * lead at hook + 12 frames, then slot, level, flute, 6 throwaways and the wild block; an item roll at hook + 46 otherwise
 */
class FishingGenerator7
{
public:
    /**
     * @brief Construct a new FishingGenerator7 object
     *
     * @param continueAdvance Advance when Continue was pressed (title advance + needles read)
     * @param minFrame First cast frame to list
     * @param maxFrame Last cast frame to list
     * @param spot Fishing spot
     * @param model Field blink model at the standing position
     * @param area Encounter table of the spot (its rare table when \ref setRare is set)
     * @param lead Lead ability
     * @param profile Profile
     * @param filter State filter
     */
    FishingGenerator7(u32 continueAdvance, u32 minFrame, u32 maxFrame, const FishingSpot7 &spot, const FieldModel7 &model,
                      const EncounterArea7 &area, Lead lead, const Profile7 &profile, const StateFilter &filter);

    /**
     * @brief Lists the results of every cast frame in range; frames with the same cast advance are merged into one state with
     * \ref FishingState7::getFrame / getLastFrame
     *
     * @param seed Boot seed
     */
    std::vector<FishingState7> generate(u32 seed) const;

    /**
     * @brief Runs one cast at field frame \p castFrame after Continue
     *
     * @param seed Boot seed
     * @param castFrame Field frame of the cast press (the cast is processed after that frame's model draws)
     */
    FishingState7 generateAt(u32 seed, u32 castFrame) const;

    /**
     * @brief Sets whether the player is the boy (fidget animation lengths)
     */
    void setBoy(bool flag)
    {
        boy = flag;
    }

    /**
     * @brief Sets the field frame the X menu opened at (the fidget stops), 0 for never
     */
    void setMenuFrame(u32 frame)
    {
        menuFrame = frame;
    }

    /**
     * @brief Sets whether the spot was loaded in its rare (bubbling) state: hook 21 frames after the bite, the rare table and item list
     */
    void setRare(bool flag)
    {
        rare = flag;
    }

    /**
     * @brief Sets whether casts with no bite or an item are listed too (they never pass the Pokemon filter)
     */
    void setShowAll(bool flag)
    {
        showAll = flag;
    }

private:
    FishingSpot7 spot;
    FieldModel7 model;
    EncounterArea7 area;
    Profile7 profile;
    StateFilter filter;
    u32 continueAdvance;
    u32 minFrame;
    u32 maxFrame;
    u32 menuFrame;
    Lead lead;
    bool boy;
    bool rare;
    bool showAll;
};

#endif // FISHINGGENERATOR7_HPP
