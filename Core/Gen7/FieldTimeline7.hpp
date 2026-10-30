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

#ifndef FIELDTIMELINE7_HPP
#define FIELDTIMELINE7_HPP

#include <Core/Global.hpp>
#include <Core/RNG/SFMT.hpp>
#include <array>
#include <vector>

struct Spot7;

/**
 * @brief The Gen 7 field timeline (handoff section 3): the main RNG advances only through things that draw. After a field rebuild
 * (Continue, bag close, Honey's Use) frame 1 updates every loaded model from a fresh blink state, later frames update only the
 * persistent models, continuing from the state frame 1 left them in (the "Continue rule"). Plus the player's fidget, rain splashes
 * and one-off load draws. Live draw for draw on Azahar (all four games) and on a New 3DS (Moon 1.2); the RNG never skips or adds
 * a draw under lag, so the tool simulates frames and converts to seconds only for the timer.
 */
class FieldTimeline7
{
public:
    /**
     * @brief Blink engine of one character model (CharaModel eye update)
     */
    struct Model
    {
        u8 state = 0; ///< 0 idle (counting the wait), 1-5 blinking
        u8 wait = 0; ///< Frames before the next rand(128) while idle
        bool doubleBlink = false; ///< Set by the rand(3) == 0 branch: the next blink follows without a draw
    };

    /**
     * @brief Construct a new FieldTimeline7 object
     *
     * @param seed Boot seed
     * @param advance Advance the field starts from (Continue: title advance + needles; Honey: the bag advance)
     */
    FieldTimeline7(u32 seed, u32 advance);

    /**
     * @brief Returns the current advance
     */
    u32 getAdvance() const
    {
        return advance;
    }

    /**
     * @brief One main draw (64-bit)
     */
    u64 next();

    /**
     * @brief Runs the blink engine of \p model for one frame, drawing when the engine does
     */
    void blink(Model &model);

    /**
     * @brief Rebuild frame: all \p count models update once from a fresh state; the returned list holds their states
     */
    std::vector<Model> rebuild(u8 count);

    /**
     * @brief One field frame of persistent-model updates for the models whose bit is set in \p persist
     *
     * @param models Model states from \ref rebuild
     * @param persist Bit i set when model i keeps blinking
     * @param rain Whether the rain splash pair draws this frame
     */
    void frame(std::vector<Model> &models, u32 persist, bool rain);

    /**
     * @brief Continue timeline: the advance at the end of every field frame 1..frames after the Continue press
     *
     * @param spot Spot preset (L, persistent list, load draws, fidget, rain)
     * @param frames Frames to simulate
     * @param menuFrame Field frame the X menu opened (no fidget fires after it), 0 for never
     * @param boy Whether the player character is the boy (fidget animation lengths 407/412 vs 402/407)
     * @param rain Whether it rains at this tile and hour
     *
     * @return advances[f] = advance after field frame f (advances[0] = after the load draws)
     */
    static std::vector<u32> continueAdvances(u32 seed, u32 advance, const Spot7 &spot, u32 frames, u32 menuFrame, bool boy, bool rain);

private:
    SFMT sfmt;
    u32 advance;
};

#endif // FIELDTIMELINE7_HPP
