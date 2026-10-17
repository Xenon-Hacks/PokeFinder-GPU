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

#ifndef EGGS6_HPP
#define EGGS6_HPP

#include <QWidget>

class EggModel6;
class Profile6;

namespace Ui
{
    class Eggs6;
}

/**
 * @brief Egg generator for X/Y
 */
class Eggs6 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new Eggs6 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    Eggs6(QWidget *parent = nullptr);

    /**
     * @brief Destroy the Eggs6 object
     */
    ~Eggs6() override;

public slots:
    /**
     * @brief Reloads profiles
     */
    void updateProfiles();

private:
    Ui::Eggs6 *ui;

    const Profile6 *currentProfile;
    EggModel6 *model;

    /**
     * @brief Validates the parents and the filter
     *
     * @return true Settings are valid
     * @return false Settings are not valid
     */
    bool validate();

private slots:
    /**
     * @brief Shows the egg the current egg seed produces
     */
    void current();

    /**
     * @brief Generates the next egg for every accept frame of the main MT
     */
    void generate();

    /**
     * @brief Reads the egg seed from a decrypted save
     */
    void loadSave();

    /**
     * @brief Updates the current profile
     *
     * @param profile Current profile
     */
    void profileChanged(const Profile6 &profile);
};

#endif // EGGS6_HPP
