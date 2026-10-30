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

#include "Fishing7.hpp"
#include "ui_Fishing7.h"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Gen7/EncounterArea7.hpp>
#include <Core/Gen7/FishingSpot7.hpp>
#include <Core/Gen7/Gen7.hpp>
#include <Core/Gen7/Generators/FishingGenerator7.hpp>
#include <Core/Gen7/Profile7.hpp>
#include <Core/Gen7/Timer7.hpp>
#include <Core/Util/Translator.hpp>
#include <Form/Controls/Controls.hpp>
#include <Model/Gen7/FishingModel7.hpp>
#include <QMessageBox>
#include <QSettings>
#include <cmath>

static const QString settingPrefix = QStringLiteral("fishing7");

Fishing7::Fishing7(QWidget *parent) : QWidget(parent), ui(new Ui::Fishing7), currentProfile(nullptr)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::Gen7);

    model = new FishingModel7(ui->tableView);
    ui->tableView->setModel(model);

    ui->textBoxSeed->setValues(InputType::Seed32Bit);

    ui->comboMenuLead->addAction(tr("None"), toInt(Lead::None));
    ui->comboMenuLead->addMenu(tr("Synchronize"), Translator::getNatures());
    ui->comboMenuLead->addMenu(tr("Cute Charm"), { { tr("♂ Lead"), toInt(Lead::CuteCharmM) }, { tr("♀ Lead"), toInt(Lead::CuteCharmF) } });
    ui->comboMenuLead->addAction(tr("Suction Cups / Sticky Hold"), toInt(Lead::SuctionCups));
    ui->comboMenuLead->addAction(tr("Compound Eyes"), toInt(Lead::CompoundEyes));
    ui->comboMenuLead->addAction(tr("Pressure / Hustle / Vital Spirit"), toInt(Lead::Pressure));

    ui->filter->disableControls(Controls::Height | Controls::Weight | Controls::Wild);

    ui->labelNeedles->setToolTip(tr("Needles read on the title screen: the RNG sits at the title advance + needles when Continue is pressed"));
    ui->labelModels->setToolTip(tr("Models updated on the first field frame after Continue at the standing position. Only the player keeps "
                                   "blinking afterwards in every logged fishing run (L 4 at Melemele Sea / Hau'oli, L 1 at Kala'e Bay)"));
    ui->labelLoadDraws->setToolTip(tr("Draws before the first field frame: after a zone change the game rolls rand(100) once per fishing "
                                      "spot in the zone (the rare-state lottery); none when the save was made in the zone"));
    ui->checkBoxFidgetOnce->setToolTip(tr("Every logged fishing run saw one fidget pair (frame 4) and never a second; a shore tile may "
                                          "repeat it after rand(90) + 407/412 frames (uncheck). Casting before ~13 s avoids the question"));
    ui->checkBoxRare->setToolTip(tr("The spot was loaded bubbling: hook 21 frames after the bite instead of 16, the rare table and item list. "
                                    "Decided by the rare lottery at the first load after a zone change (Moon: only Hau'oli spot 4, always)"));
    ui->groupBoxTimer->setToolTip(tr("Press A to cast at (frame - offset) / fps seconds after the A press on Continue. No fishing spot has a "
                                     "measured hardware offset: start from the Honey estimate and calibrate with one cast (the wait tells "
                                     "you which cast advance you hit)"));
    ui->labelReel->setToolTip(tr("Reel (A) between the bite and the reel-by time; the hook roll then comes 16 frames after the bite whatever the press"));

    connect(ui->profileDisplay, &ProfileDisplay7::profileChanged, this, &Fishing7::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay7::profilesChanged, this, &Fishing7::profilesChanged);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Fishing7::generate);
    connect(ui->pushButtonCalibrate, &QPushButton::clicked, this, &Fishing7::calibrate);
    connect(ui->comboBoxSpot, &QComboBox::currentIndexChanged, this, &Fishing7::spotIndexChanged);
    connect(ui->doubleSpinBoxOffset, &QDoubleSpinBox::valueChanged, this, &Fishing7::timerChanged);
    connect(ui->doubleSpinBoxFps, &QDoubleSpinBox::valueChanged, this, &Fishing7::timerChanged);
    connect(ui->filter, &Filter::showStatsChanged, model, &FishingModel7::setShowStats);

    updateProfiles();

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Fishing7::~Fishing7()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void Fishing7::updateProfiles()
{
    ui->profileDisplay->updateProfiles();
}

const FishingSpot7 *Fishing7::currentSpot() const
{
    int index = ui->comboBoxSpot->currentIndex();
    return index >= 0 && index < static_cast<int>(spots.size()) ? spots[index] : nullptr;
}

Timer7 Fishing7::timer() const
{
    return Timer7 { static_cast<float>(ui->doubleSpinBoxOffset->value()), static_cast<float>(ui->doubleSpinBoxFps->value()) };
}

void Fishing7::profileChanged(const Profile7 &profile)
{
    currentProfile = &profile;
    Game version = profile.getVersion();
    ui->labelTitleAdvance->setText(tr("Title advance %1").arg(Gen7::titleAdvance(version)));

    spots = FishingSpots7::getSpots(version);
    int current = ui->comboBoxSpot->currentIndex();
    ui->comboBoxSpot->clear();
    for (size_t i = 0; i < spots.size(); i++)
    {
        const FishingSpot7 *spot = spots[i];
        QString name = QString("%1 #%2 (zone %3, table %4)").arg(QString::fromUtf8(spot->location)).arg(i).arg(spot->zone).arg(spot->table);
        if (!spot->reachable)
        {
            name += tr(" [never casts]");
        }
        else if (spot->lapras)
        {
            name += tr(" [from Lapras]");
        }
        else if (!spot->verified)
        {
            name += tr(" [not cast in this game]");
        }
        ui->comboBoxSpot->addItem(name);
    }
    ui->comboBoxSpot->setCurrentIndex(current >= 0 && current < ui->comboBoxSpot->count() ? current : 4);
    spotIndexChanged(ui->comboBoxSpot->currentIndex());
}

