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

#include "SeedFinder6.hpp"
#include "ui_SeedFinder6.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen6/Profile6.hpp>
#include <Core/Gen6/Searchers/SeedSearcher6.hpp>
#include <Core/Util/OpenCL.hpp>
#include <Core/Util/Translator.hpp>
#include <Model/Gen6/StateModel6.hpp>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSettings>
#include <QTimeZone>
#include <QThread>
#include <QTimer>

static const QString settingPrefix = QStringLiteral("seedFinder6");

namespace
{
    struct Preset
    {
        u32 delay;
        u8 rolls;
        u8 ivCount;
        bool ability;
    };

    // Wild: EC = trigger + 60; gifts and legends skip the ability draw when it is fixed
    constexpr Preset presets[] = {
        { 0, 1, 0, true }, // Wild
        { 0, 3, 0, true }, // Wild + Shiny Charm
        { 0, 5, 2, true }, // Friend Safari
        { 0, 7, 2, true }, // Friend Safari + Shiny Charm
        { 80, 1, 0, true }, // Gift / fossil
        { 0, 1, 3, false } // Legend
    };

    bool parseRange(const QString &text, u8 &min, u8 &max)
    {
        QString trimmed = text.trimmed();
        if (trimmed.isEmpty() || trimmed == "-")
        {
            min = 0;
            max = 31;
            return true;
        }

        static const QRegularExpression re("^(\\d{1,2})(?:\\s*-\\s*(\\d{1,2}))?$");
        auto match = re.match(trimmed);
        if (!match.hasMatch())
        {
            return false;
        }
        min = match.captured(1).toUInt();
        max = match.captured(2).isEmpty() ? min : match.captured(2).toUInt();
        return min <= max && max <= 31;
    }

    u64 consoleMs(const QDateTime &dateTime)
    {
        // The console clock has no time zone, read the fields as they are
        QDateTime utc(dateTime.date(), dateTime.time(), QTimeZone::UTC);
        QDateTime epoch(QDate(2000, 1, 1), QTime(0, 0), QTimeZone::UTC);
        return static_cast<u64>(epoch.msecsTo(utc));
    }
}

SeedFinder6::SeedFinder6(QWidget *parent) : QWidget(parent), ui(new Ui::SeedFinder6)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::XY);

    model = new SeedModel6(ui->tableView);
    ui->tableView->setModel(model);

    ui->textBoxMinFrame->setValues(InputType::Advance32Bit);
    ui->textBoxMaxFrame->setValues(InputType::Advance32Bit);
    ui->textBoxDelay->setValues(InputType::Advance32Bit);

    ui->dateTimeEditStart->setDisplayFormat("yyyy-MM-dd hh:mm:ss.zzz");
    ui->dateTimeEditEnd->setDisplayFormat("yyyy-MM-dd hh:mm:ss.zzz");
    QDateTime now = QDateTime::currentDateTime();
    ui->dateTimeEditStart->setDateTime(now.addSecs(-30));
    ui->dateTimeEditEnd->setDateTime(now.addSecs(30));

    ui->comboBoxMode->setToolTip(tr("The game seeds MT with the save's Misc[0x00] plus the console clock in ms when the title screen loads"));
    ui->labelMinFrame->setToolTip(tr("MT frame of the trigger (button press or step); the EC is frame + delay + 60"));
    ui->labelDelay->setToolTip(tr("MT frames from the press to the 60-frame skip (Mewtwo 3, Zygarde 2, Xerneas 161, Yveltal 210)"));

    ui->comboBoxNature->addItem(tr("Any"), 255);
    const auto &natures = Translator::getNatures();
    for (size_t i = 0; i < natures.size(); i++)
    {
        ui->comboBoxNature->addItem(QString::fromStdString(natures[i]), static_cast<int>(i));
    }

    QSettings settings;
    ui->checkBoxGPU->setChecked(OpenCL::isAvailable() && settings.value("settings/gpu", true).toBool());
    ui->checkBoxGPU->setEnabled(OpenCL::isAvailable());
    if (OpenCL::isAvailable())
    {
        ui->checkBoxGPU->setText(tr("Use the GPU (%1)").arg(QString::fromStdString(OpenCL::deviceName())));
    }
    ui->pushButtonCancel->setEnabled(false);

    connect(ui->profileDisplay, &ProfileDisplay6::profileChanged, this, &SeedFinder6::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay6::profilesChanged, this, &SeedFinder6::profilesChanged);
    connect(ui->comboBoxEncounter, &QComboBox::currentIndexChanged, this, &SeedFinder6::encounterIndexChanged);
    connect(ui->comboBoxMode, &QComboBox::currentIndexChanged, this, &SeedFinder6::modeIndexChanged);
    connect(ui->pushButtonSearch, &QPushButton::clicked, this, &SeedFinder6::search);

    updateProfiles();
    encounterIndexChanged(0);
    modeIndexChanged(0);

    settings.beginGroup(settingPrefix);
    if (settings.contains("geometry"))
    {
        this->restoreGeometry(settings.value("geometry").toByteArray());
    }
    settings.endGroup();
}

