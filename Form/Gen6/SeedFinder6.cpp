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
#include <Core/Gen6/SeedWindow6.hpp>
#include <Core/Parents/ProfileLoader.hpp>
#include <Core/Util/OpenCL.hpp>
#include <Core/Util/Translator.hpp>
#include <Form/Gen6/SeedWindowUI6.hpp>
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
        bool container;
    };

    // Encounters run the 60-output container before the EC; fossils and gifts do not. Fixed-ability legends skip the ability draw.
    constexpr Preset presets[] = {
        { 0, 1, 0, true, true }, // Wild
        { 0, 3, 0, true, true }, // Wild + Shiny Charm
        { 0, 5, 2, true, true }, // Friend Safari
        { 0, 7, 2, true, true }, // Friend Safari + Shiny Charm
        { 81, 1, 0, true, false }, // Gift / fossil (OR fossil: A press + 81; X/Y not timed)
        { 0, 1, 3, false, true }, // X/Y legend: fixed ability, no ability draw
        { 0, 1, 3, true, true } // OR/AS legend: the ability is drawn even for single-ability legends (live OR)
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

}

SeedFinder6::SeedFinder6(QWidget *parent) : QWidget(parent), ui(new Ui::SeedFinder6)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::Gen6);

    model = new SeedModel6(ui->tableView);
    ui->tableView->setModel(model);

    ui->textBoxMinFrame->setValues(InputType::Advance32Bit);
    ui->textBoxMaxFrame->setValues(InputType::Advance32Bit);
    ui->textBoxDelay->setValues(InputType::Advance32Bit);
    ui->textBoxMinGap->setValues(InputType::Advance32Bit);
    ui->textBoxMaxGap->setValues(InputType::Advance32Bit);
    ui->textBoxTimeSeed->setValues(InputType::Seed32Bit);
    ui->textBoxTitleDelay->setValues(InputType::Advance32Bit);
    ui->textBoxTargetFrame->setValues(InputType::Advance32Bit);
    ui->textBoxLostTicks->setValues(InputType::Advance32Bit);
    ui->dateTimeEditAround->setDisplayFormat("yyyy-MM-dd hh:mm");
    ui->dateTimeEditAround->setDateTimeRange(QDateTime(QDate(2000, 1, 1), QTime(0, 0)), QDateTime(QDate(2099, 12, 31), QTime(23, 59)));
    ui->dateTimeEditAround->setDateTime(QDateTime::currentDateTime());
    ui->labelAround->setToolTip(tr("Earliest console time to aim for; the seed comes round once every 49 days 17:02:47.296"));
    ui->labelPressToSeedMs->setToolTip(tr("ms of console time from the A press on the title screen to the seed being set, measured from RAM on "
                                          "Azahar: X/Y 86 frames + about 16 ms = 1453, Omega Ruby 236 frames = 3945, Alpha Sapphire 237 frames "
                                          "= 3960. It follows the profile's game until you calibrate it. Calibrate on hardware, where a slower "
                                          "save read adds to it (most of all on OR/AS)"));
    ui->labelLostTicks->setToolTip(tr("MT ticks lost to load screens before the trigger: frames while the game is loading get no "
                                      "per-frame advance. About 60 from the seed to Honey on Azahar on every game (X/Y: 32-34 for the "
                                      "map load after Continue, 27-29 for opening the bag; OR/AS Route 103: 59-60). About 52 to Sweet "
                                      "Scent on OR/AS. 0 if your target frame is already in frames"));
    ui->labelLeadS->setToolTip(tr("Seconds you want between confirming the clock and the timed press"));
    ui->labelTargetFrame->setToolTip(tr("MT frame (advances since the seed) to trigger the target at"));
    ui->labelPlan->setWordWrap(true);
    ui->labelPlan->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->pushButtonCalibrate->setEnabled(false);
    ui->textBoxRecoverSeedA->setValues(InputType::Seed32Bit);
    ui->textBoxRecoverSeedB->setValues(InputType::Seed32Bit);
    for (auto *edit : { ui->dateTimeEditRecoverA, ui->dateTimeEditRecoverB })
    {
        edit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
        edit->setDateTime(QDateTime::currentDateTime());
    }
    ui->labelRecoverHelp->setWordWrap(true);
    ui->labelRecover->setWordWrap(true);
    ui->labelRecover->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->labelToleranceS->setToolTip(tr("How far off each noted title time may be"));
    ui->pushButtonRecoverSave->setEnabled(false);
    ui->labelMinGap->setToolTip(tr("Fewest MT frames between the previous catch's trigger and this one"));
    ui->labelMaxGap->setToolTip(tr("Most MT frames between the previous catch's trigger and this one (about 58 per second of play, 60 when idle)"));
    ui->labelNarrow->setWordWrap(true);
    ui->labelNarrowHelp->setWordWrap(true);

    ui->dateTimeEditStart->setDisplayFormat("yyyy-MM-dd hh:mm:ss.zzz");
    ui->dateTimeEditEnd->setDisplayFormat("yyyy-MM-dd hh:mm:ss.zzz");
    QDateTime now = QDateTime::currentDateTime();
    ui->dateTimeEditStart->setDateTime(now.addSecs(-30));
    ui->dateTimeEditEnd->setDateTime(now.addSecs(30));

    ui->comboBoxMode->setToolTip(tr("The game seeds MT with the save's Misc[0x00] plus the console clock in ms, read after the A press on "
                                     "the title screen: 86 frames (about 1.45 s) later on X/Y, 236 (about 3.95 s) on OR/AS. Misc[0x00] "
                                     "changes on every in-game save."));
    ui->labelMinFrame->setToolTip(tr("MT frame of the trigger (button press or step); the EC is frame + delay + 60, or + delay for gifts"));
    ui->checkBoxContainer->setToolTip(tr("Encounters generate 6 dummy Pokemon (60 outputs) between the trigger and the EC; fossils and "
                                         "gifts go straight to the EC"));
    ui->labelDelay->setToolTip(tr("MT frames from the press to the 60-frame skip (Mewtwo 3, Zygarde 2, Xerneas 161, Yveltal 209; "
                                  "OR/AS Regice 186, Deoxys 301, Groudon 3831, Kyogre 3843)"));

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
    connect(ui->pushButtonNarrow, &QPushButton::clicked, this, &SeedFinder6::narrow);
    connect(ui->pushButtonSeedTime, &QPushButton::clicked, this, &SeedFinder6::seedTime);
    connect(ui->pushButtonCalibrate, &QPushButton::clicked, this, &SeedFinder6::calibrate);
    connect(ui->pushButtonRecover, &QPushButton::clicked, this, &SeedFinder6::recover);
    connect(ui->pushButtonRecoverSave, &QPushButton::clicked, this, &SeedFinder6::saveRecovered);
    connect(ui->tableView, &QTableView::clicked, this, [this](const QModelIndex &index) {
        ui->textBoxTimeSeed->setText(QString::number(model->getItem(index.row()).seed, 16).toUpper());
    });

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
        ui->checkBoxContainer->setChecked(preset.container);
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

    // X/Y and OR/AS seed about 2.5 s apart after the press, so a value calibrated on one must not carry over to the other: keep one
    // calibrated value per game family, and show the game's default where none was calibrated
    auto family = [](Game version) { return (version & Game::ORAS) != Game::None ? 1 : 0; };
    if (pressToSeedVersion != 0)
    {
        Game old = static_cast<Game>(pressToSeedVersion);
        u64 current = ui->textBoxTitleDelay->getUInt();
        pressToSeed[family(old)] = current == SeedWindow6::pressToSeedMs(old) ? 0 : current;
    }
    pressToSeedVersion = toInt(profile.getVersion());
    u64 calibrated = pressToSeed[family(profile.getVersion())];
    ui->textBoxTitleDelay->setText(QString::number(calibrated != 0 ? calibrated : SeedWindow6::pressToSeedMs(profile.getVersion())));
}

