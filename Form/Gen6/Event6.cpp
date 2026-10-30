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

#include "Event6.hpp"
#include "ui_Event6.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen6/Generators/EventGenerator6.hpp>
#include <Core/Gen6/Profile6.hpp>
#include <Core/Gen6/States/State6.hpp>
#include <Core/Gen6/WC6.hpp>
#include <Core/Util/Translator.hpp>
#include <Form/Controls/Controls.hpp>
#include <Model/Gen6/StateModel6.hpp>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

static const QString settingPrefix = QStringLiteral("event6");

static u8 pidType(Gen6::PIDType type)
{
    return static_cast<u8>(type);
}

Event6::Event6(QWidget *parent) : QWidget(parent), ui(new Ui::Event6)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::XY);

    model = new StateModel6(ui->tableView);
    ui->tableView->setModel(model);
    ui->tableView->setColumnHidden(StateModel6::spotsColumn, true);

    ui->textBoxSeed->setValues(InputType::Seed32Bit);
    ui->textBoxInitialAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxMaxAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxDelay->setValues(InputType::Advance32Bit);
    ui->textBoxPID->setValues(InputType::Seed32Bit);
    ui->textBoxEC->setValues(InputType::Seed32Bit);
    ui->textBoxTID->setValues(InputType::TIDSID);
    ui->textBoxSID->setValues(InputType::TIDSID);

    ui->labelInitialAdvances->setToolTip(tr("MT frame of the EC draw (X/Y generate the gift once, with no 60-frame skip)"));
    ui->labelEc->setToolTip(tr("Fixed EC, 0 for random"));
    ui->lineEditIVs->setToolTip(tr("HP/Atk/Def/SpA/SpD/Spe, - for random"));

    for (u16 specie = 1; specie <= 721; specie++)
    {
        ui->comboBoxSpecie->addItem(QString::fromStdString(Translator::getSpecie(specie)), specie);
    }

    ui->comboBoxPIDType->setup({ pidType(Gen6::PIDType::Random), pidType(Gen6::PIDType::Locked), pidType(Gen6::PIDType::Shiny),
                                 pidType(Gen6::PIDType::Fixed) });
    ui->comboBoxAbility->setup({ 0, 1, 2, 3, 4 });
    ui->comboBoxGender->setup({ 0, 1, 2, 3 });
    ui->comboBoxGender->setCurrentIndex(3);

    ui->comboBoxNature->addItem(tr("Random"), Gen6::Random);
    const auto &natures = Translator::getNatures();
    for (size_t i = 0; i < natures.size(); i++)
    {
        ui->comboBoxNature->addItem(QString::fromStdString(natures[i]), static_cast<int>(i));
    }

    ui->checkBoxOwnOT->setChecked(true);

    ui->filter->disableControls(Controls::Height | Controls::Weight | Controls::Wild);
    ui->filter->enableHiddenAbility();

    connect(ui->profileDisplay, &ProfileDisplay6::profileChanged, this, &Event6::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay6::profilesChanged, this, &Event6::profilesChanged);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Event6::generate);
    connect(ui->pushButtonImport, &QPushButton::clicked, this, &Event6::importWC6);
    connect(ui->filter, &Filter::showStatsChanged, model, &StateModel6::setShowStats);

    updateProfiles();

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Event6::~Event6()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void Event6::updateProfiles()
{
    ui->profileDisplay->updateProfiles();
}