SeedFinder6::~SeedFinder6()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void SeedFinder6::updateProfiles()
{
    ui->profileDisplay->updateProfiles();
}

void SeedFinder6::encounterIndexChanged(int index)
{
    if (index >= 0 && index < static_cast<int>(std::size(presets)))
    {
        const Preset &preset = presets[index];
        ui->textBoxDelay->setText(QString::number(preset.delay));
        ui->spinBoxRolls->setValue(preset.rolls);
        ui->spinBoxIVCount->setValue(preset.ivCount);
        ui->checkBoxAbility->setChecked(preset.ability);
    }
}

void SeedFinder6::modeIndexChanged(int index)
{
    bool clock = index == 0;
    ui->dateTimeEditStart->setEnabled(clock);
    ui->dateTimeEditEnd->setEnabled(clock);
}

void SeedFinder6::profileChanged(const Profile6 &profile)
{
    currentProfile = &profile;
}

void SeedFinder6::search()
{
    SeedSearch6Settings settings;
    QLineEdit *edits[] = { ui->lineEditHP, ui->lineEditAtk, ui->lineEditDef, ui->lineEditSpA, ui->lineEditSpD, ui->lineEditSpe };
    for (int i = 0; i < 6; i++)
    {
        if (!parseRange(edits[i]->text(), settings.ivMin[i], settings.ivMax[i]))
        {
            QMessageBox msg(QMessageBox::Warning, tr("Invalid IVs"), tr("Enter each IV as a value (31) or a range (0-31)"));
            msg.exec();
            return;
        }
    }

    settings.minFrame = ui->textBoxMinFrame->getUInt();
    settings.maxFrame = ui->textBoxMaxFrame->getUInt();
    if (settings.maxFrame < settings.minFrame)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid frames"), tr("Max frame must be at least min frame"));
        msg.exec();
        return;
    }
    settings.skip = ui->textBoxDelay->getUInt() + 60 + 1 + ui->spinBoxRolls->value();
    settings.ivCount = ui->spinBoxIVCount->value();
    settings.nature = ui->comboBoxNature->getCurrentUChar();
    settings.natureOffset = ui->checkBoxAbility->isChecked() ? 1 : 0;

    u32 start;
    u64 count;
    u64 clockStart = 0;
    if (ui->comboBoxMode->currentIndex() == 0)
    {
        clockStart = consoleMs(ui->dateTimeEditStart->dateTime());
        u64 clockEnd = consoleMs(ui->dateTimeEditEnd->dateTime());
        if (clockEnd < clockStart)
        {
            QMessageBox msg(QMessageBox::Warning, tr("Invalid clock"), tr("Clock end must be after clock start"));
            msg.exec();
            return;
        }
        count = std::min<u64>(clockEnd - clockStart + 1, 0x100000000ULL);
        start = currentProfile->getSaveSeed() + static_cast<u32>(clockStart);
    }
    else
    {
        start = 0;
        count = 0x100000000ULL;
    }

    model->clearModel();
    ui->pushButtonSearch->setEnabled(false);
    ui->pushButtonCancel->setEnabled(true);
    ui->progressBar->setValue(0);

    QSettings setting;
    int threads = setting.value("settings/threads", QThread::idealThreadCount()).toInt();

    auto *searcher = new SeedSearcher6(settings);
    searcher->startSearch(threads, ui->checkBoxGPU->isChecked(), start, count, clockStart);

    auto *timer = new QTimer(this);
    connect(ui->pushButtonCancel, &QPushButton::clicked, timer, [this, searcher] {
        searcher->cancelSearch();
        ui->pushButtonCancel->setEnabled(false);
    });
    connect(timer, &QTimer::timeout, this, [this, searcher, timer] {
        bool searching = searcher->isSearching();
        model->addItems(searcher->getResults());
        ui->progressBar->setValue(searcher->getProgress());

        if (!searching)
        {
            timer->stop();
            ui->pushButtonSearch->setEnabled(true);
            ui->pushButtonCancel->setEnabled(false);

            std::string error = searcher->getError();
            bool usedGPU = searcher->getUsedGPU();
            delete searcher;
            timer->deleteLater();

            if (!usedGPU && !error.empty() && ui->checkBoxGPU->isChecked())
            {
                QMessageBox msg(QMessageBox::Warning, tr("GPU search unavailable"),
                                tr("Searched on the CPU instead: %1").arg(QString::fromStdString(error)));
                msg.exec();
            }
        }
    });

    timer->start(500);
}
