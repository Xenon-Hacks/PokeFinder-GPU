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

#ifndef HONEY7_HPP
#define HONEY7_HPP

#include <Core/Gen7/Timer7.hpp>
#include <QWidget>
#include <vector>

class HoneyModel7;
class Profile7;
struct Spot7;

namespace Ui
{
    class Honey7;
}

/**
 * @brief Provides the Gen 7 Honey window: from the boot seed and the needles read, every bag frame's wild Pokemon at a spot and
 * the press time that reaches it
 */
class Honey7 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new Honey7 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    Honey7(QWidget *parent = nullptr);

    /**
     * @brief Destroy the Honey7 object
     */
    ~Honey7() override;

public slots:
    /**
     * @brief Reloads the profiles
     */
    void updateProfiles();

private:
    Ui::Honey7 *ui;
    const Profile7 *currentProfile;
    HoneyModel7 *model;
    std::vector<const Spot7 *> spots;

    /**
     * @brief Returns the selected spot, nullptr when none
     */
    const Spot7 *currentSpot() const;

    /**
     * @brief Returns the timer from the offset and fps boxes
     */
    Timer7 timer() const;

private slots:
    void calibrate();
    void generate();
    void profileChanged(const Profile7 &profile);
    void spotIndexChanged(int index);
    void timerSourceChanged();
    void timerChanged();
};

#endif // HONEY7_HPP
