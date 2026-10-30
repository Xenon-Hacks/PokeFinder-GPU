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

#ifndef PROFILEDISPLAY7_HPP
#define PROFILEDISPLAY7_HPP

#include <Core/Global.hpp>
#include <QWidget>

class Profile7;
enum class Game : u32;

namespace Ui
{
    class ProfileDisplay7;
}

/**
 * @brief Provides the profile picker and summary shown at the top of the Gen 7 windows
 */
class ProfileDisplay7 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the selected profile changed
     */
    void profileChanged(const Profile7 &);

    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new ProfileDisplay7 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    ProfileDisplay7(QWidget *parent = nullptr);

    /**
     * @brief Destroy the ProfileDisplay7 object
     */
    ~ProfileDisplay7() override;

    /**
     * @brief Sets the settings prefix and the game filter
     */
    void setup(const QString &prefix, Game filter);

    /**
     * @brief Reloads the profiles
     */
    void updateProfiles();

private:
    Ui::ProfileDisplay7 *ui;
    QString prefix;
    std::vector<Profile7> profiles;
    Game filter;

private slots:
    void profileIndexChanged(int index);
    void profileManager();
};

#endif // PROFILEDISPLAY7_HPP
