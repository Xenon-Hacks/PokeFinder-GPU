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

#ifndef SEEDFINDER7_HPP
#define SEEDFINDER7_HPP

#include <Core/Global.hpp>
#include <QWidget>
#include <vector>

class QStandardItemModel;

namespace Ui
{
    class SeedFinder7;
}

/**
 * @brief Finds the Gen 7 boot seed from the Continue clock needles with the GPU
 */
class SeedFinder7 final : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new SeedFinder7 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    SeedFinder7(QWidget *parent = nullptr);

    /**
     * @brief Destroy the SeedFinder7 object
     */
    ~SeedFinder7() override;

private:
    Ui::SeedFinder7 *ui;
    QStandardItemModel *model;

    /**
     * @brief Parses the needle text box: 0-16 values separated by spaces or commas
     *
     * @param ok Set to whether the text was valid
     */
    std::vector<u8> needles(bool *ok) const;

private slots:
    void search();
};

#endif // SEEDFINDER7_HPP
