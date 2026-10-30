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

#ifndef TINYMT6_HPP
#define TINYMT6_HPP

#include <Core/Gen6/EncounterArea6.hpp>
#include <QWidget>
#include <vector>

class QStandardItemModel;
struct ZoneModel6;

namespace Ui
{
    class TinyMT6;
}

/**
 * @brief Provides the TinyMT (overworld RNG) tools for X/Y: the Honey and Sweet Scent pre-advance of a zone and, from a synced
 * TinyMT state, which horde group, hidden-ability member and held items each number of idle draws before the trigger gives
 */
class TinyMT6 final : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new TinyMT6 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    TinyMT6(QWidget *parent = nullptr);

    /**
     * @brief Destroy the TinyMT6 object
     */
    ~TinyMT6() override;

private:
    Ui::TinyMT6 *ui;
    QStandardItemModel *model;
    std::vector<const ZoneModel6 *> zones;
    std::vector<EncounterArea6> hordes;

    /**
     * @brief Returns the horde area of the selected zone's location, nullptr when the zone has none
     */
    const EncounterArea6 *currentHordes() const;

private slots:
    /**
     * @brief Fills the result table
     */
    void generate();

    /**
     * @brief Refreshes the zone list for the selected game
     *
     * @param index Game index
     */
    void gameIndexChanged(int index);

    /**
     * @brief Toggles between TinyMT state and seed inputs
     *
     * @param index Source index
     */
    void sourceIndexChanged(int index);

    /**
     * @brief Updates the pre-advance label
     */
    void updatePreAdvance();
};

#endif // TINYMT6_HPP
