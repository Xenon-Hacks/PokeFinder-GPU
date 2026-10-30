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

#ifndef IDS7_HPP
#define IDS7_HPP

#include <QWidget>

class IDModel8;

namespace Ui
{
    class IDs7;
}

/**
 * @brief Provides the Gen 7 trainer ID window: the ID is one main draw at the name confirmation (advance 1012 on Sun/Moon)
 */
class IDs7 final : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new IDs7 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    IDs7(QWidget *parent = nullptr);

    /**
     * @brief Destroy the IDs7 object
     */
    ~IDs7() override;

private:
    Ui::IDs7 *ui;
    IDModel8 *model;

private slots:
    void gameIndexChanged(int index);
    void generate();
};

#endif // IDS7_HPP