WC6 Event6::getWC6(bool &valid) const
{
    std::array<u8, 6> ivs;
    ivs.fill(Gen6::Random);

    QStringList parts = ui->lineEditIVs->text().split('/');
    valid = parts.size() == 6;
    for (int i = 0; valid && i < 6; i++)
    {
        QString part = parts[i].trimmed();
        if (part != "-" && !part.isEmpty())
        {
            bool ok;
            int iv = part.toInt(&ok);
            valid = ok && iv >= 0 && iv <= 31;
            ivs[i] = iv;
        }
    }

    return WC6(ui->comboBoxSpecie->getCurrentUShort(), 0, ui->spinBoxLevel->value(), ui->textBoxTID->getUShort(),
               ui->textBoxSID->getUShort(), ui->checkBoxOwnOT->isChecked(), ui->textBoxEC->getUInt(),
               ui->comboBoxPIDType->getEnum<Gen6::PIDType>(), ui->textBoxPID->getUInt(), ivs, ui->spinBoxIVCount->value(),
               ui->comboBoxAbility->getCurrentUChar(), ui->comboBoxNature->getCurrentUChar(), ui->comboBoxGender->getCurrentUChar(), false);
}

void Event6::generate()
{
    if (!ui->filter->isValid())
    {
        return;
    }

    bool valid;
    WC6 wc6 = getWC6(valid);
    if (!valid)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid IVs"), tr("Enter the fixed IVs as HP/Atk/Def/SpA/SpD/Spe, - for random"));
        msg.exec();
        return;
    }

    model->clearModel();

    u32 seed = ui->textBoxSeed->getUInt();
    u32 initialAdvances = ui->textBoxInitialAdvances->getUInt();
    u32 maxAdvances = ui->textBoxMaxAdvances->getUInt();
    u32 delay = ui->textBoxDelay->getUInt();

    auto filter = ui->filter->getFilter<StateFilter>();
    EventGenerator6 generator(initialAdvances, maxAdvances, delay, wc6, *currentProfile, filter, ui->checkBoxGenerateTwice->isChecked());

    auto states = generator.generate(seed);
    model->addItems(states);
}

void Event6::importWC6()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a WC6 file"), QDir::currentPath(), tr("Wonder Card (*.wc6)"));
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
    if (data.size() < static_cast<qsizetype>(WC6::size))
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid file"), tr("A .wc6 file is 264 bytes"));
        msg.exec();
        return;
    }

    WC6 wc6 = WC6::parse(reinterpret_cast<const u8 *>(data.constData()));
    if (wc6.getEgg())
    {
        QMessageBox msg(QMessageBox::Information, tr("Egg gift"), tr("This card is an egg; its traits are only set when it hatches"));
        msg.exec();
    }

    ui->comboBoxSpecie->setCurrentIndex(ui->comboBoxSpecie->findData(wc6.getSpecie()));
    ui->spinBoxLevel->setValue(wc6.getLevel());
    ui->comboBoxPIDType->setCurrentIndex(ui->comboBoxPIDType->findData(pidType(wc6.getPIDType())));
    ui->textBoxPID->setText(QString::number(wc6.getPID(), 16));
    ui->textBoxEC->setText(QString::number(wc6.getEC(), 16));
    ui->comboBoxAbility->setCurrentIndex(ui->comboBoxAbility->findData(wc6.getAbility()));
    ui->comboBoxNature->setCurrentIndex(ui->comboBoxNature->findData(wc6.getNature()));
    ui->comboBoxGender->setCurrentIndex(ui->comboBoxGender->findData(wc6.getGender()));
    ui->spinBoxIVCount->setValue(wc6.getIVCount());

    QStringList ivs;
    for (int i = 0; i < 6; i++)
    {
        ivs.append(wc6.getIV(i) == Gen6::Random ? QString("-") : QString::number(wc6.getIV(i)));
    }
    ui->lineEditIVs->setText(ivs.join('/'));

    ui->checkBoxOwnOT->setChecked(wc6.getOwnOT());
    ui->textBoxTID->setText(QString::number(wc6.getTID()));
    ui->textBoxSID->setText(QString::number(wc6.getSID()));
}

void Event6::profileChanged(const Profile6 &profile)
{
    currentProfile = &profile;
    bool oras = (profile.getVersion() & Game::ORAS) != Game::None;
    ui->checkBoxGenerateTwice->setEnabled(oras);
    ui->checkBoxGenerateTwice->setChecked(oras);
}
