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

#ifndef SEEDFINDER6_HPP
#define SEEDFINDER6_HPP

#include <QWidget>

class Profile6;
struct SeedSearch6Settings;
class SeedModel6;

namespace Ui
{
    class SeedFinder6;
}

/**
 * @brief Finds the X/Y initial MT seed (X/Y and OR/AS) from a caught Pokemon's IVs, on the GPU when available
 */
class SeedFinder6 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new SeedFinder6 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    SeedFinder6(QWidget *parent = nullptr);

    /**
     * @brief Destroy the SeedFinder6 object
     */
    ~SeedFinder6() override;

public slots:
    /**
     * @brief Reloads profiles
     */
    void updateProfiles();

private:
    Ui::SeedFinder6 *ui;

    const Profile6 *currentProfile;
    SeedModel6 *model;
    unsigned long long pressToSeed[2] = { 0, 0 }; ///< Calibrated press-to-seed ms for X/Y and OR/AS, 0 for the game's default
    unsigned int pressToSeedVersion = 0; ///< Game of the profile the press-to-seed field belongs to, 0 before the first profile
    unsigned long long planClockSet = 0;
    unsigned long long planTimer = 0;
    unsigned int recovered = 0;
    bool hasRecovered = false;

    /**
     * @brief Reads the caught Pokemon panel
     *
     * @param settings Settings to fill (IVs, nature, skip, perfect IVs)
     *
     * @return true Panel is valid
     * @return false Panel is not valid
     */
    bool readCatch(SeedSearch6Settings &settings);

private slots:
    /**
     * @brief Fills delay, PID rolls and IV count for the encounter preset
     *
     * @param index Encounter index
     */
    void encounterIndexChanged(int index);

    /**
     * @brief Enables the clock fields for clock searches
     *
     * @param index Mode index
     */
    void modeIndexChanged(int index);

    /**
     * @brief Keeps the results that also produce the catch in the panel
     */
    void narrow();

    /**
     * @brief Plans the console clock and timer that hit the target seed
     */
    void seedTime();

    /**
     * @brief Measures the delay from the attempt in the selected result row
     */
    void calibrate();

    /**
     * @brief Recovers the save seed from boots found with the full search
     */
    void recover();

    /**
     * @brief Stores the recovered save seed in the current profile
     */
    void saveRecovered();

    /**
     * @brief Updates the current profile
     *
     * @param profile Current profile
     */
    void profileChanged(const Profile6 &profile);

    /**
     * @brief Starts the seed search
     */
    void search();
};

#endif // SEEDFINDER6_HPP
