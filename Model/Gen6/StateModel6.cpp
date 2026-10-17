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

#include "StateModel6.hpp"
#include <Core/Util/Translator.hpp>
#include <QDateTime>
#include <QTimeZone>

static QString hex32(u32 value)
{
    return QString::number(value, 16).toUpper().rightJustified(8, '0');
}

static QString shinyText(u8 shiny)
{
    return shiny == 2 ? QObject::tr("Square") : shiny == 1 ? QObject::tr("Star") : QObject::tr("No");
}

static QString abilityText(u8 ability, u16 abilityIndex)
{
    if (ability == 0 || ability == 1)
    {
        return QString("%1: %2").arg(ability).arg(QString::fromStdString(Translator::getAbility(abilityIndex)));
    }
    return QString("H: %1").arg(QString::fromStdString(Translator::getAbility(abilityIndex)));
}

StateModel6::StateModel6(QObject *parent) : TableModel(parent), showStats(false)
{
}

int StateModel6::columnCount(const QModelIndex &parent) const
{
    return 16;
}

QVariant StateModel6::data(const QModelIndex &index, int role) const
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
            return state.getMember() == 0 ? QString("-") : QString::number(state.getMember());
        case 2:
            return hex32(state.getEC());
        case 3:
            return hex32(state.getPID());
        case 4:
            return shinyText(state.getShiny());
        case 5:
            return QString::fromStdString(Translator::getNature(state.getNature()));
        case 6:
            return abilityText(state.getAbility(), state.getAbilityIndex());
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            return showStats ? state.getStat(column - 7) : state.getIV(column - 7);
        case 13:
            return QString::fromStdString(Translator::getHiddenPower(state.getHiddenPower()));
        case 14:
            return QString::fromStdString(Translator::getGender(state.getGender()));
        case 15:
            return QString::fromStdString(Translator::getCharacteristic(state.getCharacteristic(), CharacteristicGeneration::Gen6));
        }
    }

    return QVariant();
}

QVariant StateModel6::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return header[section];
    }
    return QVariant();
}

void StateModel6::setShowStats(bool flag)
{
    showStats = flag;
    emit dataChanged(index(0, 7), index(rowCount() - 1, 12), { Qt::DisplayRole });
}

EggModel6::EggModel6(QObject *parent) : TableModel(parent), showInheritance(false), showStats(false)
{
}

int EggModel6::columnCount(const QModelIndex &parent) const
{
    return 15;
}

QVariant EggModel6::data(const QModelIndex &index, int role) const
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
            return QString::number(state.getSeed(), 16).toUpper().rightJustified(16, '0');
        case 2:
            return hex32(state.getEC());
        case 3:
            return state.getMainPID() ? tr("%1 (main RNG)").arg(hex32(state.getPID())) : hex32(state.getPID());
        case 4:
            return shinyText(state.getShiny());
        case 5:
            return QString::fromStdString(Translator::getNature(state.getNature()));
        case 6:
            return abilityText(state.getAbility(), state.getAbilityIndex());
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            if (showInheritance)
            {
                u8 inh = state.getInheritance(column - 7);
                if (inh)
                {
                    return inh == 1 ? "A" : "B";
                }
            }
            return showStats ? state.getStat(column - 7) : state.getIV(column - 7);
        case 13:
            return QString::fromStdString(Translator::getGender(state.getGender()));
        case 14:
            return QString::fromStdString(Translator::getCharacteristic(state.getCharacteristic(), CharacteristicGeneration::Gen6));
        }
    }

    return QVariant();
}

QVariant EggModel6::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return header[section];
    }
    return QVariant();
}

void EggModel6::setShowInheritance(bool flag)
{
    showInheritance = flag;
    emit dataChanged(index(0, 7), index(rowCount() - 1, 12), { Qt::DisplayRole });
}

void EggModel6::setShowStats(bool flag)
{
    showStats = flag;
    emit dataChanged(index(0, 7), index(rowCount() - 1, 12), { Qt::DisplayRole });
}

IDModel6::IDModel6(QObject *parent) : TableModel(parent)
{
}

int IDModel6::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant IDModel6::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        const auto &state = model[index.row()];
        switch (index.column())
        {
        case 0:
            return state.getAdvances();
        case 1:
            return state.getTID();
        case 2:
            return state.getSID();
        case 3:
            return state.getTSV();
        }
    }

    return QVariant();
}

QVariant IDModel6::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return header[section];
    }
    return QVariant();
}

SeedModel6::SeedModel6(QObject *parent) : TableModel(parent)
{
}

int SeedModel6::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant SeedModel6::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        const auto &result = model[index.row()];
        switch (index.column())
        {
        case 0:
            return hex32(result.seed);
        case 1:
            return result.frame;
        case 2:
            if (result.ms == 0)
            {
                return QString("-");
            }
            return QDateTime(QDate(2000, 1, 1), QTime(0, 0), QTimeZone::UTC)
                .addMSecs(static_cast<qint64>(result.ms))
                .toString("yyyy-MM-dd hh:mm:ss.zzz");
        }
    }

    return QVariant();
}

QVariant SeedModel6::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        return header[section];
    }
    return QVariant();
}
