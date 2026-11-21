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

#include "StockBootSeed8a.hpp"
#include <Core/Gen8a/Searchers/StockBootSeedSearcher8a.hpp>
#include <Core/Util/OpenCL.hpp>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>
#include <cmath>
#include <memory>

static const QString settingPrefix = QStringLiteral("stockBootSeed8a");

StockBootSeed8a::StockBootSeed8a(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(tr("Legends: Arceus boot seed (stock console, five Unown)"));

    auto *layout = new QVBoxLayout(this);
    auto *note
        = new QLabel(tr("For a console with no dumps or memory reads. Save at a camp in an area that still has five uncaught Unown, "
                        "close the game fully, start it and load. The game makes the five back to back about 120-150 draws after "
                        "boot. Catch all five, then press HOME (the shared RNG stops there) and enter what each summary shows, in "
                        "the order the game makes them. The game never shows IVs, only the effort level each one starts at "
                        "(0 = IV 0-19, 1 = 20-25, 2 = 26-30, 3 = 31), so the search works from those, the nature and the weight "
                        "in kg. It needs an OpenCL GPU: a few minutes to a few hours when one effort level is 3, up to about a "
                        "day otherwise."));
    note->setWordWrap(true);
    layout->addWidget(note);

    auto *grid = new QGridLayout();
    grid->addWidget(new QLabel(tr("Effort levels (HP, Atk, Def, SpA, SpD, Spe)")), 0, 1, 1, 6);
    grid->addWidget(new QLabel(tr("Nature")), 0, 7);
    grid->addWidget(new QLabel(tr("Weight (kg)")), 0, 8);
    for (int r = 0; r < 5; r++)
    {
        UnownRow row;
        grid->addWidget(new QLabel(tr("Unown %1").arg(r + 1)), r + 1, 0);
        row.effort = new Widgets8a::IVRow(grid, r + 1, 1);
        for (auto *box : row.effort->boxes)
        {
            box->setRange(0, 3);
        }
        row.nature = Widgets8a::natureBox(false);
        grid->addWidget(row.nature, r + 1, 7);
        row.weight = new QDoubleSpinBox();
        row.weight->setRange(4.0, 6.0);
        row.weight->setDecimals(1);
        row.weight->setSingleStep(0.1);
        row.weight->setValue(5.0);
        grid->addWidget(row.weight, r + 1, 8);
        rows.emplace_back(row);
    }
    layout->addLayout(grid);

    auto *options = new QGridLayout();
    minAdvance = Widgets8a::spinBox(0, 500, 118);
    maxAdvance = Widgets8a::spinBox(0, 500, 152);
    maxExtra = Widgets8a::spinBox(0, 12, 7);
    minAdvance->setToolTip(tr("Draws from boot to the first Unown's EC. Seen so far: 123-148"));
    maxExtra->setToolTip(tr("Redraws searched between two Unown (weight and nature). 7 covers 98% of batches"));
    options->addWidget(new QLabel(tr("Start advance")), 0, 0);
    options->addWidget(minAdvance, 0, 1);
    options->addWidget(new QLabel(tr("to")), 0, 2);
    options->addWidget(maxAdvance, 0, 3);
    options->addWidget(new QLabel(tr("Max redraws")), 0, 4);
    options->addWidget(maxExtra, 0, 5);
    bool gpu = OpenCL::isAvailable();
    options->addWidget(new QLabel(gpu ? tr("GPU: %1").arg(QString::fromStdString(OpenCL::deviceName())) : tr("No OpenCL GPU")), 0, 6);
    searchButton = new QPushButton(tr("Search"));
    searchButton->setEnabled(gpu);
    cancelButton = new QPushButton(tr("Cancel"));
    cancelButton->setEnabled(false);
    options->addWidget(searchButton, 0, 7);
    options->addWidget(cancelButton, 0, 8);
    layout->addLayout(options);

    progress = new QProgressBar();
    progress->setRange(0, 100);
    layout->addWidget(progress);
    result = new QLabel();
    result->setWordWrap(true);
    result->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(result);

    connect(searchButton, &QPushButton::clicked, this, &StockBootSeed8a::search);

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

StockBootSeed8a::~StockBootSeed8a()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    for (auto &row : rows)
    {
        delete row.effort;
    }
}

void StockBootSeed8a::search()
{
    std::vector<StockUnown8a> unown;
    for (const auto &row : rows)
    {
        StockUnown8a obs;
        obs.effort = row.effort->values();
        obs.nature = static_cast<u8>(row.nature->currentData().toInt());
        obs.weight = static_cast<u16>(std::lround(row.weight->value() * 10));
        unown.emplace_back(obs);
    }
    if (minAdvance->value() > maxAdvance->value())
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid range"), tr("The first start advance is above the last one"));
        msg.exec();
        return;
    }

    searchButton->setEnabled(false);
    cancelButton->setEnabled(true);
    progress->setValue(0);
    result->clear();

    auto *searcher = new StockBootSeedSearcher8a(unown, minAdvance->value(), maxAdvance->value(), maxExtra->value());
    u64 total = searcher->getAlignments();
    searcher->startSearch();

    auto clock = std::make_shared<QElapsedTimer>();
    clock->start();
    auto seeds = std::make_shared<QStringList>();
    auto userCancel = std::make_shared<bool>(false);
    auto *timer = new QTimer(this);
    connect(cancelButton, &QPushButton::clicked, timer, [searcher, userCancel] {
        *userCancel = true;
        searcher->cancelSearch();
    });
    connect(timer, &QTimer::timeout, this, [=, this] {
        bool searching = searcher->isSearching();
        for (const auto &found : searcher->getResults())
        {
            seeds->append(tr("Boot seed %1, first Unown %2 draws after boot").arg(Widgets8a::hex(found.seed, 16)).arg(found.advance));
        }
        int minutes = static_cast<int>(clock->elapsed() / 60000);
        if (searching)
        {
            progress->setValue(searcher->getProgress());
            result->setText(tr("Searching %1 alignments, most likely first: %2% done, %3 min").arg(total).arg(searcher->getProgress()).arg(minutes));
            return;
        }

        timer->stop();
        searchButton->setEnabled(true);
        cancelButton->setEnabled(false);
        std::string error = searcher->getError();
        delete searcher;
        timer->deleteLater();
        if (!*userCancel && error.empty())
        {
            progress->setValue(100);
        }

        if (!error.empty())
        {
            result->setText(QString::fromStdString(error));
        }
        else if (!seeds->isEmpty())
        {
            result->setText(seeds->join("\n") + "\n" + tr("Use it in the Unown / Static / Gift window (%1 min)").arg(minutes));
        }
        else if (*userCancel)
        {
            result->setText(tr("Cancelled"));
        }
        else
        {
            result->setText(tr("No seed fits: check the values and their order, or widen the start advance and redraw ranges"));
        }
    });
    timer->start(500);
}
