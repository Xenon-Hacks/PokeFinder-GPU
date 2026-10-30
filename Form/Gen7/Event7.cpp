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

#include "Event7.hpp"
#include "ui_Event7.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen7/Generators/EventGenerator7.hpp>
#include <Core/Gen7/Profile7.hpp>
#include <Core/Gen7/WC7.hpp>
#include <Core/Parents/States/State.hpp>
#include <Core/Util/Translator.hpp>
#include <Form/Controls/Controls.hpp>
#include <Model/Gen7/StaticModel7.hpp>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

static const QString settingPrefix = QStringLiteral("event7");

Event7::Event7(QWidget *parent) : QWidget(parent), ui(new Ui::Event7), currentProfile(nullptr)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::Gen7);

    model = new StaticModel7(ui->tableView);
    ui->tableView->setModel(model);

    ui->textBoxSeed->setValues(InputType::Seed32Bit);
    ui->textBoxInitialAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxMaxAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxPID->setValues(InputType::Seed32Bit);
    ui->textBoxEC->setValues(InputType::Seed32Bit);
    ui->textBoxTID->setValues(InputType::TIDSID);
    ui->textBoxSID->setValues(InputType::TIDSID);

    ui->labelInitialAdvances->setToolTip(tr("Main RNG advance of the first discarded block (the deliveryman's \"Here you are!\"); "
                                            "the kept Pokémon is the last block, built right after the final Yes"));
    ui->labelBlocks->setToolTip(tr("Identical blocks built and discarded before the kept one: 1 for Raikou's card, 2 for Munchlax "
                                   "and the shiny Magikarp card. Each costs its own draws"));
    ui->labelEc->setToolTip(tr("Fixed EC, 0 for random"));
    ui->lineEditIVs->setToolTip(tr("HP/Atk/Def/SpA/SpD/Spe, - for random"));
    ui->labelLevel->setToolTip(tr("0 draws rand(100) + 1 first (no released card uses it)"));

    for (u16 specie = 1; specie <= 807; specie++)
    {
        ui->comboBoxSpecie->addItem(QString::fromStdString(Translator::getSpecie(specie)), specie);
    }

    ui->comboBoxPIDType->setup({ static_cast<u8>(Gen7::PIDType7::Random), static_cast<u8>(Gen7::PIDType7::Locked),
                                 static_cast<u8>(Gen7::PIDType7::Shiny), static_cast<u8>(Gen7::PIDType7::Fixed) });
    ui->comboBoxAbility->setup({ 0, 1, 2, 3, 4 });
    ui->comboBoxGender->setup({ 0, 1, 2, 3 });
    ui->comboBoxGender->setCurrentIndex(3);

    ui->comboBoxNature->addItem(tr("Random"), Gen7::Random);
    const auto &natures = Translator::getNatures();
    for (size_t i = 0; i < natures.size(); i++)
    {
        ui->comboBoxNature->addItem(QString::fromStdString(natures[i]), static_cast<int>(i));
    }

    ui->checkBoxOwnOT->setChecked(true);

    ui->filter->disableControls(Controls::Height | Controls::Weight | Controls::Wild);
    ui->filter->enableHiddenAbility();

    connect(ui->profileDisplay, &ProfileDisplay7::profileChanged, this, &Event7::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay7::profilesChanged, this, &Event7::profilesChanged);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Event7::generate);
    connect(ui->pushButtonImport, &QPushButton::clicked, this, &Event7::importWC7);
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

Event7::~Event7()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void Event7::updateProfiles()
{
    ui->profileDisplay->updateProfiles();
}

WC7 Event7::getWC7(bool &valid) const
{
    std::array<u8, 6> ivs;
    ivs.fill(Gen7::Random);

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

    return WC7(ui->comboBoxSpecie->getCurrentUShort(), static_cast<u8>(ui->spinBoxForm->value()), static_cast<u8>(ui->spinBoxLevel->value()),
               ui->textBoxTID->getUShort(), ui->textBoxSID->getUShort(), ui->checkBoxOwnOT->isChecked(), ui->textBoxEC->getUInt(),
               ui->comboBoxPIDType->getEnum<Gen7::PIDType7>(), ui->textBoxPID->getUInt(), ivs, static_cast<u8>(ui->spinBoxIVCount->value()),
               ui->comboBoxAbility->getCurrentUChar(), ui->comboBoxNature->getCurrentUChar(), ui->comboBoxGender->getCurrentUChar(), false);
}

void Event7::generate()
{
    if (!currentProfile || !ui->filter->isValid())
    {
        return;
    }

    bool valid;
    WC7 wc7 = getWC7(valid);
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

    auto filter = ui->filter->getFilter<StateFilter>();
    EventGenerator7 generator(initialAdvances, maxAdvances, static_cast<u8>(ui->spinBoxBlocks->value()), wc7, *currentProfile, filter);
    model->addItems(generator.generate(seed));
}

void Event7::importWC7()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a WC7 file"), QDir::currentPath(), tr("Wonder Card (*.wc7 *.wc6)"));
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
    if (data.size() < static_cast<qsizetype>(WC7::size))
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid file"), tr("A .wc7 file is 264 bytes"));
        msg.exec();
        return;
    }

    WC7 wc7 = WC7::parse(reinterpret_cast<const u8 *>(data.constData()));
    if (wc7.getEgg())
    {
        QMessageBox msg(QMessageBox::Information, tr("Egg gift"), tr("This card is an egg; its traits are only set when it hatches"));
        msg.exec();
    }

    ui->comboBoxSpecie->setCurrentIndex(ui->comboBoxSpecie->findData(wc7.getSpecie()));
    ui->spinBoxForm->setValue(wc7.getForm());
    ui->spinBoxLevel->setValue(wc7.getLevel());
    ui->comboBoxPIDType->setCurrentIndex(ui->comboBoxPIDType->findData(static_cast<u8>(wc7.getPIDType())));
    ui->textBoxPID->setText(QString::number(wc7.getPID(), 16).toUpper());
    ui->textBoxEC->setText(QString::number(wc7.getEC(), 16).toUpper());
    ui->comboBoxAbility->setCurrentIndex(ui->comboBoxAbility->findData(wc7.getAbility()));
    ui->comboBoxNature->setCurrentIndex(ui->comboBoxNature->findData(wc7.getNature()));
    ui->comboBoxGender->setCurrentIndex(ui->comboBoxGender->findData(wc7.getGender()));
    ui->spinBoxIVCount->setValue(wc7.getIVCount());

    QStringList ivs;
    for (u8 i = 0; i < 6; i++)
    {
        ivs.append(wc7.getIV(i) == Gen7::Random ? QString("-") : QString::number(wc7.getIV(i)));
    }
    ui->lineEditIVs->setText(ivs.join('/'));

    ui->checkBoxOwnOT->setChecked(wc7.getOwnOT());
    ui->textBoxTID->setText(QString::number(wc7.getTID()));
    ui->textBoxSID->setText(QString::number(wc7.getSID()));
}

void Event7::profileChanged(const Profile7 &profile)
{
    currentProfile = &profile;
}