bool SeedFinder6::readCatch(SeedSearch6Settings &settings)
{
    QLineEdit *edits[] = { ui->lineEditHP, ui->lineEditAtk, ui->lineEditDef, ui->lineEditSpA, ui->lineEditSpD, ui->lineEditSpe };
    for (int i = 0; i < 6; i++)
    {
        if (!parseRange(edits[i]->text(), settings.ivMin[i], settings.ivMax[i]))
        {
            QMessageBox msg(QMessageBox::Warning, tr("Invalid IVs"), tr("Enter each IV as a value (31) or a range (0-31)"));
            msg.exec();
            return false;
        }
    }

    settings.skip = ui->textBoxDelay->getUInt() + (ui->checkBoxContainer->isChecked() ? 60 : 0) + 1 + ui->spinBoxRolls->value();
    settings.ivCount = ui->spinBoxIVCount->value();
    settings.nature = ui->comboBoxNature->getCurrentUChar();
    settings.natureOffset = ui->checkBoxAbility->isChecked() ? 1 : 0;
    return true;
}

void SeedFinder6::search()
{
    SeedSearch6Settings settings;
    if (!readCatch(settings))
    {
        return;
    }

    settings.minFrame = ui->textBoxMinFrame->getUInt();
    settings.maxFrame = ui->textBoxMaxFrame->getUInt();
    if (settings.maxFrame < settings.minFrame)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid frames"), tr("Max frame must be at least min frame"));
        msg.exec();
        return;
    }

    u32 start;
    u64 count;
    u64 clockStart = 0;
    if (ui->comboBoxMode->currentIndex() == 0)
    {
        clockStart = SeedWindowUI6::consoleMs(ui->dateTimeEditStart->dateTime());
        u64 clockEnd = SeedWindowUI6::consoleMs(ui->dateTimeEditEnd->dateTime());
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

void SeedFinder6::narrow()
{
    const auto &candidates = model->getModel();
    if (candidates.empty())
    {
        QMessageBox msg(QMessageBox::Warning, tr("No candidates"), tr("Search with the first catch, then narrow with the next ones"));
        msg.exec();
        return;
    }

    SeedSearch6Settings settings;
    if (!readCatch(settings))
    {
        return;
    }

    u32 minGap = ui->textBoxMinGap->getUInt();
    u32 maxGap = ui->textBoxMaxGap->getUInt();
    if (maxGap < minGap)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid gap"), tr("Max gap must be at least min gap"));
        msg.exec();
        return;
    }

    size_t before = candidates.size();
    auto results = SeedSearcher6::narrow(settings, candidates, minGap, maxGap);
    model->clearModel();
    model->addItems(results);

    std::vector<u32> seeds;
    for (const auto &result : results)
    {
        seeds.emplace_back(result.seed);
    }
    std::ranges::sort(seeds);
    size_t unique = std::ranges::distance(seeds.begin(), std::unique(seeds.begin(), seeds.end()));
    ui->labelNarrow->setText(unique == 1 ? tr("Seed found: %1 candidates became 1 seed").arg(before)
                                         : tr("%1 candidates became %2 seeds; add another catch").arg(before).arg(unique));
}

