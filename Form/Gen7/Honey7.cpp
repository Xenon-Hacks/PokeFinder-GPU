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

#include "Honey7.hpp"
#include "ui_Honey7.h"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Gen7/EncounterArea7.hpp>
#include <Core/Gen7/Gen7.hpp>
#include <Core/Gen7/Generators/HoneyGenerator7.hpp>
#include <Core/Gen7/Profile7.hpp>
#include <Core/Gen7/Spot7.hpp>
#include <Core/Util/Translator.hpp>
#include <Form/Controls/Controls.hpp>
#include <Model/Gen7/HoneyModel7.hpp>
#include <QMessageBox>
#include <QSettings>
#include <cmath>

static const QString settingPrefix = QStringLiteral("honey7");

Honey7::Honey7(QWidget *parent) : QWidget(parent), ui(new Ui::Honey7), currentProfile(nullptr)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::Gen7);

    model = new HoneyModel7(ui->tableView);
    ui->tableView->setModel(model);

    ui->textBoxSeed->setValues(InputType::Seed32Bit);
    ui->spinBoxNeedles->setRange(0, 40);
    ui->spinBoxNeedles->setValue(8);

    ui->comboMenuLead->addAction(tr("None"), toInt(Lead::None));
    ui->comboMenuLead->addMenu(tr("Cute Charm"), { { tr("♂ Lead"), toInt(Lead::CuteCharmM) }, { tr("♀ Lead"), toInt(Lead::CuteCharmF) } });
    ui->comboMenuLead->addMenu(tr("Synchronize"), Translator::getNatures());
    ui->comboMenuLead->addAction(tr("Compound Eyes"), toInt(Lead::CompoundEyes));
    ui->comboMenuLead->addAction(tr("Pressure / Hustle / Vital Spirit"), toInt(Lead::Pressure));

    ui->filter->disableControls(Controls::Height | Controls::Weight | Controls::Wild);

    ui->labelNeedles->setToolTip(tr("Needles read on the title screen: the RNG sits at the title advance + needles when Continue is pressed"));
    ui->labelMenu->setToolTip(tr("Seconds after Continue the X menu opened. The fidget timer stops while the menu is open; open it before the "
                                 "second fidget (about 13 s) and at least 3 s before the bag press"));
    ui->labelTimeOfDay->setToolTip(tr("Game hour: Moon = console hour + 12, Sun = console hour (after the Route 1 lab). Day 06:00-17:59"));
    ui->groupBoxTimer->setToolTip(tr("Press A on BAG at (frame - offset) / fps seconds after the A press on Continue. Hardware offsets "
                                     "(New 3DS, Moon 1.2, 268 MHz) exist for 9 spots; others are estimates. One timed press calibrates "
                                     "the offset: enter the seconds you pressed at and the frame you hit (from the Pokemon you got)"));

    connect(ui->profileDisplay, &ProfileDisplay7::profileChanged, this, &Honey7::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay7::profilesChanged, this, &Honey7::profilesChanged);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Honey7::generate);
    connect(ui->pushButtonCalibrate, &QPushButton::clicked, this, &Honey7::calibrate);
    connect(ui->comboBoxSpot, &QComboBox::currentIndexChanged, this, &Honey7::spotIndexChanged);
    connect(ui->comboBoxTimerSource, &QComboBox::currentIndexChanged, this, &Honey7::timerSourceChanged);
    connect(ui->doubleSpinBoxOffset, &QDoubleSpinBox::valueChanged, this, &Honey7::timerChanged);
    connect(ui->doubleSpinBoxFps, &QDoubleSpinBox::valueChanged, this, &Honey7::timerChanged);
    connect(ui->filter, &Filter::showStatsChanged, model, &HoneyModel7::setShowStats);

    updateProfiles();

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Honey7::~Honey7()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void Honey7::updateProfiles()
{
    ui->profileDisplay->updateProfiles();
}

const Spot7 *Honey7::currentSpot() const
{
    int index = ui->comboBoxSpot->currentIndex();
    return index >= 0 && index < static_cast<int>(spots.size()) ? spots[index] : nullptr;
}

Timer7 Honey7::timer() const
{
    return Timer7 { static_cast<float>(ui->doubleSpinBoxOffset->value()), static_cast<float>(ui->doubleSpinBoxFps->value()) };
}

void Honey7::profileChanged(const Profile7 &profile)
{
    currentProfile = &profile;
    Game version = profile.getVersion();

    ui->labelTitleAdvance->setText(tr("Title advance %1").arg(Gen7::titleAdvance(version)));

    spots = Spots7::getSpots(version);
    int current = ui->comboBoxSpot->currentIndex();
    ui->comboBoxSpot->clear();
    for (const Spot7 *spot : spots)
    {
        QString name = QString("%1 (zone %2/%3)").arg(QString::fromUtf8(spot->location)).arg(spot->zone).arg(spot->table);
        if (spot->flags & Spot7::Unsafe)
        {
            name += tr(" [calibrate per attempt]");
        }
        ui->comboBoxSpot->addItem(name);
    }
    ui->comboBoxSpot->setCurrentIndex(current >= 0 && current < ui->comboBoxSpot->count() ? current : 0);
    spotIndexChanged(ui->comboBoxSpot->currentIndex());
}

