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

#ifndef SPOT7_HPP
#define SPOT7_HPP

#include <Core/Global.hpp>
#include <array>
#include <vector>

enum class Game : u32;

/**
 * @brief Everything the Gen 7 field timeline needs at one Honey tile: which models blink after a Continue and after Honey's Use,
 * the one-off load draws, and the timer offsets. Measured live on Azahar (and on a New 3DS where noted) by the Gen 7 RNG research.
 */
struct Spot7
{
    enum Flag : u8
    {
        Rain = 1, ///< Rain splashes: two draws every second frame from field frame 1 (at every hour at this tile)
        Unsafe = 2, ///< Moving models or walkers: the result shifts between attempts, calibrate per attempt
        FidgetSingle = 4, ///< Hot/cold zone: the fidget draws one rand(90) and repeats after rand(90) + 402 frames
        ContinueUnverified = 8 ///< The Continue timeline of this tile was derived from the rule, not fitted to a log
    };

    Game games; ///< Games the preset applies to
    u16 zone; ///< Zone id of the game
    u8 table; ///< Encounter table of the tile (encounter-data id)
    const char *location; ///< Location name
    std::array<s32, 3> tile; ///< Standing position (x, z, height), save order
    u8 continueModels; ///< L: models updated on the Continue rebuild frame
    u32 continuePersist; ///< Bit i set when model i keeps blinking after the Continue rebuild frame (bit 0 = player)
    u8 continueLoadDraws; ///< Load-time draws before field frame 1 after a Continue (fishing-spot lottery and the like)
    u8 honeyModels; ///< F: models updated on the Honey rebuild frame
    u32 honeyPersist; ///< Bit i set when model i keeps blinking after the Honey rebuild frame
    u8 honeyPreDraws; ///< Draws on Use before the rebuild: 1 on US/UM, +2 for the rain-sound pair, 2 for a walker pair on SM
    u8 honeyDelay; ///< D: frames from the rebuild to the lead roll, 93 on SM and 63 on US/UM
    u8 fidget; ///< Field frame of the first fidget after a rebuild (4 at most spots)
    u8 flags; ///< Flag bits
    bool hardwareTimer; ///< Whether the New 3DS timer offset and fps were measured at this tile
    float hardwareOffset; ///< Hardware timer offset O in frames (t = (T - O) / fps)
    float hardwareFps; ///< Hardware frame rate at this tile (busy maps run below 29.913 at 268 MHz)
    bool azaharTimer; ///< Whether the Azahar timer offset was measured
    float azaharOffset; ///< Azahar timer offset O in frames
    const char *note; ///< How well the preset fit, and anything to watch

    /**
     * @brief Returns whether model \p index keeps blinking after the Continue rebuild
     */
    constexpr bool continuePersists(u8 index) const
    {
        return (continuePersist >> index) & 1;
    }

    /**
     * @brief Returns whether model \p index keeps blinking after the Honey rebuild
     */
    constexpr bool honeyPersists(u8 index) const
    {
        return (honeyPersist >> index) & 1;
    }

    /**
     * @brief Returns the Azahar timer offset, or the rule-of-thumb -61.5 - 0.6 L when none was measured
     */
    constexpr float azaharOffsetOrEstimate() const
    {
        return azaharTimer ? azaharOffset : -61.5f - 0.6f * continueModels;
    }

    /**
     * @brief Returns the hardware timer offset, or the estimate Azahar - 43 when none was measured (sd 3.5 frames)
     */
    constexpr float hardwareOffsetOrEstimate() const
    {
        return hardwareTimer ? hardwareOffset : azaharOffsetOrEstimate() - 43.0f;
    }
};

namespace Spots7
{
    /**
     * @brief Returns the Honey presets of \p version
     */
    std::vector<const Spot7 *> getSpots(Game version);

    /**
     * @brief Returns the preset of \p zone / \p table in \p version, nullptr if none
     */
    const Spot7 *getSpot(Game version, u16 zone, u8 table);
}

#endif // SPOT7_HPP
