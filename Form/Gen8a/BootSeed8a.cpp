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

#include "BootSeed8a.hpp"
#include <Form/Gen8a/StockBootSeed8a.hpp>
#include <Core/Gen8a/Searchers/BootSeedSearcher8a.hpp>
#include <Core/Util/OpenCL.hpp>
#include <QCheckBox>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QStandardItemModel>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>

static const QString settingPrefix = QStringLiteral("bootSeed8a");

BootSeed8a::BootSeed8a(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(tr("Legends: Arceus boot seed (Unown)"));

    auto *layout = new QVBoxLayout(this);
    auto *note = new QLabel(tr("Close the game fully, start it, and load a save in an area with at least two uncaught Unown. The game "
                               "makes them on that first load, back to back, about 120-170 shared-RNG draws after boot. Catch them "
                               "and enter what the summary shows, in the order of their letters (A..Z, !, ?), which is the order "
                               "the game makes them. Both Unown abilities are Levitate, so leave Ability on Unknown unless you "
                               "know the slot. Weight ranges break ties between candidates."));
    note->setWordWrap(true);
    layout->addWidget(note);

    auto *grid = new QGridLayout();
    grid->addWidget(new QLabel(tr("Use")), 0, 0);
    grid->addWidget(new QLabel(tr("IVs")), 0, 1);
    grid->addWidget(new QLabel(tr("Nature")), 0, 7);
    grid->addWidget(new QLabel(tr("Ability")), 0, 8);
    grid->addWidget(new QLabel(tr("Weight scalar")), 0, 9, 1, 2);
    for (int r = 0; r < 3; r++)
    {
        UnownRow row;
        row.use = new QCheckBox(tr("Unown %1").arg(r + 1));
        row.use->setChecked(r < 2);
        row.use->setEnabled(r == 2);
        grid->addWidget(row.use, r + 1, 0);
        row.ivs = new Widgets8a::IVRow(grid, r + 1, 1);
        row.nature = Widgets8a::natureBox(false);
        grid->addWidget(row.nature, r + 1, 7);
        row.ability = Widgets8a::choiceBox({ tr("Slot 1"), tr("Slot 2") });
        grid->addWidget(row.ability, r + 1, 8);
        row.weightMin = Widgets8a::spinBox(0, 255, 0);
        row.weightMax = Widgets8a::spinBox(0, 255, 255);
        grid->addWidget(row.weightMin, r + 1, 9);
        grid->addWidget(row.weightMax, r + 1, 10);
        rows.emplace_back(row);
    }
    layout->addLayout(grid);

    auto *options = new QGridLayout();
    minAdvance = Widgets8a::spinBox(0, 2000, 120);
    maxAdvance = Widgets8a::spinBox(0, 2000, 170);
    maxGap = Widgets8a::spinBox(3, 40, 14);
    minAdvance->setToolTip(tr("Draws from boot to the first Unown's EC. Every console boot seen so far: 123-163"));
    maxGap->setToolTip(tr("Draws between the first Unown's ability and the second Unown's EC: 3 plus the nature and weight redraws"));
    options->addWidget(new QLabel(tr("Start advance")), 0, 0);
    options->addWidget(minAdvance, 0, 1);
    options->addWidget(new QLabel(tr("to")), 0, 2);
    options->addWidget(maxAdvance, 0, 3);
    options->addWidget(new QLabel(tr("Max gap")), 0, 4);
    options->addWidget(maxGap, 0, 5);
    gpu = new QCheckBox(OpenCL::isAvailable() ? tr("GPU: %1").arg(QString::fromStdString(OpenCL::deviceName())) : tr("No OpenCL GPU"));
    gpu->setChecked(OpenCL::isAvailable());
    gpu->setEnabled(OpenCL::isAvailable());
    options->addWidget(gpu, 0, 6);
    searchButton = new QPushButton(tr("Search"));
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
    layout->addWidget(result);

    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({ tr("Boot seed"), tr("First Unown at"), tr("Advance after the Unown"), tr("Weights") });
    auto *table = new QTableView();
    table->setModel(model);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(table);

    auto *stockButton = new QPushButton(tr("No IVs? Stock console search from five Unown (effort levels)..."));
    stockButton->setToolTip(tr("Exact IVs need a dump or a memory read: the game only shows effort levels. This opens the search that "
                               "works from what the summary shows"));
    layout->addWidget(stockButton);
    connect(stockButton, &QPushButton::clicked, this, [] {
        auto *window = new StockBootSeed8a();
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->show();
        window->raise();
    });

    connect(searchButton, &QPushButton::clicked, this, &BootSeed8a::search);

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

BootSeed8a::~BootSeed8a()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    for (auto &row : rows)
    {
        delete row.ivs;
    }
}

void BootSeed8a::search()
{
    std::vector<UnownObservation8a> unown;
    for (const auto &row : rows)
    {
        if (!row.use->isChecked())
        {
            continue;
        }
        UnownObservation8a obs;
        obs.ivs = row.ivs->values();
        obs.nature = static_cast<u8>(row.nature->currentData().toInt());
        obs.ability = static_cast<s8>(row.ability->currentData().toInt());
        obs.weightMin = static_cast<u8>(row.weightMin->value());
        obs.weightMax = static_cast<u8>(row.weightMax->value());
        unown.emplace_back(obs);
    }
    if (minAdvance->value() > maxAdvance->value())
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid range"), tr("The first start advance is above the last one"));
        msg.exec();
        return;
    }

    model->removeRows(0, model->rowCount());
    searchButton->setEnabled(false);
    cancelButton->setEnabled(true);
    progress->setValue(0);
    result->clear();

    auto *searcher = new BootSeedSearcher8a(unown, minAdvance->value(), maxAdvance->value(), maxGap->value());
    QSettings settings;
    searcher->startSearch(settings.value("settings/threads").toInt(), gpu->isChecked());

    auto *timer = new QTimer(this);
    connect(cancelButton, &QPushButton::clicked, timer, [searcher] { searcher->cancelSearch(); });
    connect(timer, &QTimer::timeout, this, [this, searcher, timer] {
        bool searching = searcher->isSearching();
        for (const auto &found : searcher->getResults())
        {
            QStringList weights;
            for (u8 w : found.weights)
            {
                weights.append(QString::number(w));
            }
            model->appendRow({ Widgets8a::item(Widgets8a::hex(found.seed, 16)), Widgets8a::item(QString::number(found.advance)),
                               Widgets8a::item(QString::number(found.end)), Widgets8a::item(weights.join(", ")) });
        }
        progress->setValue(searcher->getProgress());

        if (!searching)
        {
            timer->stop();
            searchButton->setEnabled(true);
            cancelButton->setEnabled(false);
            progress->setValue(100);
            std::string error = searcher->getError();
            bool usedGPU = searcher->getUsedGPU();
            delete searcher;
            timer->deleteLater();

            if (!error.empty() && !usedGPU)
            {
                result->setText(tr("Searched on the CPU: %1").arg(QString::fromStdString(error)));
            }
            if (model->rowCount() > 1)
            {
                result->setText(result->text() + " "
                                + tr("%1 seeds fit: compare the weights with the summary, or add a third Unown").arg(model->rowCount()));
            }
            else if (model->rowCount() == 1)
            {
                result->setText(result->text() + " " + tr("One seed fits. Use it in the Unown / Static / Gift window"));
            }
            else
            {
                result->setText(result->text() + " " + tr("No seed fits: check the IVs, the order and the start advance range"));
            }
        }
    });
    timer->start(250);
}
