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

#ifndef SPAWNER8A_HPP
#define SPAWNER8A_HPP

#include <Core/Gen8a/Searchers/SpawnerSearcher8a.hpp>
#include <Form/Gen8a/Widgets8a.hpp>
#include <QWidget>
#include <memory>

class QCheckBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QStandardItemModel;

/**
 * @brief Legends: Arceus wild spawners: group seed from two Pokémon (experimental) and future spawns from a group seed
 */
class Spawner8a final : public QWidget
{
    Q_OBJECT
public:
    Spawner8a(QWidget *parent = nullptr);
    ~Spawner8a() override;

private:
    struct MonInput
    {
        std::unique_ptr<Widgets8a::IVRow> ivs;
        QComboBox *nature;
        QComboBox *ability;
        QComboBox *gender;
        QCheckBox *shiny;
        QSpinBox *heightMin;
        QSpinBox *heightMax;
        QSpinBox *weightMin;
        QSpinBox *weightMax;

        WildObservation8a read(u8 rolls, u8 ratio) const;
    };

    MonInput mons[2];
    QSpinBox *rolls;
    QSpinBox *ratio;
    QSpinBox *maxCandidates;
    QCheckBox *gpu;
    QPushButton *searchButton;
    QPushButton *cancelButton;
    QProgressBar *progress;
    QLabel *status;
    QStandardItemModel *results;

    QLineEdit *groupSeed;
    QSpinBox *fills;
    QSpinBox *fillSize;
    QCheckBox *shinyOnly;
    QStandardItemModel *spawns;

private slots:
    void search();
    void predict();
};

#endif // SPAWNER8A_HPP