void SeedFinder6::seedTime()
{
    u32 seed = ui->textBoxTimeSeed->getUInt();
    u64 around = SeedWindowUI6::consoleMs(ui->dateTimeEditAround->dateTime());
    u64 delay = ui->textBoxTitleDelay->getUInt();
    u64 lead = static_cast<u64>(ui->spinBoxLead->value()) * 1000;
    u32 frame = ui->textBoxTargetFrame->getUInt();

    auto plan = SeedWindow6::planTimer(seed, currentProfile->getSaveSeed(), around, delay, lead);
    if (!plan.valid)
    {
        ui->labelPlan->setText(tr("The next time for this seed falls after 2099-12-31 23:59, the latest 3DS clock setting. "
                                  "Pick an earlier Around date."));
        ui->pushButtonCalibrate->setEnabled(false);
        return;
    }
    planClockSet = plan.clockSet;
    planTimer = plan.timer;
    ui->pushButtonCalibrate->setEnabled(true);

    // The title screen replays the intro about 45 s after PRESS START shows (about 15 s after launch); a press on the intro
    // only skips it (live Y; not timed on OR/AS), so launch about 40 s before the A press
    constexpr u64 launchBefore = 40000;
    QString launch = plan.timer > launchBefore
        ? tr("at %1 s on the timer, about 40 s before the A press").arg(static_cast<double>(plan.timer - launchBefore) / 1000.0, 0, 'f', 0)
        : tr("right away");
    QString text = tr("1. Set the 3DS clock to %1 and start your timer the moment you confirm it.\n"
                      "2. Launch the game from the HOME Menu %2. The title screen replays its intro about 45 s after PRESS START "
                      "shows, and an A press during the intro does not set the seed.\n"
                      "3. Press A on the title screen (PRESS START) at %3 ms (%4 s). The seed %5 is set at %6.\n")
                       .arg(SeedWindowUI6::consoleTime(plan.clockSet).toString("yyyy-MM-dd hh:mm"))
                       .arg(launch)
                       .arg(plan.timer)
                       .arg(static_cast<double>(plan.timer) / 1000.0, 0, 'f', 3)
                       .arg(QString::number(seed, 16).toUpper().rightJustified(8, '0'))
                       .arg(SeedWindowUI6::consoleTime(plan.target).toString("hh:mm:ss.zzz"));
    if (frame != 0)
    {
        // The main MT ticks once per main-loop pass, so frames spent loading add frames without advances
        constexpr double frameMs = 1000.0 / 59.8261;
        u32 lost = ui->textBoxLostTicks->getUInt();
        u64 frames = static_cast<u64>(frame) + lost;
        double afterPress = static_cast<double>(delay) + static_cast<double>(frames) * frameMs;
        text += tr("4. Trigger the target at MT frame %1: %2 frames (%3 s) after the seed with %4 ticks lost to loading, "
                   "%5 s after the A press (%6 frames on a 59.8261 fps timer).\n")
                    .arg(frame)
                    .arg(frames)
                    .arg(static_cast<double>(frames) * frameMs / 1000.0, 0, 'f', 3)
                    .arg(lost)
                    .arg(afterPress / 1000.0, 0, 'f', 3)
                    .arg(static_cast<u64>(afterPress / frameMs + 0.5));
        // Input timing, loading and the trigger animation each scatter by 1-2 frames, more than a 3-frame horde spot (live Y)
        text += tr("The frame you land on scatters by a few frames from attempt to attempt. Adjust this timing from the average of "
                   "3 to 5 misses, not from a single one.\n");
    }
    text += tr("Afterwards, find the seed you got with a clock search, select its row and press Calibrate.");
    ui->labelPlan->setText(text);
}

