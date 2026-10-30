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

#ifndef STATIC7_HPP
#define STATIC7_HPP

#include <QWidget>

class Profile7;
class StaticModel7;

namespace Ui
{
    class Static7;
}

/**
 * @brief Provides the Gen 7 static, gift and in-game trade window
 */
class Static7 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new Static7 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    Static7(QWidget *parent = nullptr);

    /**
     * @brief Destroy the Static7 object
     */
    ~Static7() override;

public slots:
    /**
     * @brief Reloads the profiles
     */
    void updateProfiles();

private:
    Ui::Static7 *ui;
    const Profile7 *currentProfile;
    StaticModel7 *model;

private slots:
    void categoryIndexChanged(int index);
    void generate();
    void pokemonIndexChanged(int index);
    void profileChanged(const Profile7 &profile);
};

#endif // STATIC7_HPP
