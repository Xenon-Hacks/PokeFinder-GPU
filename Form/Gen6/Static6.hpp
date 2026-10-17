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

#ifndef STATIC6_HPP
#define STATIC6_HPP

#include <QWidget>

class Profile6;
class StateModel6;

namespace Ui
{
    class Static6;
}

/**
 * @brief Static encounter generator for X/Y
 */
class Static6 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new Static6 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    Static6(QWidget *parent = nullptr);

    /**
     * @brief Destroy the Static6 object
     */
    ~Static6() override;

public slots:
    /**
     * @brief Reloads profiles
     */
    void updateProfiles();

private:
    Ui::Static6 *ui;

    const Profile6 *currentProfile;
    StateModel6 *model;

private slots:
    /**
     * @brief Updates the pokemon listed
     *
     * @param index Category index
     */
    void categoryIndexChanged(int index);

    /**
     * @brief Generates static encounters from a seed
     */
    void generate();

    /**
     * @brief Updates the template information
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

#endif // STATIC6_HPP
