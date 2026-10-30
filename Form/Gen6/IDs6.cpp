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

#include "IDs6.hpp"
#include "ui_IDs6.h"
#include <Core/Gen6/Generators/IDGenerator6.hpp>
#include <Core/Parents/Filters/IDFilter.hpp>
#include <Core/Parents/States/IDState.hpp>
#include <Model/Gen6/StateModel6.hpp>
#include <QSettings>

static const QString settingPrefix = QStringLiteral("ids6");

IDs6::IDs6(QWidget *parent) : QWidget(parent), ui(new Ui::IDs6)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    model = new IDModel6(ui->tableView);
    ui->tableView->setModel(model);

    ui->textBoxState0->setValues(InputType::Seed32Bit);
    ui->textBoxState1->setValues(InputType::Seed32Bit);
    ui->textBoxState2->setValues(InputType::Seed32Bit);
    ui->textBoxState3->setValues(InputType::Seed32Bit);
    ui->textBoxInitialAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxMaxAdvances->setValues(InputType::Advance32Bit);

    ui->comboBoxSource->setToolTip(tr("The TinyMT state is the 4 words at RAM 0x8C52808; the seed is the console clock in ms at boot"));
    ui->labelInitialAdvances->setToolTip(tr("TinyMT advances; the IDs are the next TinyMT output after the name is confirmed"));

    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &IDs6::generate);
    connect(ui->comboBoxSource, &QComboBox::currentIndexChanged, this, &IDs6::sourceIndexChanged);

    sourceIndexChanged(0);

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

IDs6::~IDs6()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void IDs6::generate()
{
    model->clearModel();

    u32 initialAdvances = ui->textBoxInitialAdvances->getUInt();
    u32 maxAdvances = ui->textBoxMaxAdvances->getUInt();

    IDFilter filter = ui->idFilter->getFilter(true);
    IDGenerator6 generator(initialAdvances, maxAdvances, filter);

    std::vector<IDState> states;
    if (ui->comboBoxSource->currentIndex() == 0)
    {
        states = generator.generate(std::array<u32, 4> { ui->textBoxState0->getUInt(), ui->textBoxState1->getUInt(),
                                                         ui->textBoxState2->getUInt(), ui->textBoxState3->getUInt() });
    }
    else
    {
        states = generator.generate(ui->textBoxState0->getUInt());
    }
    model->addItems(states);
}

void IDs6::sourceIndexChanged(int index)
{
    bool state = index == 0;
    ui->labelState1->setVisible(state);
    ui->labelState2->setVisible(state);
    ui->labelState3->setVisible(state);
    ui->textBoxState1->setVisible(state);
    ui->textBoxState2->setVisible(state);
    ui->textBoxState3->setVisible(state);
    ui->labelStateSeed0->setText(state ? tr("State 0") : tr("Seed"));
}
