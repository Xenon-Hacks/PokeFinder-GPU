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

#include "Pelago7.hpp"
#include "ui_Pelago7.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen7/Generators/PelagoGenerator7.hpp>
#include <Core/Gen7/Profile7.hpp>
#include <Core/Parents/States/State.hpp>
#include <Core/Util/Translator.hpp>
#include <Form/Controls/Controls.hpp>
#include <Model/Gen7/StaticModel7.hpp>
#include <QSettings>

static const QString settingPrefix = QStringLiteral("pelago7");

Pelago7::Pelago7(QWidget *parent) : QWidget(parent), ui(new Ui::Pelago7), currentProfile(nullptr)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::Gen7);

    model = new StaticModel7(ui->tableView);
    ui->tableView->setModel(model);

    ui->textBoxSeed->setValues(InputType::Seed32Bit);
    ui->textBoxInitialAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxMaxAdvances->setValues(InputType::Advance32Bit);

    ui->labelInitialAdvances->setToolTip(tr("Main RNG advance of the visitor's EC when Poké Pelago is entered. The entry first builds "
                                            "the boxed residents (10 draws each); the visitors are the last blocks, 12 draws each"));
    ui->labelPokemon->setToolTip(tr("Which species visits comes from the Pelago TinyMT in the save, not the main RNG"));

    for (u16 specie = 1; specie <= 807; specie++)
    {
        ui->comboBoxSpecie->addItem(QString::fromStdString(Translator::getSpecie(specie)), specie);
    }

    ui->filter->disableControls(Controls::Height | Controls::Weight | Controls::Wild);

    connect(ui->profileDisplay, &ProfileDisplay7::profileChanged, this, &Pelago7::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay7::profilesChanged, this, &Pelago7::profilesChanged);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Pelago7::generate);
    connect(ui->filter, &Filter::showStatsChanged, model, &StaticModel7::setShowStats);

    updateProfiles();

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Pelago7::~Pelago7()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void Pelago7::updateProfiles()
{
    ui->profileDisplay->updateProfiles();
}

void Pelago7::generate()
{
    if (!currentProfile || !ui->filter->isValid())
    {
        return;
    }

    model->clearModel();

    u32 seed = ui->textBoxSeed->getUInt();
    u32 initialAdvances = ui->textBoxInitialAdvances->getUInt();
    u32 maxAdvances = ui->textBoxMaxAdvances->getUInt();

    auto filter = ui->filter->getFilter<StateFilter>();
    PelagoGenerator7 generator(initialAdvances, maxAdvances, ui->comboBoxSpecie->getCurrentUShort(),
                               static_cast<u8>(ui->spinBoxLevel->value()), *currentProfile, filter);
    model->addItems(generator.generate(seed));
}

void Pelago7::profileChanged(const Profile7 &profile)
{
    currentProfile = &profile;
}
