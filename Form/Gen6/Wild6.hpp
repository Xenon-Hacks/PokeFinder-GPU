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

#ifndef WILD6_HPP
#define WILD6_HPP

#include <Core/Gen6/EncounterArea6.hpp>
#include <QWidget>
#include <vector>

class Profile6;
class StateModel6;

namespace Ui
{
    class Wild6;
}

/**
 * @brief Wild encounter generator for X/Y (main MT side)
 */
class Wild6 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new Wild6 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    Wild6(QWidget *parent = nullptr);

    /**
     * @brief Destroy the Wild6 object
     */
    ~Wild6() override;

public slots:
    /**
     * @brief Reloads profiles
     */
    void updateProfiles();

private:
    Ui::Wild6 *ui;

    const Profile6 *currentProfile;
    StateModel6 *model;
    std::vector<EncounterArea6> areas;

    /**
     * @brief Returns the selected area
     */
    const EncounterArea6 *currentArea() const;

private slots:
    /**
     * @brief Lists the locations that have the encounter type
     *
     * @param index Encounter index
     */
    void encounterIndexChanged(int index);

    /**
     * @brief Generates wild encounters from a seed
     */
    void generate();

    /**
     * @brief Lists the targets of the location and sets its default delay
     *
     * @param index Location index
     */
    void locationIndexChanged(int index);

    /**
     * @brief Shows the level of the target
     *
     * @param index Pokemon index
     */
    void pokemonIndexChanged(int index);

    /**
     * @brief Updates the current profile
     *
     * @param profile Current profile
     */
    void profileChanged(const Profile6 &profile);
};

#endif // WILD6_HPP
