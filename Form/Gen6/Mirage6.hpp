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

#ifndef MIRAGE6_FORM_HPP
#define MIRAGE6_FORM_HPP

#include <QWidget>

namespace Ui
{
    class Mirage6;
}

/**
 * @brief OR/AS Mirage spot calendar: the spot of the day and its hidden item for the days after the save was last played
 */
class Mirage6 final : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new Mirage6 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    Mirage6(QWidget *parent = nullptr);

    /**
     * @brief Destroy the Mirage6 object
     */
    ~Mirage6() override;

private:
    Ui::Mirage6 *ui;

private slots:
    /**
     * @brief Lists the Mirage spots
     */
    void generate();

    /**
     * @brief Reads the daily value and TID from an OR/AS save
     */
    void loadSave();
};

#endif // MIRAGE6_FORM_HPP