void Fishing7::spotIndexChanged(int index)
{
    const FishingSpot7 *spot = currentSpot();
    if (!spot)
    {
        return;
    }

    ui->checkBoxRare->setEnabled(spot->rareTable != 255);
    if (spot->rareTable == 255)
    {
        ui->checkBoxRare->setChecked(false);
    }
    else if (spot->rareRate == 100)
    {
        ui->checkBoxRare->setChecked(true);
    }

    ui->labelSpotInfo->setText(tr("Spot %1 %2 %3. Stand at %4 %5 %6 (%7). Bite = cast + %8 + wait. Rare table %9 (rate %10).")
                                   .arg(spot->spot[0])
                                   .arg(spot->spot[1])
                                   .arg(spot->spot[2])
                                   .arg(spot->stand[0])
                                   .arg(spot->stand[1])
                                   .arg(spot->stand[2])
                                   .arg(QString::fromUtf8(spot->standNote))
                                   .arg(spot->castDelay)
                                   .arg(spot->rareTable == 255 ? tr("none") : QString::number(spot->rareTable))
                                   .arg(spot->rareRate));

    QSettings setting;
    QString key = QString("%1/offset/%2/%3/%4").arg(settingPrefix).arg(toInt(currentProfile->getVersion())).arg(spot->zone).arg(spot->table);
    if (setting.contains(key))
    {
        ui->doubleSpinBoxOffset->setValue(setting.value(key).toDouble());
        ui->doubleSpinBoxFps->setValue(setting.value(key + "/fps", 29.913).toDouble());
        ui->labelTimerStatus->setText(tr("Calibrated"));
    }
    else
    {
        // No fishing tile has a measured offset: the Honey rule of thumb (Azahar -61.5 - 0.6 L, hardware 43 frames earlier)
        double azahar = -61.5 - 0.6 * ui->spinBoxModels->value();
        ui->doubleSpinBoxOffset->setValue(ui->comboBoxTimerSource->currentIndex() == 1 ? azahar : azahar - 43.0);
        ui->doubleSpinBoxFps->setValue(ui->comboBoxTimerSource->currentIndex() == 1 ? 29.91 : 29.913);
        ui->labelTimerStatus->setText(tr("Estimate: calibrate with one cast"));
    }
    timerChanged();
}

void Fishing7::timerChanged()
{
    model->setTimer(timer());
}

void Fishing7::calibrate()
{
    const FishingSpot7 *spot = currentSpot();
    if (!spot || !currentProfile)
    {
        return;
    }

    double seconds = ui->doubleSpinBoxCalibrateSeconds->value();
    u32 hit = static_cast<u32>(ui->spinBoxCalibrateFrame->value());
    float offset = timer().calibrate(seconds, hit);
    ui->doubleSpinBoxOffset->setValue(offset);

    QSettings setting;
    QString key = QString("%1/offset/%2/%3/%4").arg(settingPrefix).arg(toInt(currentProfile->getVersion())).arg(spot->zone).arg(spot->table);
    setting.setValue(key, offset);
    setting.setValue(key + "/fps", ui->doubleSpinBoxFps->value());
    ui->labelTimerStatus->setText(tr("Calibrated: offset %1 frames").arg(offset, 0, 'f', 1));
}

void Fishing7::generate()
{
    const FishingSpot7 *spot = currentSpot();
    if (!spot || !currentProfile || !ui->filter->isValid())
    {
        return;
    }

    Game version = currentProfile->getVersion();
    bool night = ui->comboBoxTimeOfDay->currentIndex() == 1;
    bool rare = ui->checkBoxRare->isChecked() && spot->rareTable != 255;
    auto areas = Encounters7::getEncounters(version, spot->zone, rare ? spot->rareTable : spot->table, night);
    if (areas.empty())
    {
        QMessageBox msg(QMessageBox::Warning, tr("No encounter table"), tr("No encounter table is embedded for this zone and spot"));
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

    FieldModel7 fieldModel;
    fieldModel.continueModels = static_cast<u8>(ui->spinBoxModels->value());
    fieldModel.persist = 1;
    fieldModel.loadDraws = static_cast<u8>(ui->spinBoxLoadDraws->value());
    fieldModel.fidget = static_cast<u8>(ui->spinBoxFidget->value());
    fieldModel.fidgetOnce = ui->checkBoxFidgetOnce->isChecked();

    auto lead = ui->comboMenuLead->getEnum<Lead>();
    auto filter = ui->filter->getFilter<StateFilter>();
    FishingGenerator7 generator(continueAdvance, minFrame, maxFrame, *spot, fieldModel, areas[0], lead, *currentProfile, filter);
    generator.setBoy(ui->comboBoxPlayer->currentIndex() == 0);
    generator.setMenuFrame(menuFrame);
    generator.setRare(rare);
    generator.setShowAll(ui->checkBoxShowAll->isChecked());
    model->addItems(generator.generate(seed));
}
