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

#include "Spawner8a.hpp"
#include <Core/Gen8a/SpawnerGenerator8a.hpp>
#include <Core/Util/OpenCL.hpp>
#include <Core/Util/Translator.hpp>
#include <QCheckBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QStandardItemModel>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>

static const QString settingPrefix = QStringLiteral("spawner8a");

WildObservation8a Spawner8a::MonInput::read(u8 rolls, u8 ratio) const
{
    WildObservation8a obs;
    obs.ivs = ivs->values();
    obs.nature = static_cast<s8>(nature->currentData().toInt());
    obs.ability = static_cast<s8>(ability->currentData().toInt());
    obs.gender = static_cast<s8>(gender->currentData().toInt());
    obs.shiny = shiny->isChecked() ? 1 : 0;
    obs.rolls = rolls;
    obs.genderRatio = ratio;
    obs.heightMin = static_cast<u8>(heightMin->value());
    obs.heightMax = static_cast<u8>(heightMax->value());
    obs.weightMin = static_cast<u8>(weightMin->value());
    obs.weightMax = static_cast<u8>(weightMax->value());
    return obs;
}

Spawner8a::Spawner8a(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(tr("Legends: Arceus spawners"));
    auto *layout = new QVBoxLayout(this);

    // Group seed recovery
    auto *solve = new QGroupBox(tr("Group seed from two Pokémon of one spawner (experimental, not tried on a console)"));
    auto *solveLayout = new QVBoxLayout(solve);
    auto *note = new QLabel(tr("Catch a Pokémon from a spawner, reload so it refills, catch the next one (or two from one fill). "
                               "Not for alphas. Step 1 (GPU) lists the entity seeds that fit the first Pokémon; step 2 costs about "
                               "half a second per entity seed and CPU thread, so narrow height and weight scalars are needed: "
                               "exact values leave a few thousand seeds (about 2 minutes on 15 threads)."));
    note->setWordWrap(true);
    solveLayout->addWidget(note);

    auto *grid = new QGridLayout();
    grid->addWidget(new QLabel(tr("IVs")), 0, 1);
    grid->addWidget(new QLabel(tr("Nature")), 0, 7);
    grid->addWidget(new QLabel(tr("Ability")), 0, 8);
    grid->addWidget(new QLabel(tr("Gender")), 0, 9);
    grid->addWidget(new QLabel(tr("Height")), 0, 11, 1, 2);
    grid->addWidget(new QLabel(tr("Weight")), 0, 13, 1, 2);
    for (int m = 0; m < 2; m++)
    {
        MonInput &mon = mons[m];
        grid->addWidget(new QLabel(m == 0 ? tr("First") : tr("Second")), m + 1, 0);
        mon.ivs = std::make_unique<Widgets8a::IVRow>(grid, m + 1, 1);
        mon.nature = Widgets8a::natureBox(true);
        mon.ability = Widgets8a::choiceBox({ tr("Slot 1"), tr("Slot 2") });
        mon.gender = Widgets8a::choiceBox({ tr("Male"), tr("Female") });
        mon.shiny = new QCheckBox(tr("Shiny"));
        mon.heightMin = Widgets8a::spinBox(0, 255, 0);
        mon.heightMax = Widgets8a::spinBox(0, 255, 255);
        mon.weightMin = Widgets8a::spinBox(0, 255, 0);
        mon.weightMax = Widgets8a::spinBox(0, 255, 255);
        grid->addWidget(mon.nature, m + 1, 7);
        grid->addWidget(mon.ability, m + 1, 8);
        grid->addWidget(mon.gender, m + 1, 9);
        grid->addWidget(mon.shiny, m + 1, 10);
        grid->addWidget(mon.heightMin, m + 1, 11);
        grid->addWidget(mon.heightMax, m + 1, 12);
        grid->addWidget(mon.weightMin, m + 1, 13);
        grid->addWidget(mon.weightMax, m + 1, 14);
    }
    solveLayout->addLayout(grid);

    auto *options = new QHBoxLayout();
    rolls = Widgets8a::spinBox(1, 40, 1);
    rolls->setToolTip(tr("Shiny rolls: 1, +1 research level 10, +2 more when perfect, +3 Shiny Charm, +25 outbreak, +12 massive outbreak"));
    ratio = Widgets8a::spinBox(0, 255, 127);
    ratio->setToolTip(tr("Species gender ratio: 0 male only, 254 female only, 255 genderless, 31 = 87.5% male, 127 = 50%, 191 = 25% male"));
    maxCandidates = Widgets8a::spinBox(1, 100000000, 200000);
    gpu = new QCheckBox(OpenCL::isAvailable() ? tr("GPU: %1").arg(QString::fromStdString(OpenCL::deviceName())) : tr("No OpenCL GPU"));
    gpu->setChecked(OpenCL::isAvailable());
    gpu->setEnabled(OpenCL::isAvailable());
    searchButton = new QPushButton(tr("Search"));
    cancelButton = new QPushButton(tr("Cancel"));
    cancelButton->setEnabled(false);
    options->addWidget(new QLabel(tr("Shiny rolls")));
    options->addWidget(rolls);
    options->addWidget(new QLabel(tr("Gender ratio")));
    options->addWidget(ratio);
    options->addWidget(new QLabel(tr("Max entity seeds")));
    options->addWidget(maxCandidates);
    options->addWidget(gpu);
    options->addWidget(searchButton);
    options->addWidget(cancelButton);
    solveLayout->addLayout(options);

    progress = new QProgressBar();
    progress->setRange(0, 100);
    solveLayout->addWidget(progress);
    status = new QLabel();
    status->setWordWrap(true);
    solveLayout->addWidget(status);

    results = new QStandardItemModel(this);
    results->setHorizontalHeaderLabels(
        { tr("Group seed"), tr("Relation"), tr("Slot seed 1"), tr("Entity seed 1"), tr("Slot seed 2"), tr("Entity seed 2") });
    auto *resultTable = new QTableView();
    resultTable->setModel(results);
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    resultTable->setMaximumHeight(140);
    solveLayout->addWidget(resultTable);
    layout->addWidget(solve);

    // Prediction
    auto *predictBox = new QGroupBox(tr("Future spawns from a group seed"));
    auto *predictLayout = new QVBoxLayout(predictBox);
    auto *row = new QHBoxLayout();
    groupSeed = new QLineEdit();
    groupSeed->setInputMask("HHHHHHHHHHHHHHHH");
    fills = Widgets8a::spinBox(1, 100000, 50);
    fillSize = Widgets8a::spinBox(1, 8, 1);
    fillSize->setToolTip(tr("Pokémon per fill: a reload refills a spawner only when all its slots are empty"));
    shinyOnly = new QCheckBox(tr("Shiny only"));
    auto *predictButton = new QPushButton(tr("Generate"));
    row->addWidget(new QLabel(tr("Group seed")));
    row->addWidget(groupSeed);
    row->addWidget(new QLabel(tr("Fills")));
    row->addWidget(fills);
    row->addWidget(new QLabel(tr("Pokémon per fill")));
    row->addWidget(fillSize);
    row->addWidget(shinyOnly);
    row->addWidget(predictButton);
    predictLayout->addLayout(row);

    spawns = new QStandardItemModel(this);
    spawns->setHorizontalHeaderLabels({ tr("Fill"), tr("Slot"), tr("Group seed"), tr("Slot roll"), tr("Entity seed"), tr("EC"), tr("PID"),
                                        tr("Shiny"), tr("IVs"), tr("Ability"), tr("Gender"), tr("Nature"), tr("Height"), tr("Weight") });
    auto *spawnTable = new QTableView();
    spawnTable->setModel(spawns);
    spawnTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    predictLayout->addWidget(spawnTable);
    layout->addWidget(predictBox);

    connect(searchButton, &QPushButton::clicked, this, &Spawner8a::search);
    connect(predictButton, &QPushButton::clicked, this, &Spawner8a::predict);

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Spawner8a::~Spawner8a()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();
}

