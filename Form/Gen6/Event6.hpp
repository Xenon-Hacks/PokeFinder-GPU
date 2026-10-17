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

#ifndef EVENT6_HPP
#define EVENT6_HPP

#include <QWidget>

class Profile6;
class StateModel6;
class WC6;

namespace Ui
{
    class Event6;
}

/**
 * @brief Wonder Card generator for X/Y
 */
class Event6 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new Event6 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    Event6(QWidget *parent = nullptr);

    /**
     * @brief Destroy the Event6 object
     */
    ~Event6() override;

public slots:
    /**
     * @brief Reloads profiles
     */
    void updateProfiles();

private:
    Ui::Event6 *ui;

    const Profile6 *currentProfile;
    StateModel6 *model;

    /**
     * @brief Builds the card from the UI fields
     *
     * @param valid Set to false when the fixed IVs are malformed
     *
     * @return Wonder Card
     */
    WC6 getWC6(bool &valid) const;

private slots:
    /**
     * @brief Generates gifts from a seed
     */
    void generate();

    /**
     * @brief Reads a .wc6 file into the UI fields
     */
    void importWC6();

    /**
     * @brief Updates the current profile
     *
     * @param profile Current profile
     */
    void profileChanged(const Profile6 &profile);
};

#endif // EVENT6_HPP
