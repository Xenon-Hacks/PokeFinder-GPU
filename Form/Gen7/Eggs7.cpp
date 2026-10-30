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

#include "Eggs7.hpp"
#include "ui_Eggs7.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen7/Generators/EggGenerator7.hpp>
#include <Core/Gen7/Profile7.hpp>
#include <Core/Gen7/Save7.hpp>
#include <Core/Gen7/States/EggState7.hpp>
#include <Form/Controls/Controls.hpp>
#include <Model/Gen7/EggModel7.hpp>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

static const QString settingPrefix = QStringLiteral("egg7");

Eggs7::Eggs7(QWidget *parent) : QWidget(parent), ui(new Ui::Eggs7), currentProfile(nullptr)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::Gen7);

    model = new EggModel7(ui->tableView);
    ui->tableView->setModel(model);

    ui->textBoxState0->setValues(InputType::Seed32Bit);
    ui->textBoxState1->setValues(InputType::Seed32Bit);
    ui->textBoxState2->setValues(InputType::Seed32Bit);
    ui->textBoxState3->setValues(InputType::Seed32Bit);
    ui->textBoxSeed->setValues(InputType::Seed32Bit);
    ui->textBoxInitialAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxMaxAdvances->setValues(InputType::Advance32Bit);

    ui->labelState->setToolTip(tr("Daycare TinyMT state (save Sodateya + 0x1DC), four words low to high as PKHeX shows the egg seed"));
    ui->labelRejects->setToolTip(tr("Eggs rejected before the first one: each costs one draw and generates nothing"));
    ui->checkBoxSameSpecies->setToolTip(tr("Both parents the same species: the ball roll rand(100) adds one draw after the PID"));
    ui->labelMainSeed->setToolTip(tr("Without Masuda or the Shiny Charm the PID is one main RNG draw when the egg is accepted; "
                                     "the advance is the accept press with the field blinks running underneath"));

    ui->filter->disableControls(Controls::Height | Controls::Weight | Controls::Wild);
    ui->filter->enableHiddenAbility();

    ui->eggSettings->setup(Game::Gen7);

    connect(ui->profileDisplay, &ProfileDisplay7::profileChanged, this, &Eggs7::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay7::profilesChanged, this, &Eggs7::profilesChanged);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Eggs7::generateChain);
    connect(ui->pushButtonMainPID, &QPushButton::clicked, this, &Eggs7::generateMainPID);
    connect(ui->pushButtonLoadSave, &QPushButton::clicked, this, &Eggs7::loadSave);
    connect(ui->eggSettings, &EggSettings::showInheritanceChanged, model, &EggModel7::setShowInheritance);
    connect(ui->filter, &Filter::showStatsChanged, model, &EggModel7::setShowStats);

    updateProfiles();

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Eggs7::~Eggs7()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void Eggs7::updateProfiles()
{
    ui->profileDisplay->updateProfiles();
}

std::array<u32, 4> Eggs7::getState() const
{
    return { ui->textBoxState0->getUInt(), ui->textBoxState1->getUInt(), ui->textBoxState2->getUInt(), ui->textBoxState3->getUInt() };
}

bool Eggs7::validate()
{
    if (!currentProfile)
    {
        return false;
    }

    bool hiddenAbility = !ui->filter->getDisableFilters() && ui->filter->getAbility() == 2;
    if (!ui->eggSettings->isValid(hiddenAbility))
    {
        return false;
    }

    if (ui->eggSettings->reorderParents())
    {
        QMessageBox box(QMessageBox::Information, tr("Parents Reordered"),
                        tr("Parents were swapped to match the game: A is the father role, B the mother role"));
        box.exec();
    }

    return ui->filter->isValid();
}

void Eggs7::generateChain()
{
    if (!validate())
    {
        return;
    }

    model->clearModel();

    Daycare daycare = ui->eggSettings->getDaycare(currentProfile->getOvalCharm());
    EggGenerator7 generator(daycare, ui->checkBoxSameSpecies->isChecked(), *currentProfile, ui->filter->getFilter<StateFilter>());
    model->addItems(generator.generateChain(getState(), ui->spinBoxRejects->value(), ui->spinBoxEggs->value()));
}

void Eggs7::generateMainPID()
{
    if (!validate())
    {
        return;
    }

    model->clearModel();

    Daycare daycare = ui->eggSettings->getDaycare(currentProfile->getOvalCharm());
    EggGenerator7 generator(daycare, ui->checkBoxSameSpecies->isChecked(), *currentProfile, ui->filter->getFilter<StateFilter>());
    if (generator.getPIDRolls() != 0)
    {
        QMessageBox msg(QMessageBox::Information, tr("PID from the egg TinyMT"),
                        tr("With Masuda or the Shiny Charm the PID comes from the egg TinyMT, not the main RNG; use Generate Eggs"));
        msg.exec();
        return;
    }

    // Walk the state past the rejected eggs first
    std::array<u32, 4> state = EggGenerator7::reject(getState(), ui->spinBoxRejects->value());
    model->addItems(generator.generateMainPID(state, ui->textBoxSeed->getUInt(), ui->textBoxInitialAdvances->getUInt(),
                                              ui->textBoxMaxAdvances->getUInt()));
}

void Eggs7::loadSave()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a Sun/Moon/Ultra Sun/Ultra Moon save (main)"));
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
    Save7 save = Save7::parse(reinterpret_cast<const u8 *>(data.constData()), static_cast<size_t>(data.size()));
    if (!save.valid)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid save"),
                        tr("Select a decrypted Sun/Moon save (0x6BE00 bytes) or Ultra Sun/Ultra Moon save (0x6CC00 bytes)"));
        msg.exec();
        return;
    }

    ui->textBoxState0->setText(QString::number(save.eggState[0], 16).toUpper().rightJustified(8, '0'));
    ui->textBoxState1->setText(QString::number(save.eggState[1], 16).toUpper().rightJustified(8, '0'));
    ui->textBoxState2->setText(QString::number(save.eggState[2], 16).toUpper().rightJustified(8, '0'));
    ui->textBoxState3->setText(QString::number(save.eggState[3], 16).toUpper().rightJustified(8, '0'));

    if (!save.eggReady)
    {
        QMessageBox msg(QMessageBox::Information, tr("No egg waiting"),
                        tr("The daycare has no egg waiting; the state is for the next egg it makes"));
        msg.exec();
    }
}

void Eggs7::profileChanged(const Profile7 &profile)
{
    currentProfile = &profile;
}
