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

#ifndef IDS6_HPP
#define IDS6_HPP

#include <QWidget>

class IDModel6;

namespace Ui
{
    class IDs6;
}

/**
 * @brief TID/SID generator for X/Y and OR/AS
 */
class IDs6 final : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new IDs6 object
     *
     * @param parent Parent widget, which takes memory ownership
     */
    IDs6(QWidget *parent = nullptr);

    /**
     * @brief Destroy the IDs6 object
     */
    ~IDs6() override;

private:
    Ui::IDs6 *ui;

    IDModel6 *model;

private slots:
    /**
     * @brief Generates TID/SID from the TinyMT
     */
    void generate();

    /**
     * @brief Shows the state fields that the source needs
     *
     * @param index Source index
     */
    void sourceIndexChanged(int index);
};

#endif // IDS6_HPP
