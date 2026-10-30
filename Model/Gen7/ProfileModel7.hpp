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

#ifndef PROFILEMODEL7_HPP
#define PROFILEMODEL7_HPP

#include <Core/Gen7/Profile7.hpp>
#include <Model/TableModel.hpp>

/**
 * @brief Provides a table model implementation to show Gen 7 profile information
 */
class ProfileModel7 final : public TableModel<Profile7>
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new ProfileModel7 object
     *
     * @param parent Parent object, which takes memory ownership
     */
    ProfileModel7(QObject *parent);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

private:
    QStringList header = { tr("Profile Name"), tr("Version"), tr("TID"), tr("SID"), tr("Shiny Charm"), tr("Oval Charm") };
};

#endif // PROFILEMODEL7_HPP
