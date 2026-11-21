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

#ifndef STOCKBOOTSEED8A_HPP
#define STOCKBOOTSEED8A_HPP

#include <Form/Gen8a/Widgets8a.hpp>
#include <QWidget>
#include <vector>

class QDoubleSpinBox;
class QProgressBar;
class QPushButton;

/**
 * @brief Boot seed from five Unown using only what an unmodified console shows (effort levels, nature, weight)
 */
class StockBootSeed8a final : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new StockBootSeed8a object
     *
     * @param parent Parent widget
     */
    StockBootSeed8a(QWidget *parent = nullptr);

    /**
     * @brief Destroy the StockBootSeed8a object
     */
    ~StockBootSeed8a() override;

private:
    struct UnownRow
    {
        Widgets8a::IVRow *effort;
        QComboBox *nature;
        QDoubleSpinBox *weight;
    };

    std::vector<UnownRow> rows;
    QSpinBox *minAdvance;
    QSpinBox *maxAdvance;
    QSpinBox *maxExtra;
    QPushButton *searchButton;
    QPushButton *cancelButton;
    QProgressBar *progress;
    QLabel *result;

private slots:
    /**
     * @brief Runs the search
     */
    void search();
};

#endif // STOCKBOOTSEED8A_HPP
