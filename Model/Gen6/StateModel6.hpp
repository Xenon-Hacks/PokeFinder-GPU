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

#ifndef STATEMODEL6_HPP
#define STATEMODEL6_HPP

#include <Core/Gen6/States/EggState6.hpp>
#include <Core/Gen6/States/State6.hpp>
#include <Core/Gen6/Searchers/SeedSearcher6.hpp>
#include <Core/Parents/States/IDState.hpp>
#include <Model/TableModel.hpp>

/**
 * @brief Provides a table model implementation for X/Y static, wild and Wonder Card states
 */
class StateModel6 final : public TableModel<State6>
{
    Q_OBJECT
public:
    static constexpr int seedColumn = 16; ///< First of the seed window columns (Seed, Console Time)
    static constexpr int spotsColumn = 18; ///< Horde hit spots

    StateModel6(QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public slots:
    /**
     * @brief Sets whether stats are shown instead of IVs
     *
     * @param flag Whether to show stats
     */
    void setShowStats(bool flag);

private:
    QStringList header = { tr("Advances"), tr("Horde"), tr("EC"),  tr("PID"), tr("Shiny"),  tr("Nature"),        tr("Ability"), tr("HP"),
                           tr("Atk"),      tr("Def"),   tr("SpA"), tr("SpD"), tr("Spe"),    tr("Hidden Power"), tr("Gender"),  tr("Characteristic"),
                           tr("Seed"),     tr("Console Time"), tr("Hit Spots") };
    bool showStats;
};

/**
 * @brief Provides a table model implementation for X/Y eggs
 */
class EggModel6 final : public TableModel<EggState6>
{
    Q_OBJECT
public:
    EggModel6(QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public slots:
    /**
     * @brief Sets whether inheritance is shown instead of IVs
     *
     * @param flag Whether to show inheritance
     */
    void setShowInheritance(bool flag);

    /**
     * @brief Sets whether stats are shown instead of IVs
     *
     * @param flag Whether to show stats
     */
    void setShowStats(bool flag);

private:
    QStringList header = { tr("Advances"), tr("Egg Seed"), tr("EC"),  tr("PID"), tr("Shiny"), tr("Nature"), tr("Ability"),       tr("HP"),
                           tr("Atk"),      tr("Def"),      tr("SpA"), tr("SpD"), tr("Spe"),   tr("Gender"), tr("Characteristic") };
    bool showInheritance;
    bool showStats;
};

/**
 * @brief Provides a table model implementation for X/Y TID/SID states
 */
class IDModel6 final : public TableModel<IDState>
{
    Q_OBJECT
public:
    IDModel6(QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QStringList header = { tr("Advances"), tr("TID"), tr("SID"), tr("TSV") };
};

/**
 * @brief Provides a table model implementation for X/Y seed search results
 */
class SeedModel6 final : public TableModel<SeedResult6>
{
    Q_OBJECT
public:
    SeedModel6(QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QStringList header = { tr("Seed"), tr("Frame"), tr("Console Time") };
};

#endif // STATEMODEL6_HPP