void Spawner8a::search()
{
    WildObservation8a first = mons[0].read(rolls->value(), ratio->value());
    WildObservation8a second = mons[1].read(rolls->value(), ratio->value());

    results->removeRows(0, results->rowCount());
    searchButton->setEnabled(false);
    cancelButton->setEnabled(true);
    progress->setValue(0);
    status->setText(tr("Step 1: entity seeds of the first Pokémon..."));

    auto *searcher = new SpawnerSearcher8a(first, second);
    QSettings settings;
    searcher->startSearch(settings.value("settings/threads").toInt(), gpu->isChecked(), maxCandidates->value());

    auto *timer = new QTimer(this);
    connect(cancelButton, &QPushButton::clicked, timer, [searcher] { searcher->cancelSearch(); });
    connect(timer, &QTimer::timeout, this, [this, searcher, timer] {
        bool searching = searcher->isSearching();
        for (const auto &found : searcher->getResults())
        {
            QString relation = found.relation == SpawnerRelation8a::SameFill ? tr("same fill, slot %1").arg(found.position)
                                                                             : tr("next fill after %1").arg(found.position);
            results->appendRow({ Widgets8a::item(Widgets8a::hex(found.groupSeed, 16)), Widgets8a::item(relation),
                                 Widgets8a::item(Widgets8a::hex(found.slot1, 16)), Widgets8a::item(Widgets8a::hex(found.entity1, 16)),
                                 Widgets8a::item(Widgets8a::hex(found.slot2, 16)), Widgets8a::item(Widgets8a::hex(found.entity2, 16)) });
        }
        u64 candidates = searcher->getCandidates();
        if (candidates)
        {
            status->setText(tr("Step 2: %1 entity seeds, finding their slot seeds").arg(candidates));
        }
        progress->setValue(searcher->getProgress());

        if (!searching)
        {
            timer->stop();
            searchButton->setEnabled(true);
            cancelButton->setEnabled(false);
            progress->setValue(100);
            std::string error = searcher->getError();
            delete searcher;
            timer->deleteLater();
            if (!error.empty() && results->rowCount() == 0)
            {
                status->setText(QString::fromStdString(error));
            }
            else
            {
                status->setText(tr("%1 group seeds fit. The first one's fill is fill 0 in the prediction below").arg(results->rowCount()));
                if (results->rowCount() > 0)
                {
                    groupSeed->setText(results->item(0, 0)->text());
                }
            }
        }
    });
    timer->start(250);
}

