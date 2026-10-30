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

#ifndef EVENT7_HPP
#define EVENT7_HPP

#include <QWidget>

class Profile7;
class StaticModel7;
class WC7;

namespace Ui
{
    class Event7;
}

/**
 * @brief Provides the Gen 7 Wonder Card window
 */
class Event7 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new Event7 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    Event7(QWidget *parent = nullptr);

    /**
     * @brief Destroy the Event7 object
     */
    ~Event7() override;

public slots:
    /**
     * @brief Reloads profiles
     */
    void updateProfiles();

private:
    Ui::Event7 *ui;
    const Profile7 *currentProfile;
    StaticModel7 *model;

    WC7 getWC7(bool &valid) const;

private slots:
    void generate();
    void importWC7();
    void profileChanged(const Profile7 &profile);
};

#endif // EVENT7_HPP
