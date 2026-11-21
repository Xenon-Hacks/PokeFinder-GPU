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

#ifndef IVCALC8A_HPP
#define IVCALC8A_HPP

#include <Form/Gen8a/Widgets8a.hpp>
#include <QWidget>
#include <array>

/**
 * @brief IV calculator for Legends: Arceus: from the stats and effort levels the summary shows to the IV range of each stat
 */
class IVCalc8a final : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new IVCalc8a object
     *
     * @param parent Parent widget
     */
    IVCalc8a(QWidget *parent = nullptr);

    /**
     * @brief Destroy the IVCalc8a object
     */
    ~IVCalc8a() override;

private:
    QComboBox *species;
    QSpinBox *level;
    QComboBox *nature;
    QSpinBox *flawless;
    std::array<QSpinBox *, 6> stats;
    std::array<QSpinBox *, 6> effort;
    std::array<QSpinBox *, 6> grit;
    std::array<QLabel *, 6> ranges;
    QLabel *note;

private slots:
    /**
     * @brief Recomputes the IV ranges
     */
    void update();
};

#endif // IVCALC8A_HPP
