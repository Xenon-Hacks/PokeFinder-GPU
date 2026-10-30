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

#ifndef TIMER7_HPP
#define TIMER7_HPP

#include <Core/Global.hpp>

/**
 * @brief Converts a target field frame into a press time: t = (T - O) / fps seconds after the A press on Continue, where O is the
 * per-spot offset (bag press -> freeze delay minus Continue press -> field frame 0, in frames) and fps the spot's frame rate
 * (29.913 nominal on a New 3DS; busy maps run slower at 268 MHz). Any fixed input-path delay cancels in O. One timed press
 * calibrates O for the session: hardware values drift about 1 frame between sessions.
 */
struct Timer7
{
    float offset; ///< O in frames
    float fps; ///< Frames per second of the spot

    /**
     * @brief Seconds after the Continue press to press A on BAG so the bag freezes on field frame \p frame
     */
    constexpr double seconds(u32 frame) const
    {
        return (static_cast<double>(frame) - offset) / fps;
    }

    /**
     * @brief Field frame a press at \p seconds lands on
     */
    constexpr double frame(double seconds) const
    {
        return seconds * fps + offset;
    }

    /**
     * @brief Returns the offset that would have made a press at \p seconds land on \p hitFrame (one-press calibration)
     */
    constexpr float calibrate(double seconds, u32 hitFrame) const
    {
        return static_cast<float>(hitFrame - seconds * fps);
    }

    /**
     * @brief Returns the frame rate that fits two presses at \p seconds1 / \p seconds2 landing on \p frame1 / \p frame2
     */
    static constexpr float fitFps(double seconds1, u32 frame1, double seconds2, u32 frame2)
    {
        return static_cast<float>((static_cast<double>(frame2) - frame1) / (seconds2 - seconds1));
    }
};

#endif // TIMER7_HPP
