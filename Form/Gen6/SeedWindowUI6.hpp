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

#ifndef SEEDWINDOWUI6_HPP
#define SEEDWINDOWUI6_HPP

#include <Core/Gen6/SeedWindow6.hpp>
#include <Model/Gen6/StateModel6.hpp>
#include <QCheckBox>
#include <QDateTimeEdit>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QTableView>
#include <QThread>
#include <QTimeZone>

/**
 * @brief Shared seed window controls of the Gen 6 Static and Wild windows
 */
namespace SeedWindowUI6
{
    /**
     * @brief Console ms since 2000 of \p dateTime, read as the 3DS clock (no time zone)
     *
     * @param dateTime Console date and time
     *
     * @return ms since 2000-01-01 00:00:00.000
     */
    inline u64 consoleMs(const QDateTime &dateTime)
    {
        QDateTime utc(dateTime.date(), dateTime.time(), QTimeZone::UTC);
        QDateTime epoch(QDate(2000, 1, 1), QTime(0, 0), QTimeZone::UTC);
        return static_cast<u64>(epoch.msecsTo(utc));
    }

    /**
     * @brief Console date and time for \p ms since 2000
     *
     * @param ms Console ms since 2000
     *
     * @return Console date and time
     */
    inline QDateTime consoleTime(u64 ms)
    {
        QDateTime utc = QDateTime(QDate(2000, 1, 1), QTime(0, 0), QTimeZone::UTC).addMSecs(static_cast<qint64>(ms));
        return QDateTime(utc.date(), utc.time());
    }

    /**
     * @brief Sets up the controls and toggles them with the check box
     */
    inline void setup(QCheckBox *check, QDateTimeEdit *start, QDateTimeEdit *end, QWidget *seed, QLabel *odds, QTableView *table)
    {
        for (auto *edit : { start, end })
        {
            edit->setDisplayFormat("yyyy-MM-dd hh:mm:ss.zzz");
        }
        QDateTime now = QDateTime::currentDateTime();
        start->setDateTime(now.addSecs(-1));
        end->setDateTime(now.addSecs(1));

        check->setToolTip(QObject::tr("Generate every seed the title screen can set in this console clock window (Misc[0x00] + ms). "
                                      "The odds line shows how many of those seeds have a result that passes the filters."));
        odds->setWordWrap(true);

        auto toggle = [=](bool window) {
            start->setEnabled(window);
            end->setEnabled(window);
            seed->setEnabled(!window);
            odds->setVisible(window);
            table->setColumnHidden(StateModel6::seedColumn, !window);
            table->setColumnHidden(StateModel6::seedColumn + 1, !window);
        };
        QObject::connect(check, &QCheckBox::toggled, check, toggle);
        toggle(false);
    }

    /**
     * @brief Runs \p generator over the clock window and fills \p model
     *
     * @return false when the window is invalid or too large
     */
    template <class Generator>
    bool run(const Generator &generator, u32 saveSeed, u32 maxAdvances, QDateTimeEdit *start, QDateTimeEdit *end, QLabel *odds,
             StateModel6 *model)
    {
        u64 msStart = consoleMs(start->dateTime());
        u64 msEnd = consoleMs(end->dateTime());
        if (msEnd < msStart)
        {
            QMessageBox msg(QMessageBox::Warning, QObject::tr("Invalid clock"), QObject::tr("Clock end must be after clock start"));
            msg.exec();
            return false;
        }

        // Each seed generates every advance, keep the total in a range the CPU finishes quickly
        u64 work = (msEnd - msStart + 1) * (static_cast<u64>(maxAdvances) + 1);
        if (work > 500000000ULL)
        {
            QMessageBox msg(QMessageBox::Warning, QObject::tr("Window too large"),
                            QObject::tr("Seeds × advances is %1; narrow the clock window or the advances below 500,000,000").arg(work));
            msg.exec();
            return false;
        }

        QSettings setting;
        int threads = setting.value("settings/threads", QThread::idealThreadCount()).toInt();
        auto window = SeedWindow6::generate(generator, saveSeed, msStart, msEnd, threads);
        model->addItems(window.states);

        double percent = 100.0 * static_cast<double>(window.seedsWithHits) / static_cast<double>(window.seeds);
        odds->setText(QObject::tr("Seeds with a result: %1 / %2 (%3%)")
                          .arg(window.seedsWithHits)
                          .arg(window.seeds)
                          .arg(percent, 0, 'f', 2));
        return true;
    }
}

#endif // SEEDWINDOWUI6_HPP
