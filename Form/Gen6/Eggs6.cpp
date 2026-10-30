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

#include "Eggs6.hpp"
#include "ui_Eggs6.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen6/Generators/EggGenerator6.hpp>
#include <Core/Gen6/Profile6.hpp>
#include <Core/Gen6/Save6.hpp>
#include <Core/Gen6/States/EggState6.hpp>
#include <Form/Controls/Controls.hpp>
#include <Model/Gen6/StateModel6.hpp>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

static const QString settingPrefix = QStringLiteral("egg6");

Eggs6::Eggs6(QWidget *parent) : QWidget(parent), ui(new Ui::Eggs6)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::Gen6);

    model = new EggModel6(ui->tableView);
    ui->tableView->setModel(model);

    ui->textBoxEggSeed->setValues(InputType::Seed64Bit);
    ui->textBoxSeed->setValues(InputType::Seed32Bit);
    ui->textBoxInitialAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxMaxAdvances->setValues(InputType::Advance32Bit);

    ui->labelEggSeed->setToolTip(tr("64bit daycare seed (save Daycare + 0x1E8), high word first as PKHeX shows it"));
    ui->labelMainMtSeed->setToolTip(tr("Accepting or rejecting an egg at MT frame N seeds the next egg with outputs N (low) and N + 1 (high)"));

    ui->filter->disableControls(Controls::Height | Controls::Weight | Controls::Wild);
    ui->filter->enableHiddenAbility();

    ui->eggSettings->setup(Game::XY);

    connect(ui->profileDisplay, &ProfileDisplay6::profileChanged, this, &Eggs6::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay6::profilesChanged, this, &Eggs6::profilesChanged);
    connect(ui->pushButtonCurrent, &QPushButton::clicked, this, &Eggs6::current);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Eggs6::generate);
    connect(ui->pushButtonLoadSave, &QPushButton::clicked, this, &Eggs6::loadSave);
    connect(ui->eggSettings, &EggSettings::showInheritanceChanged, model, &EggModel6::setShowInheritance);
    connect(ui->filter, &Filter::showStatsChanged, model, &EggModel6::setShowStats);

    updateProfiles();

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Eggs6::~Eggs6()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void Eggs6::updateProfiles()
{
    ui->profileDisplay->updateProfiles();
}

bool Eggs6::validate()
{
    bool hiddenAbility = !ui->filter->getDisableFilters() && ui->filter->getAbility() == 2;
    if (!ui->eggSettings->isValid(hiddenAbility))
    {
        return false;
    }
    if (ui->eggSettings->reorderParents())
    {
        QMessageBox box(QMessageBox::Information, tr("Parents Reordered"), tr("Parent were swapped to match the game"));
        box.exec();
    }
    return ui->filter->isValid();
}

void Eggs6::current()
{
    if (!validate())
    {
        return;
    }

    model->clearModel();

    Daycare daycare = ui->eggSettings->getDaycare(false);
    EggGenerator6 generator(0, 0, daycare, *currentProfile, ui->filter->getFilter<StateFilter>());
    model->addItem(generator.generate(ui->textBoxEggSeed->getULong()));
}

void Eggs6::generate()
{
    if (!validate())
    {
        return;
    }

    model->clearModel();

    u32 seed = ui->textBoxSeed->getUInt();
    u32 initialAdvances = ui->textBoxInitialAdvances->getUInt();
    u32 maxAdvances = ui->textBoxMaxAdvances->getUInt();
    Daycare daycare = ui->eggSettings->getDaycare(false);

    EggGenerator6 generator(initialAdvances, maxAdvances, daycare, *currentProfile, ui->filter->getFilter<StateFilter>());
    model->addItems(generator.generateChain(seed));
}

void Eggs6::loadSave()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select an X/Y/OR/AS save (main)"));
    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }
    QByteArray data = file.readAll();

    Save6 save = Save6::parse(reinterpret_cast<const u8 *>(data.constData()), static_cast<size_t>(data.size()));
    if (!save.valid)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid save"), tr("Select a decrypted X/Y save (0x65600 bytes) or OR/AS save (0x76000 bytes)"));
        msg.exec();
        return;
    }

    u64 eggSeed = save.eggSeed;
    bool eggReady = save.eggReady;
    if (save.resortUsed)
    {
        // OR/AS have a second daycare at the Battle Resort with its own egg seed
        QMessageBox ask(QMessageBox::Question, tr("Which daycare?"),
                        tr("This save uses both daycares. Load the egg seed from which one?"));
        QPushButton *route117 = ask.addButton(tr("Route 117"), QMessageBox::AcceptRole);
        QPushButton *resort = ask.addButton(tr("Battle Resort"), QMessageBox::AcceptRole);
        ask.setDefaultButton(save.eggReadyResort && !save.eggReady ? resort : route117);
        ask.exec();
        if (ask.clickedButton() == resort)
        {
            eggSeed = save.eggSeedResort;
            eggReady = save.eggReadyResort;
        }
    }

    ui->textBoxEggSeed->setText(QString::number(eggSeed, 16).toUpper().rightJustified(16, '0'));
    if (!eggReady)
    {
        QMessageBox msg(QMessageBox::Information, tr("No egg waiting"),
                        tr("The daycare has no egg waiting; the seed is for the next egg it makes"));
        msg.exec();
    }
}

void Eggs6::profileChanged(const Profile6 &profile)
{
    currentProfile = &profile;
}
