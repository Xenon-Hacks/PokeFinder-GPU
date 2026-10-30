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

#include "FishingModel7.hpp"
#include <Core/Util/Translator.hpp>

FishingModel7::FishingModel7(QObject *parent) : TableModel(parent), timer { -64.0f, 29.913f }, showStats(false)
{
}

int FishingModel7::columnCount(const QModelIndex &parent) const
{
    return 24;
}

QVariant FishingModel7::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        const auto &state = model[index.row()];
        int column = index.column();
        bool pokemon = state.getResult() == FishingResult7::Pokemon;
        switch (column)
        {
        case 0:
            return state.getFrame();
        case 1:
            return QString::number(timer.seconds(state.getFrame()), 'f', 2);
        case 2:
            return state.getLastFrame() == state.getFrame() ? tr("1 frame") : tr("%1 frames").arg(state.getLastFrame() - state.getFrame() + 1);
        case 3:
            return state.getAdvances();
        case 4:
            return QString::number(timer.seconds(state.getBiteFrame()), 'f', 2);
        case 5:
            return QString::number(timer.seconds(state.getReelFrame()), 'f', 2);
        case 6:
            return QString("%1 (%2)").arg(state.getBiteAdvance()).arg(state.getBiteRoll());
        case 7:
            return state.getResult() == FishingResult7::NoBite ? tr("-") : QString("%1 (%2)").arg(state.getHookAdvance()).arg(state.getHookRoll());
        case 8:
            switch (state.getResult())
            {
            case FishingResult7::NoBite:
                return tr("Nothing bites");
            case FishingResult7::Item:
                return tr("Item");
            default:
                return tr("Pokémon");
            }
        case 9:
            if (pokemon)
            {
                return QString::fromStdString(Translator::getSpecie(state.getSpecie(), state.getForm()));
            }
            return state.getItem() == 0 ? tr("-") : QString::fromStdString(Translator::getItem(state.getItem()));
        case 10:
            return pokemon ? QVariant(state.getLevel()) : QVariant(tr("-"));
        case 11:
            return pokemon ? QString::number(state.getEC(), 16).toUpper().rightJustified(8, '0') : tr("-");
        case 12:
            return pokemon ? QString::number(state.getPID(), 16).toUpper().rightJustified(8, '0') : tr("-");
        case 13:
        {
            if (!pokemon)
            {
                return tr("-");
            }
            u8 shiny = state.getShiny();
            return shiny == 2 ? tr("Square") : shiny == 1 ? tr("Star") : tr("No");
        }
        case 14:
            return pokemon ? QString::fromStdString(Translator::getNature(state.getNature())) : tr("-");
        case 15:
            return pokemon ? QString("%1: %2").arg(state.getAbility() + 1).arg(QString::fromStdString(Translator::getAbility(state.getAbilityIndex())))
                           : tr("-");
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
            if (!pokemon)
            {
                return tr("-");
            }
            return showStats ? state.getStat(column - 16) : state.getIV(column - 16);
        case 22:
            return pokemon ? QString::fromStdString(Translator::getGender(state.getGender())) : tr("-");
        case 23:
            return pokemon && state.getItem() != 0 ? QString::fromStdString(Translator::getItem(state.getItem())) : tr("-");
        }
    }
    return QVariant();
}

QVariant FishingModel7::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return header[section];
    }
    return QVariant();
}

void FishingModel7::setTimer(const Timer7 &t)
{
    timer = t;
    emit dataChanged(index(0, 1), index(rowCount() - 1, 5), { Qt::DisplayRole });
}

void FishingModel7::setShowStats(bool flag)
{
    showStats = flag;
    emit dataChanged(index(0, 16), index(rowCount() - 1, 21), { Qt::DisplayRole });
}
