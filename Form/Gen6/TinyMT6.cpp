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

#include "TinyMT6.hpp"
#include "ui_TinyMT6.h"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Gen6/TinyTimeline6.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/Util/Translator.hpp>
#include <QSettings>
#include <QStandardItemModel>

static const QString settingPrefix = QStringLiteral("tinymt6");

TinyMT6::TinyMT6(QWidget *parent) : QWidget(parent), ui(new Ui::TinyMT6)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    model = new QStandardItemModel(this);
    ui->tableView->setModel(model);

    ui->textBoxState0->setValues(InputType::Seed32Bit);
    ui->textBoxState1->setValues(InputType::Seed32Bit);
    ui->textBoxState2->setValues(InputType::Seed32Bit);
    ui->textBoxState3->setValues(InputType::Seed32Bit);

    ui->comboBoxGame->setup({ toInt(Game::X), toInt(Game::Y) });
    ui->comboBoxLead->setup({ toInt(Lead::None), toInt(Lead::Synchronize), toInt(Lead::CuteCharmF), toInt(Lead::CompoundEyes) });

    ui->comboBoxSource->setToolTip(tr("The TinyMT state is the 4 words at RAM 0x8C52808 (X/Y); the seed is the console clock in ms about "
                                      "0.58 s after the game starts"));
    ui->labelIdleDraws->setToolTip(tr("Idle draws that fall between the field resume and the trigger (player idle about every 3-4 s, NPC "
                                      "timers, weather). On the quietest maps this is 0-3; each row of the table assumes one value"));
    ui->labelPreAdvance->setToolTip(tr("Fixed TinyMT draws from the Use press to the trigger: 3 per party member on the bag close, then "
                                       "the field resume (rand(90), 8 x 3 on outdoor maps, one rand(4) per loaded ceiling shadow, rand(8), "
                                       "rand(60)); Sweet Scent adds one rand(240) and has no party term. Live-verified in 9 zones"));

    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &TinyMT6::generate);
    connect(ui->comboBoxSource, &QComboBox::currentIndexChanged, this, &TinyMT6::sourceIndexChanged);
    connect(ui->comboBoxGame, &QComboBox::currentIndexChanged, this, &TinyMT6::gameIndexChanged);
    connect(ui->comboBoxZone, &QComboBox::currentIndexChanged, this, &TinyMT6::updatePreAdvance);
    connect(ui->comboBoxMethod, &QComboBox::currentIndexChanged, this, &TinyMT6::updatePreAdvance);
    connect(ui->spinBoxParty, &QSpinBox::valueChanged, this, &TinyMT6::updatePreAdvance);

    sourceIndexChanged(0);
    gameIndexChanged(0);

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

TinyMT6::~TinyMT6()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

const EncounterArea6 *TinyMT6::currentHordes() const
{
    int index = ui->comboBoxZone->currentIndex();
    if (index < 0 || index >= static_cast<int>(zones.size()))
    {
        return nullptr;
    }
    u16 location = zones[index]->location;
    for (const auto &area : hordes)
    {
        if (area.getLocation() == location)
        {
            return &area;
        }
    }
    return nullptr;
}

void TinyMT6::gameIndexChanged(int index)
{
    if (index < 0)
    {
        return;
    }
    Game version = ui->comboBoxGame->getEnum<Game>();
    hordes = Encounters6::getEncounters(Encounter6::HordeHoney, version);

    int size;
    const ZoneModel6 *table = ZoneModels6::getZones(&size);
    zones.clear();
    std::vector<u16> locations;
    for (int i = 0; i < size; i++)
    {
        zones.emplace_back(&table[i]);
        locations.emplace_back(table[i].location);
    }
    auto names = Translator::getLocations(locations, version);

    int current = ui->comboBoxZone->currentIndex();
    ui->comboBoxZone->clear();
    for (size_t i = 0; i < zones.size(); i++)
    {
        QString name = QString::fromStdString(names[i]);
        if (zones[i]->shadows != 0)
        {
            name += tr(" (zone %1, %2 shadows)").arg(zones[i]->zone).arg(zones[i]->shadows);
        }
        else if (ZoneModels6::getZones(zones[i]->location).size() > 1)
        {
            name += tr(" (zone %1)").arg(zones[i]->zone);
        }
        if (!zones[i]->live)
        {
            name += tr(" [unverified]");
        }
        ui->comboBoxZone->addItem(name);
    }
    ui->comboBoxZone->setCurrentIndex(current >= 0 && current < ui->comboBoxZone->count() ? current : 0);
    updatePreAdvance();
}

