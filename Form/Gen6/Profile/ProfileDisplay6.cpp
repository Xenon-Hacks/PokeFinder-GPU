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

#include "ProfileDisplay6.hpp"
#include "ui_ProfileDisplay6.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen6/Profile6.hpp>
#include <Core/Parents/ProfileLoader.hpp>
#include <Core/Util/Translator.hpp>
#include <Form/Gen6/Profile/ProfileManager6.hpp>
#include <QSettings>

static const QString settingKey = QStringLiteral("%1/profile");

ProfileDisplay6::ProfileDisplay6(QWidget *parent) : QWidget(parent), ui(new Ui::ProfileDisplay6)
{
    ui->setupUi(this);

    connect(ui->comboBoxProfiles, &QComboBox::currentIndexChanged, this, &ProfileDisplay6::profileIndexChanged);
    connect(ui->pushButtonProfileManager, &QPushButton::clicked, this, &ProfileDisplay6::profileManager);
}

ProfileDisplay6::~ProfileDisplay6()
{
    QSettings setting;
    setting.setValue(settingKey.arg(prefix), ui->comboBoxProfiles->currentIndex());

    delete ui;
}

void ProfileDisplay6::setup(const QString &prefix, Game filter)
{
    this->prefix = prefix;
    this->filter = filter;
}

void ProfileDisplay6::updateProfiles()
{
    Game version = (filter & Game::X) != Game::None ? Game::X : Game::Y;

    profiles = ProfileLoader6::getProfiles(filter);
    profiles.insert(profiles.begin(), Profile6("-", version, 12345, 54321, false, 0));

    ui->comboBoxProfiles->clear();
    for (const auto &profile : profiles)
    {
        ui->comboBoxProfiles->addItem(QString::fromStdString(profile.getName()));
    }

    QSettings setting;
    int val = setting.value(settingKey.arg(prefix), 0).toInt();
    if (val < ui->comboBoxProfiles->count())
    {
        ui->comboBoxProfiles->setCurrentIndex(val);
    }
}

void ProfileDisplay6::profileIndexChanged(int index)
{
    if (index >= 0)
    {
        const Profile6 &profile = profiles[index];

        ui->labelTIDValue->setText(QString::number(profile.getTID()));
        ui->labelSIDValue->setText(QString::number(profile.getSID()));
        ui->labelGameValue->setText(QString::fromStdString(Translator::getGame(profile.getVersion())));
        ui->labelSaveSeedValue->setText(QString::number(profile.getSaveSeed(), 16).toUpper().rightJustified(8, QChar('0')));
        ui->labelShinyCharmValue->setText(profile.getShinyCharm() ? tr("Yes") : tr("No"));

        emit profileChanged(profile);
    }
}

void ProfileDisplay6::profileManager()
{
    auto *manager = new ProfileManager6();
    connect(manager, &ProfileManager6::profilesChanged, this, [this](int num) {
        updateProfiles();
        emit profilesChanged(num);
    });
    manager->show();
}
