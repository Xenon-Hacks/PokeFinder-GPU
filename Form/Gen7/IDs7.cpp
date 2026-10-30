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

#include "IDs7.hpp"
#include "ui_IDs7.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen7/Gen7.hpp>
#include <Core/Gen7/Generators/IDGenerator7.hpp>
#include <Core/Parents/Filters/IDFilter.hpp>
#include <Model/Gen8/IDModel8.hpp>
#include <QSettings>

static const QString settingPrefix = QStringLiteral("ids7");

IDs7::IDs7(QWidget *parent) : QWidget(parent), ui(new Ui::IDs7)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    model = new IDModel8(ui->tableView);
    ui->tableView->setModel(model);

    ui->textBoxSeed->setValues(InputType::Seed32Bit);
    ui->textBoxInitialAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxMaxAdvances->setValues(InputType::Advance32Bit);
    ui->comboBoxGame->setup({ toInt(Game::Sun), toInt(Game::Moon), toInt(Game::US), toInt(Game::UM) });

    ui->labelSeed->setToolTip(tr("Boot seed of the main RNG, found from the Continue clock needles (Seed Finder)"));
    ui->labelInitialAdvances->setToolTip(tr("Main-RNG advance of the ID draw when the player name is confirmed: 1012 on Sun/Moon (live, "
                                            "2 runs). Ultra Sun/Ultra Moon use 3DSRNGTool's 1132, which is not verified. The intro "
                                            "makes no other draws, so the ID depends only on the boot seed"));

    connect(ui->comboBoxGame, &QComboBox::currentIndexChanged, this, &IDs7::gameIndexChanged);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &IDs7::generate);

    gameIndexChanged(0);

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

IDs7::~IDs7()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void IDs7::gameIndexChanged(int index)
{
    if (index >= 0)
    {
        Game version = ui->comboBoxGame->getEnum<Game>();
        ui->textBoxInitialAdvances->setText(QString::number(Gen7::idAdvance(version)));
        ui->labelVerified->setText(Gen7::idAdvanceVerified(version) ? tr("ID advance measured live") : tr("ID advance unverified (3DSRNGTool)"));
    }
}

void IDs7::generate()
{
    model->clearModel();

    u32 seed = ui->textBoxSeed->getUInt();
    u32 initialAdvances = ui->textBoxInitialAdvances->getUInt();
    u32 maxAdvances = ui->textBoxMaxAdvances->getUInt();
    IDFilter filter = ui->idFilter->getFilter(false);

    IDGenerator7 generator(initialAdvances, maxAdvances, filter);
    auto states = generator.generate(seed);
    model->addItems(states);
}
