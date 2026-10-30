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

#ifndef PROFILEMANAGER7_HPP
#define PROFILEMANAGER7_HPP

#include <QWidget>

class ProfileModel7;

namespace Ui
{
    class ProfileManager7;
}

/**
 * @brief Provides the window to create, edit and remove Gen 7 profiles
 */
class ProfileManager7 final : public QWidget
{
    Q_OBJECT
signals:
    /**
     * @brief Emits that the profiles have been modified
     */
    void profilesChanged(int);

public:
    /**
     * @brief Construct a new ProfileManager7 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    ProfileManager7(QWidget *parent = nullptr);

    /**
     * @brief Destroy the ProfileManager7 object
     */
    ~ProfileManager7() override;

private:
    Ui::ProfileManager7 *ui;
    ProfileModel7 *model;

private slots:
    void create();
    void duplicate();
    void edit();
    void remove();
};

#endif // PROFILEMANAGER7_HPP