void SeedFinder6::calibrate()
{
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid() || model->getItem(index.row()).ms == 0)
    {
        QMessageBox msg(QMessageBox::Warning, tr("No result selected"),
                        tr("Run a save seed + console clock search for the attempt, then select the seed you got"));
        msg.exec();
        return;
    }

    u64 found = model->getItem(index.row()).ms;
    s64 delay = SeedWindow6::measuredDelay(found, planClockSet, planTimer);
    s64 old = ui->textBoxTitleDelay->getUInt();
    if (delay < 0)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Unexpected result"),
                        tr("That seed was set %1 ms before the timed press, so it is not from this plan").arg(-delay));
        msg.exec();
        return;
    }

    ui->textBoxTitleDelay->setText(QString::number(delay));
    ui->labelPlan->setText(tr("Measured delay %1 ms (was %2 ms, off by %3 ms). Press Plan Timer again for the next attempt.")
                               .arg(delay)
                               .arg(old)
                               .arg(delay - old));
}

void SeedFinder6::recover()
{
    // The seed is set 86 frames (about 1453 ms, X/Y) or 236 frames (about 3945 ms, OR/AS) after the A press on the title screen; the
    // press-to-seed field holds the game's default or the calibrated value
    u64 pressToSeed = ui->textBoxTitleDelay->getUInt();
    std::vector<SeedWindow6::Boot> boots;
    if (!ui->textBoxRecoverSeedA->text().isEmpty())
    {
        boots.push_back({ SeedWindowUI6::consoleMs(ui->dateTimeEditRecoverA->dateTime()) + pressToSeed, ui->textBoxRecoverSeedA->getUInt() });
    }
    if (!ui->textBoxRecoverSeedB->text().isEmpty())
    {
        boots.push_back({ SeedWindowUI6::consoleMs(ui->dateTimeEditRecoverB->dateTime()) + pressToSeed, ui->textBoxRecoverSeedB->getUInt() });
    }
    if (boots.empty())
    {
        QMessageBox msg(QMessageBox::Warning, tr("No boots"), tr("Enter the seed of at least one boot and when you pressed A on its title screen"));
        msg.exec();
        return;
    }

    u64 tolerance = static_cast<u64>(ui->spinBoxRecoverTolerance->value()) * 1000;
    auto recovery = SeedWindow6::recoverSaveSeed(boots, tolerance);
    recovered = recovery.saveSeed;
    hasRecovered = recovery.consistent;
    ui->pushButtonRecoverSave->setEnabled(hasRecovered);

    QString seed = QString::number(recovered, 16).toUpper().rightJustified(8, '0');
    if (!recovery.consistent)
    {
        ui->labelRecover->setText(tr("The boots disagree by %1 ms. One of the seeds is probably wrong; narrow it with another catch.")
                                      .arg(recovery.worstMismatch));
    }
    else if (boots.size() > 1)
    {
        ui->labelRecover->setText(tr("Save seed: %1. Both boots agree within %2 ms. It may be off by your timing error, "
                                     "which acts like a clock offset: Calibrate removes it.")
                                      .arg(seed)
                                      .arg(recovery.worstMismatch));
    }
    else
    {
        ui->labelRecover->setText(tr("Save seed: %1. It may be off by your timing error, which acts like a clock offset: "
                                     "Calibrate removes it. A second boot checks that the seed is right.")
                                      .arg(seed));
    }
}

void SeedFinder6::saveRecovered()
{
    if (!hasRecovered)
    {
        return;
    }

    if (currentProfile->getName() == "-")
    {
        QMessageBox msg(QMessageBox::Warning, tr("No profile selected"), tr("Create or select a Gen 6 profile to store the save seed in"));
        msg.exec();
        return;
    }

    Profile6 updated(currentProfile->getName(), currentProfile->getVersion(), currentProfile->getTID(), currentProfile->getSID(),
                     currentProfile->getShinyCharm(), recovered);
    ProfileLoader6::updateProfile(updated, *currentProfile);
    ui->profileDisplay->updateProfiles();
    emit profilesChanged(6);
    ui->labelRecover->setText(tr("Saved %1 to the profile").arg(QString::number(recovered, 16).toUpper().rightJustified(8, '0')));
}
