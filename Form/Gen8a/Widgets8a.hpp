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

#ifndef WIDGETS8A_HPP
#define WIDGETS8A_HPP

#include <Core/Global.hpp>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QString>
#include <array>

class QStandardItem;

/**
 * @brief Small helpers shared by the Legends: Arceus windows (built in code)
 */
namespace Widgets8a
{
    /**
     * @brief Six IV spin boxes (HP, Atk, Def, SpA, SpD, Spe) in one row
     */
    struct IVRow
    {
        std::array<QSpinBox *, 6> boxes;

        IVRow(QGridLayout *layout, int row, int column, int value = 0);
        std::array<u8, 6> values() const;
    };

    /**
     * @brief Nature combo box; \p any adds an "Any" entry with data -1
     */
    QComboBox *natureBox(bool any);

    /**
     * @brief Combo box with an "Unknown" entry (data -1) and the given entries (data 0..n-1)
     */
    QComboBox *choiceBox(const QStringList &entries, const QString &unknown = QObject::tr("Unknown"));

    QSpinBox *spinBox(int min, int max, int value);

    QString hex(u64 value, int width);

    QStandardItem *item(const QString &text);

    QString ivText(const std::array<u8, 6> &ivs);

    QString genderText(u8 gender);
}

#endif // WIDGETS8A_HPP
