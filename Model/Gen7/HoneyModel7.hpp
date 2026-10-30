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

#ifndef HONEYMODEL7_HPP
#define HONEYMODEL7_HPP

#include <Core/Gen7/States/HoneyState7.hpp>
#include <Core/Gen7/Timer7.hpp>
#include <Model/TableModel.hpp>

/**
 * @brief Provides a table model implementation to show Gen 7 Honey results, one row per press window
 */
class HoneyModel7 final : public TableModel<HoneyState7>
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new HoneyModel7 object
     *
     * @param parent Parent object, which takes memory ownership
     */
    HoneyModel7(QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * @brief Sets the timer used for the press-time column
     */
    void setTimer(const Timer7 &timer);

public slots:
    /**
     * @brief Toggles between IVs and stats
     */
    void setShowStats(bool flag);

private:
    QStringList header = { tr("Frame"), tr("Press (s)"), tr("Window"), tr("Bag Adv"), tr("Lead Adv"), tr("Slot"), tr("Pokémon"), tr("Level"),
                           tr("EC"),    tr("PID"),       tr("Shiny"),  tr("Nature"),  tr("Ability"),  tr("HP"),   tr("Atk"),     tr("Def"),
                           tr("SpA"),   tr("SpD"),       tr("Spe"),    tr("Hidden Power"), tr("Gender"), tr("Item"), tr("Special") };
    Timer7 timer;
    bool showStats;
};

#endif // HONEYMODEL7_HPP