void Spawner8a::predict()
{
    bool ok;
    u64 seed = groupSeed->text().toULongLong(&ok, 16);
    if (!ok)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid seed"), tr("Enter the 16-digit group seed"));
        msg.exec();
        return;
    }

    auto states
        = SpawnerGenerator8a::generate(seed, fills->value(), fillSize->value(), rolls->value(), ratio->value(), shinyOnly->isChecked());
    spawns->removeRows(0, spawns->rowCount());
    const auto &natures = Translator::getNatures();
    for (const auto &s : states)
    {
        const State8a &p = s.state;
        spawns->appendRow({ Widgets8a::item(QString::number(s.fill)), Widgets8a::item(QString::number(s.slot)),
                            Widgets8a::item(Widgets8a::hex(s.groupSeed, 16)), Widgets8a::item(QString::number(s.slotRoll, 'f', 4)),
                            Widgets8a::item(Widgets8a::hex(s.entitySeed, 16)), Widgets8a::item(Widgets8a::hex(p.ec, 8)),
                            Widgets8a::item(Widgets8a::hex(p.pid, 8)), Widgets8a::item(p.shiny ? tr("Yes") : tr("No")),
                            Widgets8a::item(Widgets8a::ivText(p.ivs)), Widgets8a::item(QString::number(p.ability + 1)),
                            Widgets8a::item(Widgets8a::genderText(p.gender)), Widgets8a::item(QString::fromStdString(natures[p.nature])),
                            Widgets8a::item(QString::number(p.height)), Widgets8a::item(QString::number(p.weight)) });
    }
}
