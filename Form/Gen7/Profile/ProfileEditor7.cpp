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

#include "ProfileEditor7.hpp"
#include "ui_ProfileEditor7.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen7/Profile7.hpp>
#include <QMessageBox>
#include <QSettings>

ProfileEditor7::ProfileEditor7(QWidget *parent) : QDialog(parent), ui(new Ui::ProfileEditor7)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->textBoxTID->setValues(InputType::TIDSID);
    ui->textBoxSID->setValues(InputType::TIDSID);
    ui->comboBoxVersion->setup({ toInt(Game::Sun), toInt(Game::Moon), toInt(Game::US), toInt(Game::UM) });

    connect(ui->pushButtonOkay, &QPushButton::clicked, this, &ProfileEditor7::okay);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &ProfileEditor7::reject);

    QSettings setting;
    if (setting.contains("profileEditor7/geometry"))
    {
        this->restoreGeometry(setting.value("profileEditor7/geometry").toByteArray());
    }
}

ProfileEditor7::ProfileEditor7(const Profile7 &profile, QWidget *parent) : ProfileEditor7(parent)
{
    ui->lineEditProfile->setText(QString::fromStdString(profile.getName()));
    ui->comboBoxVersion->setCurrentIndex(ui->comboBoxVersion->findData(toInt(profile.getVersion())));
    ui->textBoxTID->setText(QString::number(profile.getTID()));
    ui->textBoxSID->setText(QString::number(profile.getSID()));
    ui->checkBoxShinyCharm->setChecked(profile.getShinyCharm());
    ui->checkBoxOvalCharm->setChecked(profile.getOvalCharm());
}

ProfileEditor7::~ProfileEditor7()
{
    QSettings setting;
    setting.setValue("profileEditor7/geometry", this->saveGeometry());

    delete ui;
}

Profile7 ProfileEditor7::getProfile()
{
    return Profile7(ui->lineEditProfile->text().toStdString(), ui->comboBoxVersion->getEnum<Game>(), ui->textBoxTID->getUShort(),
                    ui->textBoxSID->getUShort(), ui->checkBoxShinyCharm->isChecked(), ui->checkBoxOvalCharm->isChecked());
}

void ProfileEditor7::okay()
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
