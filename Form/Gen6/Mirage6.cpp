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

#include "Mirage6.hpp"
#include "ui_Mirage6.h"
#include <Core/Enum/Game.hpp>
#include <Core/Gen6/Mirage6.hpp>
#include <Core/Gen6/Save6.hpp>
#include <Core/Util/Translator.hpp>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

static const QString settingPrefix = QStringLiteral("mirage6");

Mirage6::Mirage6(QWidget *parent) : QWidget(parent), ui(new Ui::Mirage6)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->textBoxValue->setValues(InputType::Seed32Bit);
    ui->textBoxTID->setValues(InputType::TIDSID);
    ui->dateEditStart->setDisplayFormat("yyyy-MM-dd");
    ui->dateEditStart->setDate(QDate::currentDate());

    ui->labelValue->setToolTip(tr("RandomGroup daily value, u32 at save offset 0x1600 (Load Save reads it)"));
    ui->labelTID->setToolTip(tr("Trainer ID: the spot of the day depends on it, the hidden item does not"));
    ui->labelStart->setToolTip(tr("The day the save was last played. The daily value in the save belongs to that day; it advances "
                                  "once for every day the game counts when it next loads, or at midnight while playing"));
    ui->labelHelp->setText(tr("Lists the Mirage spot shown on the sky map each day, and the item hidden at it.\n\n"
                              "Spot = TinyMT(daily value + TID) % 33 + 1; the daily value steps once per day. Spot 1 is Crescent "
                              "Isle (landing checked on Omega Ruby); spots 2-33 are the 8 Mirage Forests, Caves, Islands and "
                              "Mountains from the sky map's own records. The daily value steps once for each day the game counts "
                              "(at load or at midnight), so the list holds as long as the 3DS date runs forward normally."));

    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels({ tr("Date"), tr("Day"), tr("Spot"), tr("Place"), tr("Hidden Item") });

    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Mirage6::generate);
    connect(ui->pushButtonLoadSave, &QPushButton::clicked, this, &Mirage6::loadSave);

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Mirage6::~Mirage6()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void Mirage6::generate()
{
    if (ui->textBoxValue->text().isEmpty())
    {
        QMessageBox msg(QMessageBox::Warning, tr("No daily value"), tr("Enter the daily value or load an OR/AS save"));
        msg.exec();
        return;
    }

    auto days = MirageSpot6::calendar(ui->textBoxValue->getUInt(), ui->textBoxTID->getUShort(), ui->spinBoxDays->value());
    const QString kinds[] = { tr("Crescent Isle"), tr("Mirage Forest"), tr("Mirage Cave"), tr("Mirage Island"), tr("Mirage Mountain") };

    QDate date = ui->dateEditStart->date();
    ui->tableWidget->setRowCount(static_cast<int>(days.size()));
    for (size_t i = 0; i < days.size(); i++)
    {
        const auto &day = days[i];
        int row = static_cast<int>(i);
        QString place = day.kind == MirageSpot6::Kind::CrescentIsle
            ? tr("%1 (zone %2)").arg(kinds[0]).arg(day.zone)
            : tr("%1 %2 (zone %3)").arg(kinds[static_cast<u8>(day.kind)]).arg(day.number).arg(day.zone);

        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(date.addDays(row).toString("yyyy-MM-dd")));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(row)));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(day.spot)));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(place));
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(Translator::getItem(day.item))));
    }
    ui->tableWidget->resizeColumnsToContents();
}

void Mirage6::loadSave()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select an OR/AS save (main)"));
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
    if (!save.valid || (save.version & Game::ORAS) == Game::None)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid save"), tr("Select a decrypted OR/AS save (0x76000 bytes)"));
        msg.exec();
        return;
    }

    ui->textBoxValue->setText(QString::number(save.mirageValue, 16).toUpper().rightJustified(8, '0'));
    ui->textBoxTID->setText(QString::number(save.tid));
    QMessageBox msg(QMessageBox::Information, tr("Save loaded"),
                    tr("Daily value %1, TID %2. Set Last Played to the day you last played this save, then press Generate.")
                        .arg(QString::number(save.mirageValue, 16).toUpper().rightJustified(8, '0'))
                        .arg(save.tid));
    msg.exec();
}