void TinyMT6::sourceIndexChanged(int index)
{
    bool state = index == 0;
    ui->labelState1->setVisible(state);
    ui->labelState2->setVisible(state);
    ui->labelState3->setVisible(state);
    ui->textBoxState1->setVisible(state);
    ui->textBoxState2->setVisible(state);
    ui->textBoxState3->setVisible(state);
    ui->labelStateSeed0->setText(state ? tr("State 0") : tr("Seed"));
}

void TinyMT6::updatePreAdvance()
{
    int index = ui->comboBoxZone->currentIndex();
    if (index < 0 || index >= static_cast<int>(zones.size()))
    {
        ui->labelPreAdvanceValue->setText("-");
        return;
    }
    const ZoneModel6 &zone = *zones[index];
    bool honey = ui->comboBoxMethod->currentIndex() == 0;
    u32 preAdvance = honey ? zone.honeyPreAdvance(static_cast<u8>(ui->spinBoxParty->value())) : zone.sweetScentPreAdvance();
    ui->labelPreAdvanceValue->setText(QString::number(preAdvance));
    ui->spinBoxParty->setEnabled(honey);
}

void TinyMT6::generate()
{
    model->clear();
    model->setHorizontalHeaderLabels({ tr("Idle Draws"), tr("Total Draws"), tr("Lead Roll"), tr("Lead"), tr("Group"), tr("Horde"),
                                       tr("HA Member"), tr("Extra"), tr("Items") });

    int index = ui->comboBoxZone->currentIndex();
    const EncounterArea6 *area = currentHordes();
    if (index < 0 || index >= static_cast<int>(zones.size()) || !area)
    {
        return;
    }
    const ZoneModel6 &zone = *zones[index];
    Game version = ui->comboBoxGame->getEnum<Game>();
    auto lead = ui->comboBoxLead->getEnum<Lead>();
    bool honey = ui->comboBoxMethod->currentIndex() == 0;
    u8 party = static_cast<u8>(ui->spinBoxParty->value());
    auto names = area->getNames();

    TinyMT base = ui->comboBoxSource->currentIndex() == 0
        ? TinyMT(ui->textBoxState0->getUInt(), ui->textBoxState1->getUInt(), ui->textBoxState2->getUInt(), ui->textBoxState3->getUInt())
        : TinyMT(ui->textBoxState0->getUInt());

    for (int idle = 0; idle <= ui->spinBoxIdleDraws->value(); idle++)
    {
        TinyTimeline6 timeline(base, version);
        if (honey)
        {
            timeline.bagClose(party);
            timeline.fieldResume(zone);
        }
        else
        {
            timeline.fieldResume(zone);
            timeline.rand(240); // Pokemon-display draw about 54 ticks after the block
        }
        for (int i = 0; i < idle; i++)
        {
            timeline.next();
        }
        u32 fixed = timeline.getDraws();
        HordeDecision6 decision = timeline.hordeTrigger();

        QString leadText = tr("-");
        if (lead == Lead::Synchronize)
        {
            leadText = decision.lead < 50 ? tr("Synchronize") : tr("No");
        }
        else if (lead == Lead::CuteCharmF)
        {
            leadText = decision.lead < 67 ? tr("Cute Charm") : tr("No");
        }

        QStringList items;
        if (decision.group < area->getHordeCount())
        {
            auto members = area->getHorde(decision.group);
            std::array<const PersonalInfo *, 5> infos;
            for (size_t i = 0; i < 5; i++)
            {
                infos[i] = members[i].getInfo();
            }
            // X/Y: one draw per member at its EC + block, in member order
            auto held = timeline.hordeItems(infos, lead);
            for (u16 item : held)
            {
                items.append(item == 0 ? "-" : QString::fromStdString(Translator::getItem(item)));
            }
        }

        QList<QStandardItem *> row;
        row.append(new QStandardItem(QString::number(idle)));
        row.append(new QStandardItem(QString::number(fixed)));
        row.append(new QStandardItem(QString::number(decision.lead)));
        row.append(new QStandardItem(leadText));
        row.append(new QStandardItem(QString::number(decision.group + 1)));
        row.append(new QStandardItem(decision.group < names.size() ? QString::fromStdString(names[decision.group]) : tr("-")));
        row.append(new QStandardItem(decision.haMember == 0 ? tr("None") : QString::number(decision.haMember)));
        row.append(new QStandardItem(QString::number(decision.extra)));
        row.append(new QStandardItem(items.join(", ")));
        for (auto *item : row)
        {
            item->setEditable(false);
        }
        model->appendRow(row);
    }
    ui->tableView->resizeColumnsToContents();
}