void Honey7::spotIndexChanged(int index)
{
    const Spot7 *spot = currentSpot();
    if (!spot)
    {
        return;
    }

    ui->checkBoxRain->setChecked(spot->flags & Spot7::Rain);
    ui->labelSpotInfo->setText(tr("Tile %1 %2 %3. Continue: %4 models, Honey: %5 models, pre %6, delay %7 frames. %8")
                                   .arg(spot->tile[0])
                                   .arg(spot->tile[1])
                                   .arg(spot->tile[2])
                                   .arg(spot->continueModels)
                                   .arg(spot->honeyModels)
                                   .arg(spot->honeyPreDraws)
                                   .arg(spot->honeyDelay)
                                   .arg(QString::fromUtf8(spot->note)));

    // Restore a calibrated offset for this spot, else the table value
    QSettings setting;
    QString key = QString("%1/offset/%2/%3/%4").arg(settingPrefix).arg(toInt(currentProfile->getVersion())).arg(spot->zone).arg(spot->table);
    if (setting.contains(key))
    {
        ui->comboBoxTimerSource->setCurrentIndex(2);
        ui->doubleSpinBoxOffset->setValue(setting.value(key).toDouble());
        ui->doubleSpinBoxFps->setValue(setting.value(key + "/fps", spot->hardwareTimer ? spot->hardwareFps : 29.913).toDouble());
    }
    else
    {
        timerSourceChanged();
    }
}

void Honey7::timerSourceChanged()
{
    const Spot7 *spot = currentSpot();
    if (!spot)
    {
        return;
    }

    int source = ui->comboBoxTimerSource->currentIndex();
    if (source == 0)
    {
        ui->doubleSpinBoxOffset->setValue(spot->hardwareOffsetOrEstimate());
        ui->doubleSpinBoxFps->setValue(spot->hardwareTimer ? spot->hardwareFps : 29.913);
        ui->labelTimerStatus->setText(spot->hardwareTimer ? tr("Measured on a New 3DS at this spot") : tr("Estimate (Azahar - 43 frames): calibrate"));
    }
    else if (source == 1)
    {
        ui->doubleSpinBoxOffset->setValue(spot->azaharOffsetOrEstimate());
        ui->doubleSpinBoxFps->setValue(29.91);
        ui->labelTimerStatus->setText(spot->azaharTimer ? tr("Measured in Azahar at this spot") : tr("Estimate (-61.5 - 0.6 L): calibrate"));
    }
    else
    {
        ui->labelTimerStatus->setText(tr("Calibrated"));
    }
    timerChanged();
}

void Honey7::timerChanged()
{
    model->setTimer(timer());
}

void Honey7::calibrate()
{
    const Spot7 *spot = currentSpot();
    if (!spot || !currentProfile)
    {
        return;
    }

    double seconds = ui->doubleSpinBoxCalibrateSeconds->value();
    u32 hit = static_cast<u32>(ui->spinBoxCalibrateFrame->value());
    float offset = timer().calibrate(seconds, hit);

    ui->comboBoxTimerSource->setCurrentIndex(2);
    ui->doubleSpinBoxOffset->setValue(offset);

    QSettings setting;
    QString key = QString("%1/offset/%2/%3/%4").arg(settingPrefix).arg(toInt(currentProfile->getVersion())).arg(spot->zone).arg(spot->table);
    setting.setValue(key, offset);
    setting.setValue(key + "/fps", ui->doubleSpinBoxFps->value());
    ui->labelTimerStatus->setText(tr("Calibrated: offset %1 frames").arg(offset, 0, 'f', 1));
}

void Honey7::generate()
{
    const Spot7 *spot = currentSpot();
    if (!spot || !currentProfile || !ui->filter->isValid())
    {
        return;
    }

    Game version = currentProfile->getVersion();
    bool night = ui->comboBoxTimeOfDay->currentIndex() == 1;
    auto areas = Encounters7::getEncounters(version, spot->zone, spot->table, night);
    if (areas.empty())
    {
        QMessageBox msg(QMessageBox::Warning, tr("No encounter table"), tr("No encounter table is embedded for this zone and tile"));
        msg.exec();
        return;
    }

    model->clearModel();

    Timer7 t = timer();
    u32 seed = ui->textBoxSeed->getUInt();
    u32 continueAdvance = Gen7::titleAdvance(version) + static_cast<u32>(ui->spinBoxNeedles->value());
    u32 minFrame = static_cast<u32>(std::max(1.0, std::floor(t.frame(ui->doubleSpinBoxMinSeconds->value()))));
    u32 maxFrame = static_cast<u32>(std::max<double>(minFrame, std::ceil(t.frame(ui->doubleSpinBoxMaxSeconds->value()))));
    u32 menuFrame = ui->doubleSpinBoxMenu->value() > 0 ? static_cast<u32>(std::floor(t.frame(ui->doubleSpinBoxMenu->value()))) : 0;
    auto lead = ui->comboMenuLead->getEnum<Lead>();
    auto filter = ui->filter->getFilter<StateFilter>();

    HoneyGenerator7 generator(continueAdvance, minFrame, maxFrame, *spot, areas[0], lead, *currentProfile, filter);
    generator.setBoy(ui->comboBoxPlayer->currentIndex() == 0);
    generator.setMenuFrame(menuFrame);
    generator.setRain(ui->checkBoxRain->isChecked());
    if (ui->checkBoxSpecial->isChecked())
    {
        SpecialTarget7 target;
        target.specie = static_cast<u16>(ui->spinBoxSpecialSpecie->value());
        target.level = static_cast<u8>(ui->spinBoxSpecialLevel->value());
        target.rate = static_cast<u8>(ui->spinBoxSpecialRate->value());
        target.legend = ui->comboBoxSpecialKind->currentIndex() == 0;
        target.perfectIVs = target.legend ? static_cast<u8>(ui->spinBoxSpecialIVs->value()) : 0;
        generator.setSpecial(target);
    }

    auto states = generator.generate(seed);
    model->addItems(states);
}
