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

#include "IVCalc8a.hpp"
#include <Core/Gen8a/IVCalculator8a.hpp>
#include <Core/Util/Translator.hpp>
#include <QSettings>
#include <QVBoxLayout>

static const QString settingPrefix = QStringLiteral("ivCalc8a");

IVCalc8a::IVCalc8a(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(tr("Legends: Arceus IV calculator"));

    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(tr("Legends: Arceus never uses the IV itself. Each IV only sets the effort level a Pokémon starts with "
                                "(IV 0-19 = 0, 20-25 = 1, 26-30 = 2, 31 = 3) and grit adds to it. So the screen can only give the "
                                "range of each IV; 31 is the only exact value. Enter the stats, the effort levels, or both, and "
                                "leave the rest on Unknown. Grit is the number of levels you fed that stat (0 for a fresh catch). "
                                "Use the range to pick your Pokémon out of the result list of the Unown / Static / Gift window, "
                                "which shows the effort levels of every result."));
    intro->setWordWrap(true);
    layout->addWidget(intro);

    auto *top = new QGridLayout();
    species = new QComboBox();
    for (const auto &entry : IVCalculator8a::getEntries())
    {
        QString name = QString::fromStdString(Translator::getSpecie(entry.specie));
        if (entry.form)
        {
            name += QString(" (%1 %2)").arg(tr("form")).arg(entry.form);
        }
        species->addItem(name, QPoint(entry.specie, entry.form));
    }
    species->setEditable(true);
    species->setInsertPolicy(QComboBox::NoInsert);
    level = Widgets8a::spinBox(1, 100, 25);
    nature = Widgets8a::natureBox(false);
    flawless = Widgets8a::spinBox(0, 6, 0);
    flawless->setToolTip(tr("IVs the encounter guarantees as 31: 3 for alphas and for the legendary and mythical encounters"));
    top->addWidget(new QLabel(tr("Pokémon")), 0, 0);
    top->addWidget(species, 0, 1);
    top->addWidget(new QLabel(tr("Level")), 0, 2);
    top->addWidget(level, 0, 3);
    top->addWidget(new QLabel(tr("Nature")), 0, 4);
    top->addWidget(nature, 0, 5);
    top->addWidget(new QLabel(tr("Guaranteed 31s")), 0, 6);
    top->addWidget(flawless, 0, 7);
    layout->addLayout(top);

    auto *grid = new QGridLayout();
    const QStringList names = { tr("HP"), tr("Atk"), tr("Def"), tr("SpA"), tr("SpD"), tr("Spe") };
    grid->addWidget(new QLabel(tr("Stat")), 1, 0);
    grid->addWidget(new QLabel(tr("Effort level")), 2, 0);
    grid->addWidget(new QLabel(tr("Grit levels fed")), 3, 0);
    grid->addWidget(new QLabel(tr("IV")), 4, 0);
    for (int i = 0; i < 6; i++)
    {
        grid->addWidget(new QLabel(names[i]), 0, i + 1);
        stats[i] = Widgets8a::spinBox(-1, 999, -1);
        effort[i] = Widgets8a::spinBox(-1, 10, -1);
        grit[i] = Widgets8a::spinBox(-1, 10, 0);
        for (QSpinBox *box : { stats[i], effort[i], grit[i] })
        {
            box->setSpecialValueText(tr("Unknown"));
            connect(box, &QSpinBox::valueChanged, this, &IVCalc8a::update);
        }
        ranges[i] = new QLabel();
        ranges[i]->setTextInteractionFlags(Qt::TextSelectableByMouse);
        grid->addWidget(stats[i], 1, i + 1);
        grid->addWidget(effort[i], 2, i + 1);
        grid->addWidget(grit[i], 3, i + 1);
        grid->addWidget(ranges[i], 4, i + 1);
    }
    layout->addLayout(grid);

    note = new QLabel();
    note->setWordWrap(true);
    layout->addWidget(note);

    connect(species, &QComboBox::currentIndexChanged, this, &IVCalc8a::update);
    connect(nature, &QComboBox::currentIndexChanged, this, &IVCalc8a::update);
    connect(level, &QSpinBox::valueChanged, this, &IVCalc8a::update);
    connect(flawless, &QSpinBox::valueChanged, this, &IVCalc8a::update);
    update();

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

IVCalc8a::~IVCalc8a()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();
}

void IVCalc8a::update()
{
    QPoint key = species->currentData().toPoint();
    const auto *entry = IVCalculator8a::getEntry(static_cast<u16>(key.x()), static_cast<u8>(key.y()));
    if (!entry)
    {
        note->setText(tr("Pick a Pokémon from the list"));
        return;
    }
    std::array<int, 6> statValues;
    std::array<int, 6> effortValues;
    std::array<int, 6> gritValues;
    for (int i = 0; i < 6; i++)
    {
        statValues[i] = stats[i]->value();
        effortValues[i] = effort[i]->value();
        gritValues[i] = grit[i]->value();
    }
    u8 lv = static_cast<u8>(level->value());
    u8 nat = static_cast<u8>(nature->currentData().toInt());
    auto result = IVCalculator8a::analyze(entry->stats, lv, nat, effortValues, statValues, gritValues, flawless->value());

    for (int i = 0; i < 6; i++)
    {
        QStringList parts;
        for (const auto &range : result.stats[i].ranges)
        {
            parts.append(range.min == range.max ? QString::number(range.min) : QString("%1-%2").arg(range.min).arg(range.max));
        }
        ranges[i]->setText(parts.isEmpty() ? tr("none") : parts.join(", "));
        QStringList tip;
        for (u8 e = 0; e <= 10; e++)
        {
            tip.append(tr("effort %1: %2").arg(e).arg(IVCalculator8a::calcStat(i, entry->stats[i], lv, e, nat)));
        }
        stats[i]->setToolTip(tip.join("\n"));
    }

    QStringList lines;
    for (const auto &error : result.errors)
    {
        lines.append(QString::fromStdString(error));
    }
    if (!result.forced.empty())
    {
        lines.append(tr("With the guaranteed perfect IVs, every stat that can be 31 must be 31"));
    }
    note->setText(lines.join("\n"));
}
