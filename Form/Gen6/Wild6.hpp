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

#include <QWidget>

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

private slots:
    /**
     * @brief Sets the default delay and IV count for the encounter type
     *
     * @param index Encounter index
     */
    void encounterIndexChanged(int index);

    /**
     * @brief Generates wild encounters from a seed
     */
    void generate();

    /**
     * @brief Updates the current profile
     *
     * @param profile Current profile
     */
    void profileChanged(const Profile6 &profile);
};

#endif // WILD6_HPP
