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

#include "Wild6.hpp"
#include "ui_Wild6.h"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Gen6/Generators/WildGenerator6.hpp>
#include <Core/Gen6/Profile6.hpp>
#include <Core/Gen6/States/State6.hpp>
#include <Core/Util/Translator.hpp>
#include <Form/Controls/Controls.hpp>
#include <Form/Gen6/SeedWindowUI6.hpp>
#include <Model/Gen6/StateModel6.hpp>
#include <QSettings>

static const QString settingPrefix = QStringLiteral("wild6");

Wild6::Wild6(QWidget *parent) : QWidget(parent), ui(new Ui::Wild6)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::XY);

    model = new StateModel6(ui->tableView);
    ui->tableView->setModel(model);
    SeedWindowUI6::setup(ui->checkBoxWindow, ui->dateTimeEditStart, ui->dateTimeEditEnd, ui->textBoxSeed, ui->labelOdds, ui->tableView);

    ui->textBoxSeed->setValues(InputType::Seed32Bit);
    ui->textBoxInitialAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxMaxAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxDelay->setValues(InputType::Advance32Bit);

    ui->labelInitialAdvances->setToolTip(
        tr("MT frame of the trigger: the encounter step, the Rock Smash Yes press, the fishing bite or the Honey press"));
    ui->labelDelay->setToolTip(tr("MT frames from the trigger to the 60-frame skip before the EC"));
    ui->comboBoxEncounter->setToolTip(tr("Species, slot, lead check and held item come from the TinyMT and are not modelled here"));

    ui->comboMenuLead->addAction(tr("None"), toInt(Lead::None));
    ui->comboMenuLead->addMenu(tr("Cute Charm"), { { tr("♂ Lead"), toInt(Lead::CuteCharmM) }, { tr("♀ Lead"), toInt(Lead::CuteCharmF) } });
    ui->comboMenuLead->addMenu(tr("Synchronize"), Translator::getNatures());

    for (u16 specie = 1; specie <= 721; specie++)
    {
        ui->comboBoxPokemon->addItem(QString::fromStdString(Translator::getSpecie(specie)), specie);
    }

    ui->filter->disableControls(Controls::Height | Controls::Weight | Controls::Wild);
    ui->filter->enableHiddenAbility();

    connect(ui->profileDisplay, &ProfileDisplay6::profileChanged, this, &Wild6::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay6::profilesChanged, this, &Wild6::profilesChanged);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Wild6::generate);
    connect(ui->comboBoxEncounter, &QComboBox::currentIndexChanged, this, &Wild6::encounterIndexChanged);
    connect(ui->filter, &Filter::showStatsChanged, model, &StateModel6::setShowStats);

    updateProfiles();
    encounterIndexChanged(0);

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Wild6::~Wild6()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void Wild6::updateProfiles()
{
    ui->profileDisplay->updateProfiles();
}

void Wild6::encounterIndexChanged(int index)
{
    if (index >= 0)
    {
        auto encounter = static_cast<Encounter6>(index);
        ui->textBoxDelay->setText(QString::number(WildGenerator6::getDefaultDelay(encounter)));
        ui->spinBoxIVCount->setValue(encounter == Encounter6::FriendSafari ? 2 : 0);
        ui->checkBoxShinyLocked->setEnabled(encounter == Encounter6::Grass);
        if (encounter != Encounter6::Grass)
        {
            ui->checkBoxShinyLocked->setChecked(false);
        }
    }
}

void Wild6::generate()
{
    if (!ui->filter->isValid())
    {
        return;
    }

    model->clearModel();

    u32 seed = ui->textBoxSeed->getUInt();
    u32 initialAdvances = ui->textBoxInitialAdvances->getUInt();
    u32 maxAdvances = ui->textBoxMaxAdvances->getUInt();
    u32 delay = ui->textBoxDelay->getUInt();
    auto lead = ui->comboMenuLead->getEnum<Lead>();
    auto encounter = static_cast<Encounter6>(ui->comboBoxEncounter->currentIndex());

    auto filter = ui->filter->getFilter<StateFilter>();
    WildGenerator6 generator(initialAdvances, maxAdvances, delay, encounter, lead, ui->comboBoxPokemon->getCurrentUShort(),
                             ui->spinBoxLevel->value(), ui->spinBoxIVCount->value(), ui->checkBoxShinyLocked->isChecked(),
                             *currentProfile, filter);

    if (ui->checkBoxWindow->isChecked())
    {
        SeedWindowUI6::run(generator, currentProfile->getSaveSeed(), maxAdvances, ui->dateTimeEditStart, ui->dateTimeEditEnd, ui->labelOdds,
                           model);
        return;
    }

    auto states = generator.generate(seed);
    model->addItems(states);
}

void Wild6::profileChanged(const Profile6 &profile)
{
    currentProfile = &profile;
}
