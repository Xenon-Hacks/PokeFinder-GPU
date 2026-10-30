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

#ifndef PELAGO7_HPP
#define PELAGO7_HPP

#include <QWidget>

class Profile7;
class StaticModel7;

namespace Ui
{
    class Pelago7;
}

/**
 * @brief Provides the Poké Pelago visitor window
 */
class Pelago7 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new Pelago7 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    Pelago7(QWidget *parent = nullptr);

    /**
     * @brief Destroy the Pelago7 object
     */
    ~Pelago7() override;

public slots:
    /**
     * @brief Reloads profiles
     */
    void updateProfiles();

private:
    Ui::Pelago7 *ui;
    const Profile7 *currentProfile;
    StaticModel7 *model;

private slots:
    void generate();
    void profileChanged(const Profile7 &profile);
};

#endif // PELAGO7_HPP
