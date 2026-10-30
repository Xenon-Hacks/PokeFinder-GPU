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

#ifndef PROFILE7_HPP
#define PROFILE7_HPP

#include <Core/Parents/Profile.hpp>

enum class Game : u32;

/**
 * @brief Provides additional storage specific to Gen 7 (Sun, Moon, Ultra Sun, Ultra Moon)
 */
class Profile7 : public Profile
{
public:
    /**
     * @brief Construct a new Profile7 object
     *
     * @param name Profile name
     * @param version Game version
     * @param tid Trainer ID
     * @param sid Secret ID
     * @param shinyCharm Whether the Shiny Charm is obtained (3 PID rolls on wild and unlocked targets)
     * @param ovalCharm Whether the Oval Charm is obtained (egg rate)
     */
    Profile7(const std::string &name, Game version, u16 tid, u16 sid, bool shinyCharm, bool ovalCharm) :
        Profile(name, version, tid, sid), shinyCharm(shinyCharm), ovalCharm(ovalCharm)
    {
    }

    /**
     * @brief Returns whether the Oval Charm is obtained
     */
    bool getOvalCharm() const
    {
        return ovalCharm;
    }

    /**
     * @brief Returns whether the Shiny Charm is obtained
     */
    bool getShinyCharm() const
    {
        return shinyCharm;
    }

    /**
     * @brief Checks if two profiles are equal
     */
    bool operator==(const Profile7 &other) const;

    /**
     * @brief Checks if two profiles are not equal
     */
    bool operator!=(const Profile7 &other) const;

private:
    bool shinyCharm;
    bool ovalCharm;
};

#endif // PROFILE7_HPP
