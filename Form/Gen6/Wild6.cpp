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
#include <QStandardItemModel>

static const QString settingPrefix = QStringLiteral("wild6");

Wild6::Wild6(QWidget *parent) : QWidget(parent), ui(new Ui::Wild6)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::Gen6);

    model = new StateModel6(ui->tableView);
    ui->tableView->setModel(model);
    SeedWindowUI6::setup(ui->checkBoxWindow, ui->dateTimeEditStart, ui->dateTimeEditEnd, ui->textBoxSeed, ui->labelOdds, ui->tableView);

    ui->textBoxSeed->setValues(InputType::Seed32Bit);
    ui->textBoxInitialAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxMaxAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxDelay->setValues(InputType::Advance32Bit);

    ui->labelInitialAdvances->setToolTip(
        tr("MT frame of the trigger: the encounter step, the Rock Smash Yes press, the fishing bite or the Honey / Sweet Scent press"));
    ui->labelDelay->setToolTip(tr("MT frames from the trigger to the 60-output container before the EC (Azahar values, calibrate on hardware). "
                                  "Honey 195 on X/Y, 177 on OR/AS; Sweet Scent 339 on X, 321 on OR, 317 on AS"));
    ui->comboBoxEncounter->setToolTip(tr("Which slot appears, the lead check and held items come from the TinyMT and are not modelled: "
                                         "pick the target you are after"));
    ui->comboBoxLocation->setToolTip(tr("Locations that have this encounter type in the selected game"));
    ui->comboBoxHAMember->setToolTip(tr("Horde member with the hidden ability (chosen on the TinyMT); it skips its ability draw, so the "
                                        "members after it start one output earlier"));
    ui->comboBoxLocation->enableAutoComplete();
    ui->comboBoxPokemon->enableAutoComplete();

    ui->comboMenuLead->addAction(tr("None"), toInt(Lead::None));
    ui->comboMenuLead->addMenu(tr("Cute Charm"), { { tr("♂ Lead"), toInt(Lead::CuteCharmM) }, { tr("♀ Lead"), toInt(Lead::CuteCharmF) } });
    ui->comboMenuLead->addMenu(tr("Synchronize"), Translator::getNatures());

    ui->filter->disableControls(Controls::Height | Controls::Weight | Controls::Wild);
    ui->filter->enableHiddenAbility();

    connect(ui->profileDisplay, &ProfileDisplay6::profileChanged, this, &Wild6::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay6::profilesChanged, this, &Wild6::profilesChanged);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Wild6::generate);
    connect(ui->comboBoxEncounter, &QComboBox::currentIndexChanged, this, &Wild6::encounterIndexChanged);
    connect(ui->comboBoxLocation, &QComboBox::currentIndexChanged, this, &Wild6::locationIndexChanged);
    connect(ui->comboBoxPokemon, &QComboBox::currentIndexChanged, this, &Wild6::pokemonIndexChanged);
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

const EncounterArea6 *Wild6::currentArea() const
{
    int index = ui->comboBoxLocation->currentIndex();
    return index >= 0 && index < static_cast<int>(areas.size()) ? &areas[index] : nullptr;
}

void Wild6::encounterIndexChanged(int index)
{
    if (index >= 0)
    {
        auto encounter = static_cast<Encounter6>(index);
        areas = Encounters6::getEncounters(encounter, currentProfile->getVersion());

        std::vector<u16> locations;
        for (const auto &area : areas)
        {
            locations.emplace_back(area.getLocation());
        }

        u16 current = ui->comboBoxLocation->getCurrentUShort();
        ui->comboBoxLocation->clear();
        ui->comboBoxLocation->addItems(Translator::getLocations(locations, currentProfile->getVersion()), locations);
        ui->comboBoxLocation->setCurrentIndexByData(current);

        bool horde = isHorde(encounter);
        ui->labelHaMember->setVisible(horde);
        ui->comboBoxHAMember->setVisible(horde);
        ui->tableView->setColumnHidden(StateModel6::spotsColumn, !horde);
        ui->spinBoxIVCount->setValue(encounter == Encounter6::FriendSafari ? 2 : 0);
        ui->checkBoxShinyLocked->setEnabled(encounter == Encounter6::Grass);
        if (encounter != Encounter6::Grass)
        {
            ui->checkBoxShinyLocked->setChecked(false);
        }

        if (ui->comboBoxLocation->currentIndex() < 0)
        {
            locationIndexChanged(-1);
        }
    }
}

void Wild6::locationIndexChanged(int index)
{
    const EncounterArea6 *area = currentArea();

    ui->comboBoxPokemon->clear();
    if (area)
    {
        auto names = area->getNames();
        for (size_t i = 0; i < names.size(); i++)
        {
            ui->comboBoxPokemon->addItem(QString::fromStdString(names[i]), static_cast<int>(i));
        }
        ui->textBoxDelay->setText(QString::number(area->getDelay()));
    }
}

void Wild6::pokemonIndexChanged(int index)
{
    const EncounterArea6 *area = currentArea();
    if (!area || index < 0)
    {
        ui->labelLevelValue->setText("-");
        return;
    }

    if (isHorde(area->getEncounter()))
    {
        auto members = area->getHorde(index);
        ui->labelLevelValue->setText(QString::number(members[0].getMaxLevel()));
    }
    else
    {
        const Slot &slot = area->getPokemon(index);
        ui->labelLevelValue->setText(slot.getMinLevel() == slot.getMaxLevel()
                                         ? QString::number(slot.getMaxLevel())
                                         : QString("%1-%2").arg(slot.getMinLevel()).arg(slot.getMaxLevel()));
    }
}

void Wild6::generate()
{
    const EncounterArea6 *area = currentArea();
    int target = ui->comboBoxPokemon->currentIndex();
    if (!area || target < 0 || !ui->filter->isValid())
    {
        return;
    }

    model->clearModel();

    u32 seed = ui->textBoxSeed->getUInt();
    u32 initialAdvances = ui->textBoxInitialAdvances->getUInt();
    u32 maxAdvances = ui->textBoxMaxAdvances->getUInt();
    u32 delay = ui->textBoxDelay->getUInt();
    auto lead = ui->comboMenuLead->getEnum<Lead>();
    Encounter6 encounter = area->getEncounter();

    std::vector<Slot> targets = isHorde(encounter) ? area->getHorde(target) : std::vector<Slot> { area->getPokemon(target) };
    u8 haMember = isHorde(encounter) ? static_cast<u8>(ui->comboBoxHAMember->currentIndex()) : 0;

    auto filter = ui->filter->getFilter<StateFilter>();
    WildGenerator6 generator(initialAdvances, maxAdvances, delay, encounter, lead, targets, haMember, ui->spinBoxIVCount->value(),
                             ui->checkBoxShinyLocked->isChecked(), *currentProfile, filter);

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

    // Only offer the encounter types the profile's game has (flowers, ambush and Friend Safari on X/Y, long grass on OR/AS)
    auto *items = qobject_cast<QStandardItemModel *>(ui->comboBoxEncounter->model());
    for (int i = 0; i < ui->comboBoxEncounter->count(); i++)
    {
        items->item(i)->setEnabled(hasEncounter(static_cast<Encounter6>(i), profile.getVersion()));
    }
    if (!hasEncounter(static_cast<Encounter6>(ui->comboBoxEncounter->currentIndex()), profile.getVersion()))
    {
        ui->comboBoxEncounter->setCurrentIndex(0);
    }

    encounterIndexChanged(ui->comboBoxEncounter->currentIndex());
}
