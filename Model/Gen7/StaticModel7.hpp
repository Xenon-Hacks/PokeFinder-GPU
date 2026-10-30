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

#ifndef STATICMODEL7_HPP
#define STATICMODEL7_HPP

#include <Core/Parents/States/State.hpp>
#include <Model/TableModel.hpp>

/**
 * @brief Provides a table model implementation to show Gen 7 static, gift and trade results
 */
class StaticModel7 final : public TableModel<GeneratorState>
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new StaticModel7 object
     *
     * @param parent Parent object, which takes memory ownership
     */
    StaticModel7(QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public slots:
    /**
     * @brief Toggles between IVs and stats
     */
    void setShowStats(bool flag);

private:
    QStringList header = { tr("Advances"), tr("EC"),  tr("PID"), tr("Shiny"), tr("Nature"),       tr("Ability"), tr("HP"),
                           tr("Atk"),      tr("Def"), tr("SpA"), tr("SpD"),   tr("Spe"),          tr("Hidden Power"), tr("Gender"),
                           tr("Characteristic") };
    bool showStats;
};

#endif // STATICMODEL7_HPP
