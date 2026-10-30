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

#include "EggModel7.hpp"
#include <Core/Util/Translator.hpp>

static QString hex32(u32 value)
{
    return QString::number(value, 16).toUpper().rightJustified(8, '0');
}

EggModel7::EggModel7(QObject *parent) : TableModel(parent), showInheritance(false), showStats(false)
{
}

int EggModel7::columnCount(const QModelIndex &parent) const
{
    return 16;
}

QVariant EggModel7::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        const auto &state = model[index.row()];
        int column = index.column();
        switch (column)
        {
        case 0:
            return state.getAdvances();
        case 1:
        {
            const auto &s = state.getState();
            return QString("%1 %2 %3 %4").arg(hex32(s[0]), hex32(s[1]), hex32(s[2]), hex32(s[3]));
        }
        case 2:
            return QString("%1 (%2)").arg(state.getDraws()).arg(state.getDraws() + 2);
        case 3:
            return hex32(state.getEC());
        case 4:
            return state.getMainPID() ? tr("%1 (main RNG)").arg(hex32(state.getPID())) : hex32(state.getPID());
        case 5:
        {
            u8 shiny = state.getShiny();
            return shiny == 2 ? tr("Square") : shiny == 1 ? tr("Star") : tr("No");
        }
        case 6:
            return QString::fromStdString(Translator::getNature(state.getNature()));
        case 7:
            if (state.getAbility() == 0 || state.getAbility() == 1)
            {
                return QString("%1: %2")
                    .arg(state.getAbility() + 1)
                    .arg(QString::fromStdString(Translator::getAbility(state.getAbilityIndex())));
            }
            return QString("H (%1)").arg(QString::fromStdString(Translator::getAbility(state.getAbilityIndex())));
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            if (showInheritance)
            {
                u8 inh = state.getInheritance(column - 8);
                if (inh)
                {
                    return inh == 1 ? "A" : "B";
                }
            }
            return showStats ? state.getStat(column - 8) : state.getIV(column - 8);
        case 14:
            return QString::fromStdString(Translator::getGender(state.getGender()));
        case 15:
            return QString::fromStdString(Translator::getCharacteristic(state.getCharacteristic(), CharacteristicGeneration::Gen7));
        }
    }
    return QVariant();
}

QVariant EggModel7::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return header[section];
    }
    return QVariant();
}

void EggModel7::setShowInheritance(bool flag)
{
    showInheritance = flag;
    emit dataChanged(index(0, 8), index(rowCount() - 1, 13), { Qt::DisplayRole });
}

void EggModel7::setShowStats(bool flag)
{
    showStats = flag;
    emit dataChanged(index(0, 8), index(rowCount() - 1, 13), { Qt::DisplayRole });
}
