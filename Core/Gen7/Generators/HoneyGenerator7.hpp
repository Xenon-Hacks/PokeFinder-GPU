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

#ifndef HONEYGENERATOR7_HPP
#define HONEYGENERATOR7_HPP

#include <Core/Gen7/EncounterArea7.hpp>
#include <Core/Gen7/Profile7.hpp>
#include <Core/Gen7/Spot7.hpp>
#include <Core/Gen7/States/HoneyState7.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <vector>

enum class Lead : u8;

/**
 * @brief Special-table target at a Honey tile (Island Scan QR target or an Ultra Beast / Necrozma): the scan roll runs at the
 * Honey lead frame; on a pass the target is generated instead of the table's Pokemon
 */
struct SpecialTarget7
{
    u16 specie = 0; ///< Species (0 = none)
    u8 form = 0;
    u8 level = 0;
    u8 rate = 0; ///< Pass rate of the rand(100) roll
    u8 perfectIVs = 3; ///< Legend block: guaranteed 31s (3; Nihilego 4); 0 for the QR wild-style block
    bool legend = true; ///< Legend block (shiny-locked, no ability draw) vs the wild-style block (QR Island Scan targets)
};

/**
 * @brief Gen 7 Honey: from the bag frame T the main RNG is frozen at the bag advance B; Use rebuilds the field (F models, then the
 * persistent ones for D frames: 93 on SM, 63 on US/UM), then lead -> slot -> level -> flute -> 60 throwaways -> the wild Pokemon.
 * Exact on Azahar for all four games and 8/8 on a New 3DS (Moon 1.2), handoff sections 3.9 and 4.
 */
class HoneyGenerator7
{
public:
    /**
     * @brief Construct a new HoneyGenerator7 object
     *
     * @param continueAdvance Advance when Continue was pressed (title advance + needles read)
     * @param minFrame First bag frame to list
     * @param maxFrame Last bag frame to list
     * @param spot Spot preset
     * @param area Encounter table of the tile for the current game hour
     * @param lead Lead ability
     * @param profile Profile
     * @param filter State filter
     */
    HoneyGenerator7(u32 continueAdvance, u32 minFrame, u32 maxFrame, const Spot7 &spot, const EncounterArea7 &area, Lead lead,
                    const Profile7 &profile, const StateFilter &filter);

    /**
     * @brief Lists the results of every bag frame in range; frames with the same bag advance are merged into one state with
     * \ref HoneyState7::getFrame / getLastFrame
     *
     * @param seed Boot seed
     */
    std::vector<HoneyState7> generate(u32 seed) const;

    /**
     * @brief Runs Honey from a known bag advance
     *
     * @param seed Boot seed
     * @param bagAdvance Advance when the bag opened
     */
    HoneyState7 generateAt(u32 seed, u32 bagAdvance) const;

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
     * @brief Sets whether the rebuild uses the older "persistent models start fresh" reading instead of the Continue rule
     * (they differ only when a persistent model rolls 0 on the rebuild frame); kept for the research's logged cases
     */
    void setPersistFresh(bool flag)
    {
        persistFresh = flag;
    }

    /**
     * @brief Sets whether it rains at the tile (rain pair every second frame)
     */
    void setRain(bool flag)
    {
        rain = flag;
    }

    /**
     * @brief Sets the special-table target of the tile
     */
    void setSpecial(const SpecialTarget7 &target)
    {
        special = target;
    }

private:
    Spot7 spot;
    EncounterArea7 area;
    Profile7 profile;
    StateFilter filter;
    SpecialTarget7 special;
    u32 continueAdvance;
    u32 minFrame;
    u32 maxFrame;
    u32 menuFrame;
    Lead lead;
    bool boy;
    bool persistFresh;
    bool rain;
};

#endif // HONEYGENERATOR7_HPP
