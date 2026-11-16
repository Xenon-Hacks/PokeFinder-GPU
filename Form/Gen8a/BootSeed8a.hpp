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

#ifndef BOOTSEED8A_HPP
#define BOOTSEED8A_HPP

#include <Form/Gen8a/Widgets8a.hpp>
#include <QWidget>
#include <vector>

class QCheckBox;
class QLabel;
class QProgressBar;
class QPushButton;
class QStandardItemModel;

/**
 * @brief Legends: Arceus boot seed from the Unown made on the first area load (IVs, nature, ability, weight)
 */
class BootSeed8a final : public QWidget
{
    Q_OBJECT
public:
    BootSeed8a(QWidget *parent = nullptr);
    ~BootSeed8a() override;

private:
    struct UnownRow
    {
        Widgets8a::IVRow *ivs;
        QComboBox *nature;
        QComboBox *ability;
        QSpinBox *weightMin;
        QSpinBox *weightMax;
        QCheckBox *use;
    };

    std::vector<UnownRow> rows;
    QSpinBox *minAdvance;
    QSpinBox *maxAdvance;
    QSpinBox *maxGap;
    QCheckBox *gpu;
    QPushButton *searchButton;
    QPushButton *cancelButton;
    QProgressBar *progress;
    QLabel *result;
    QStandardItemModel *model;

private slots:
    void search();
};

#endif // BOOTSEED8A_HPP
