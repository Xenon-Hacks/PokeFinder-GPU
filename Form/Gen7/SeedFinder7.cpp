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

#include "SeedFinder7.hpp"
#include "ui_SeedFinder7.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen7/Gen7.hpp>
#include <Core/Gen7/Searchers/NeedleSearcher7.hpp>
#include <Core/Util/OpenCL.hpp>
#include <QMessageBox>
#include <QSettings>
#include <QStandardItemModel>
#include <QTimer>

static const QString settingPrefix = QStringLiteral("seedFinder7");

SeedFinder7::SeedFinder7(QWidget *parent) : QWidget(parent), ui(new Ui::SeedFinder7)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({ tr("Seed"), tr("Advance after needles"), tr("Next 4 needles") });
    ui->tableView->setModel(model);

    ui->comboBoxGame->setup({ toInt(Game::Sun), toInt(Game::Moon), toInt(Game::US), toInt(Game::UM) });
    ui->checkBoxGPU->setChecked(OpenCL::isAvailable());
    ui->checkBoxGPU->setEnabled(OpenCL::isAvailable());
    ui->checkBoxGPU->setText(OpenCL::isAvailable() ? tr("GPU: %1").arg(QString::fromStdString(OpenCL::deviceName())) : tr("No OpenCL GPU"));

    ui->labelNeedles->setToolTip(tr("Clock poses of the Continue card, 0-16 clockwise from 12 o'clock, in the order drawn: the first "
                                    "needle is the first title -> Continue load, each B then A on the title draws the next. 8 needles "
                                    "give a unique seed in most boots; add more while several seeds fit"));
    ui->labelNote->setText(tr("The boot seed comes from the console tick and cannot be planned. The search runs all 4,294,967,296 "
                              "seeds: about 65-80 s on an RTX 4090, hours on the CPU."));

    connect(ui->pushButtonSearch, &QPushButton::clicked, this, &SeedFinder7::search);

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

SeedFinder7::~SeedFinder7()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

std::vector<u8> SeedFinder7::needles(bool *ok) const
{
    std::vector<u8> result;
    *ok = true;
    QString text = ui->lineEditNeedles->text();
    text.replace(',', ' ');
    for (const QString &part : text.split(' ', Qt::SkipEmptyParts))
    {
        bool valid;
        int value = part.toInt(&valid);
        if (!valid || value < 0 || value > 16)
        {
            *ok = false;
            return {};
        }
        result.emplace_back(static_cast<u8>(value));
    }
    if (result.empty() || result.size() > 16)
    {
        *ok = false;
    }
    return result;
}

void SeedFinder7::search()
{
    bool ok;
    auto values = needles(&ok);
    if (!ok)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid needles"), tr("Enter 1 to 16 needles, each 0-16, separated by spaces"));
        msg.exec();
        return;
    }

    model->removeRows(0, model->rowCount());
    ui->pushButtonSearch->setEnabled(false);
    ui->pushButtonCancel->setEnabled(true);
    ui->progressBar->setValue(0);

    Game version = ui->comboBoxGame->getEnum<Game>();
    u32 title = Gen7::titleAdvance(version);
    auto *searcher = new NeedleSearcher7(title, values);

    QSettings settings;
    int threads = settings.value("settings/threads").toInt();
    searcher->startSearch(threads, ui->checkBoxGPU->isChecked(), 0, 1ULL << 32);

    auto *timer = new QTimer(this);
    connect(ui->pushButtonCancel, &QPushButton::clicked, timer, [searcher] { searcher->cancelSearch(); });
    connect(timer, &QTimer::timeout, this, [this, searcher, timer, title, values] {
        bool searching = searcher->isSearching();
        for (u32 seed : searcher->getResults())
        {
            auto next = NeedleSearcher7::needlesFor(seed, title + static_cast<u32>(values.size()), 4);
            QStringList preview;
            for (u8 needle : next)
            {
                preview.append(QString::number(needle));
            }
            QList<QStandardItem *> row;
            row.append(new QStandardItem(QString::number(seed, 16).toUpper().rightJustified(8, '0')));
            row.append(new QStandardItem(QString::number(title + static_cast<u32>(values.size()))));
            row.append(new QStandardItem(preview.join(' ')));
            for (auto *item : row)
            {
                item->setEditable(false);
            }
            model->appendRow(row);
        }
        ui->progressBar->setValue(searcher->getProgress());

        if (!searching)
        {
            timer->stop();
            ui->pushButtonSearch->setEnabled(true);
            ui->pushButtonCancel->setEnabled(false);
            ui->progressBar->setValue(100);
            std::string error = searcher->getError();
            bool gpu = searcher->getUsedGPU();
            delete searcher;
            timer->deleteLater();

            if (!error.empty() && !gpu)
            {
                QMessageBox msg(QMessageBox::Warning, tr("GPU search unavailable"),
                                tr("Searched on the CPU instead: %1").arg(QString::fromStdString(error)));
                msg.exec();
            }
            if (model->rowCount() > 1)
            {
                ui->labelResult->setText(tr("%1 seeds fit: read more needles and search again").arg(model->rowCount()));
            }
            else if (model->rowCount() == 1)
            {
                ui->labelResult->setText(tr("One seed fits. Press Continue to start the field at the advance shown"));
            }
            else
            {
                ui->labelResult->setText(tr("No seed fits these needles: check the order and the game"));
            }
        }
    });
    timer->start(500);
}
