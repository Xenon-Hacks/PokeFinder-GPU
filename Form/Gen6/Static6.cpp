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

#include "Static6.hpp"
#include "ui_Static6.h"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Gen6/Encounters6.hpp>
#include <Core/Gen6/Generators/StaticGenerator6.hpp>
#include <Core/Gen6/Profile6.hpp>
#include <Core/Gen6/States/State6.hpp>
#include <Core/Gen6/StaticTemplate6.hpp>
#include <Core/Util/Translator.hpp>
#include <Form/Controls/Controls.hpp>
#include <Form/Gen6/SeedWindowUI6.hpp>
#include <Model/Gen6/StateModel6.hpp>
#include <QSettings>

static const QString settingPrefix = QStringLiteral("static6");

Static6::Static6(QWidget *parent) : QWidget(parent), ui(new Ui::Static6)
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

    ui->labelSeed->setToolTip(tr("Initial MT seed: save Misc[0x00] + console ms when the title screen loaded (Seed Finder)"));
    ui->labelInitialAdvances->setToolTip(tr("MT frame of the button press (or the step that starts the encounter)"));
    ui->labelDelay->setToolTip(tr("MT frames from the press to the 60-frame skip before the EC"));

    ui->comboMenuLead->addAction(tr("None"), toInt(Lead::None));
    ui->comboMenuLead->addMenu(tr("Synchronize"), Translator::getNatures());

    ui->comboBoxShiny->setup({ toInt(Shiny::Never), toInt(Shiny::Random) });
    ui->comboBoxAbility->setup({ 0, 1, 2, 255 });

    ui->filter->disableControls(Controls::Height | Controls::Weight | Controls::Wild);

    connect(ui->profileDisplay, &ProfileDisplay6::profileChanged, this, &Static6::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay6::profilesChanged, this, &Static6::profilesChanged);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Static6::generate);
    connect(ui->comboBoxCategory, &QComboBox::currentIndexChanged, this, &Static6::categoryIndexChanged);
    connect(ui->comboBoxPokemon, &QComboBox::currentIndexChanged, this, &Static6::pokemonIndexChanged);
    connect(ui->filter, &Filter::showStatsChanged, model, &StateModel6::setShowStats);

    updateProfiles();
    categoryIndexChanged(0);

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Static6::~Static6()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void Static6::updateProfiles()
{
    ui->profileDisplay->updateProfiles();
}

void Static6::categoryIndexChanged(int index)
{
    if (index >= 0)
    {
        int size;
        const StaticTemplate6 *templates = Encounters6::getStaticEncounters(index, &size);

        ui->comboBoxPokemon->clear();
        for (int i = 0; i < size; i++)
        {
            if ((currentProfile->getVersion() & templates[i].getVersion()) != Game::None)
            {
                ui->comboBoxPokemon->addItem(
                    QString::fromStdString(Translator::getSpecie(templates[i].getSpecie(), templates[i].getForm())), i);
            }
        }
    }
}

void Static6::generate()
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
    const StaticTemplate6 *staticTemplate
        = Encounters6::getStaticEncounter(ui->comboBoxCategory->currentIndex(), ui->comboBoxPokemon->getCurrentInt());

    auto filter = ui->filter->getFilter<StateFilter>();
    StaticGenerator6 generator(initialAdvances, maxAdvances, delay, lead, *staticTemplate, *currentProfile, filter);

    if (ui->checkBoxWindow->isChecked())
    {
        SeedWindowUI6::run(generator, currentProfile->getSaveSeed(), maxAdvances, ui->dateTimeEditStart, ui->dateTimeEditEnd, ui->labelOdds,
                           model);
        return;
    }

    auto states = generator.generate(seed);
    model->addItems(states);
}

void Static6::pokemonIndexChanged(int index)
{
    if (index >= 0)
    {
        const StaticTemplate6 *staticTemplate
            = Encounters6::getStaticEncounter(ui->comboBoxCategory->currentIndex(), ui->comboBoxPokemon->getCurrentInt());
        ui->spinBoxLevel->setValue(staticTemplate->getLevel());
        ui->comboBoxAbility->setCurrentIndex(ui->comboBoxAbility->findData(staticTemplate->getAbility()));
        ui->comboBoxShiny->setCurrentIndex(ui->comboBoxShiny->findData(toInt(staticTemplate->getShiny())));
        ui->spinBoxIVCount->setValue(staticTemplate->getIVCount());
        ui->textBoxDelay->setText(QString::number(staticTemplate->getDelay()));
        ui->comboMenuLead->setEnabled(!staticTemplate->getGift());
    }
}

void Static6::profileChanged(const Profile6 &profile)
{
    currentProfile = &profile;
    categoryIndexChanged(ui->comboBoxCategory->currentIndex());
}
