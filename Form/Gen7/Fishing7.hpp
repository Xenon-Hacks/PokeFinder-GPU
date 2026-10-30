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

#ifndef FISHING7_HPP
#define FISHING7_HPP

#include <QWidget>
#include <vector>

class FishingModel7;
class Profile7;
struct FishingSpot7;
struct Timer7;

namespace Ui
{
    class Fishing7;
}

/**
 * @brief Provides the Gen 7 fishing window
 */
class Fishing7 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new Fishing7 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    Fishing7(QWidget *parent = nullptr);

    /**
     * @brief Destroy the Fishing7 object
     */
    ~Fishing7() override;

public slots:
    /**
     * @brief Reloads profiles
     */
    void updateProfiles();

private:
    Ui::Fishing7 *ui;
    const Profile7 *currentProfile;
    FishingModel7 *model;
    std::vector<const FishingSpot7 *> spots;

    const FishingSpot7 *currentSpot() const;
    Timer7 timer() const;

private slots:
    void calibrate();
    void generate();
    void profileChanged(const Profile7 &profile);
    void spotIndexChanged(int index);
    void timerChanged();
};

#endif // FISHING7_HPP
