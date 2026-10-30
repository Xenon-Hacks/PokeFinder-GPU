/*
 * This file is part of PokéFinder
 * Copyright (C) 2017-2024 by Admiral_Fish, bumba, and EzPzStreamz
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

#include "ProfileEditor6.hpp"
#include "ui_ProfileEditor6.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen6/Profile6.hpp>
#include <Core/Gen6/Save6.hpp>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

ProfileEditor6::ProfileEditor6(QWidget *parent) : QDialog(parent), ui(new Ui::ProfileEditor6)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    ui->textBoxTID->setValues(InputType::TIDSID);
    ui->textBoxSID->setValues(InputType::TIDSID);

    ui->textBoxSaveSeed->setValues(InputType::Seed32Bit);
    ui->labelSaveSeed->setToolTip(tr("u32 at save offset 0x4200. The game adds the console clock in ms to it after the title-screen A press "
                                     "(86 frames later on X/Y, 236 on Omega Ruby, 237 on Alpha Sapphire). It changes on every in-game save, "
                                     "so reload it from the save you will play from."));

    ui->comboBoxVersion->setup({ toInt(Game::X), toInt(Game::Y), toInt(Game::OR), toInt(Game::AS) });

    connect(ui->pushButtonLoadSave, &QPushButton::clicked, this, &ProfileEditor6::loadSave);
    connect(ui->pushButtonOkay, &QPushButton::clicked, this, &ProfileEditor6::okay);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &ProfileEditor6::reject);

    QSettings setting;
    if (setting.contains("profileEditor6/geometry"))
    {
        this->restoreGeometry(setting.value("profileEditor6/geometry").toByteArray());
    }
}

ProfileEditor6::ProfileEditor6(const Profile6 &profile, QWidget *parent) : ProfileEditor6(parent)
{
    ui->lineEditProfile->setText(QString::fromStdString(profile.getName()));
    ui->comboBoxVersion->setCurrentIndex(ui->comboBoxVersion->findData(toInt(profile.getVersion())));
    ui->textBoxTID->setText(QString::number(profile.getTID()));
    ui->textBoxSID->setText(QString::number(profile.getSID()));
    ui->textBoxSaveSeed->setText(QString::number(profile.getSaveSeed(), 16));
    ui->checkBoxShinyCharm->setChecked(profile.getShinyCharm());
}

ProfileEditor6::~ProfileEditor6()
{
    QSettings setting;
    setting.setValue("profileEditor6/geometry", this->saveGeometry());

    delete ui;
}

Profile6 ProfileEditor6::getProfile()
{
    return Profile6(ui->lineEditProfile->text().toStdString(), ui->comboBoxVersion->getEnum<Game>(), ui->textBoxTID->getUShort(),
                    ui->textBoxSID->getUShort(), ui->checkBoxShinyCharm->isChecked(), ui->textBoxSaveSeed->getUInt());
}

void ProfileEditor6::okay()
{
    QString input = ui->lineEditProfile->text().trimmed();
    if (input.isEmpty())
    {
        QMessageBox msg(QMessageBox::Warning, tr("Missing name"), tr("Enter a profile name"));
        msg.exec();
        return;
    }

    done(QDialog::Accepted);
}

void ProfileEditor6::loadSave()
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

    ui->comboBoxVersion->setCurrentIndex(ui->comboBoxVersion->findData(toInt(save.version)));
    ui->textBoxTID->setText(QString::number(save.tid));
    ui->textBoxSID->setText(QString::number(save.sid));
    ui->checkBoxShinyCharm->setChecked(save.shinyCharm);
    ui->textBoxSaveSeed->setText(QString::number(save.saveSeed, 16));

    QMessageBox msg(QMessageBox::Information, tr("Save seed loaded"),
                    tr("Save seed %1. It changes on every in-game save, so load the save again after you save in the game.")
                        .arg(QString::number(save.saveSeed, 16).toUpper().rightJustified(8, '0')));
    msg.exec();
}
