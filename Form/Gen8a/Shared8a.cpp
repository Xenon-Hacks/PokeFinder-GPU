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

#include "Shared8a.hpp"
#include <Core/Gen8a/SharedGenerator8a.hpp>
#include <Core/Gen8a/Templates8a.hpp>
#include <Core/Util/Translator.hpp>
#include <QCheckBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

static const QString settingPrefix = QStringLiteral("shared8a");

Shared8a::Shared8a(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(tr("Legends: Arceus Unown / Static / Gift"));

    auto *layout = new QVBoxLayout(this);
    auto *note = new QLabel(
        tr("Advances count shared-RNG draws from boot. Unown are made back to back when an area loads (the first load after boot: "
           "the start advance from the boot seed window). A static or gift after a cutscene lands about 140-180 draws after the "
           "state at the button press, and the field moves 5-48 draws a second (only the HOME menu stops it), so use the filters "
           "to find the advance of a Pokémon you caught from its IVs and nature. Static rows other than Chimchar (hikozaru_01), "
           "the Alolan Vulpix gift (event_092) and the kept starter (shoki3_*) are built the same way but were not caught live."));
    note->setWordWrap(true);
    layout->addWidget(note);

    auto *grid = new QGridLayout();
    seed = new QLineEdit();
    seed->setInputMask("HHHHHHHHHHHHHHHH");
    seed->setPlaceholderText(tr("boot seed"));
    initialAdvance = Widgets8a::spinBox(0, 100000000, 0);
    maxAdvances = Widgets8a::spinBox(1, 100000000, 100000);
    grid->addWidget(new QLabel(tr("Boot seed")), 0, 0);
    grid->addWidget(seed, 0, 1, 1, 2);
    grid->addWidget(new QLabel(tr("Initial advance")), 0, 3);
    grid->addWidget(initialAdvance, 0, 4);
    grid->addWidget(new QLabel(tr("Max advances")), 0, 5);
    grid->addWidget(maxAdvances, 0, 6);

    category = new QComboBox();
    category->addItems({ tr("Unown (area load)"), tr("Static"), tr("Gift") });
    unownForms = new QLineEdit("7,22");
    unownForms->setToolTip(tr("Uncaught Unown forms of the area in letter order, as numbers (A = 0 ... Z = 25, ! = 26, ? = 27)"));
    pokemon = new QComboBox();
    grid->addWidget(new QLabel(tr("Category")), 1, 0);
    grid->addWidget(category, 1, 1);
    grid->addWidget(new QLabel(tr("Unown forms")), 1, 2);
    grid->addWidget(unownForms, 1, 3);
    grid->addWidget(new QLabel(tr("Pokémon")), 1, 4);
    grid->addWidget(pokemon, 1, 5, 1, 2);

    grid->addWidget(new QLabel(tr("IV min")), 2, 0);
    ivMin = std::make_unique<Widgets8a::IVRow>(grid, 2, 1, 0);
    grid->addWidget(new QLabel(tr("IV max")), 3, 0);
    ivMax = std::make_unique<Widgets8a::IVRow>(grid, 3, 1, 31);
    nature = Widgets8a::natureBox(true);
    ability = Widgets8a::choiceBox({ tr("Slot 1"), tr("Slot 2") }, tr("Any"));
    shinyOnly = new QCheckBox(tr("Shiny only"));
    grid->addWidget(new QLabel(tr("Nature")), 4, 0);
    grid->addWidget(nature, 4, 1);
    grid->addWidget(new QLabel(tr("Ability")), 4, 2);
    grid->addWidget(ability, 4, 3);
    grid->addWidget(shinyOnly, 4, 4);
    auto *button = new QPushButton(tr("Generate"));
    grid->addWidget(button, 4, 6);
    layout->addLayout(grid);

    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({ tr("Advance"), tr("Pokémon"), tr("EC"), tr("PID"), tr("Shiny"), tr("IVs"), tr("Effort levels"),
                                       tr("Ability"), tr("Gender"), tr("Nature"), tr("Height"), tr("Weight"), tr("Advance after") });
    auto *table = new QTableView();
    table->setModel(model);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    layout->addWidget(table);

    connect(category, &QComboBox::currentIndexChanged, this, &Shared8a::categoryChanged);
    connect(button, &QPushButton::clicked, this, &Shared8a::generate);
    categoryChanged(0);

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Shared8a::~Shared8a()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();
}

void Shared8a::categoryChanged(int index)
{
    unownForms->setEnabled(index == 0);
    pokemon->setEnabled(index != 0);
    pokemon->clear();
    if (index == 0)
    {
        return;
    }
    const auto &rows = index == 1 ? Templates8a::statics() : Templates8a::gifts();
    for (size_t i = 0; i < rows.size(); i++)
    {
        const auto &row = rows[i];
        pokemon->addItem(
            QString("%1 Lv %2 (%3)").arg(QString::fromStdString(Templates8a::speciesName(row.species))).arg(row.level).arg(row.name),
            static_cast<int>(i));
    }
}

void Shared8a::generate()
{
    bool ok;
    u64 bootSeed = seed->text().toULongLong(&ok, 16);
    if (!ok)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Invalid seed"), tr("Enter the 16-digit boot seed"));
        msg.exec();
        return;
    }

    std::vector<Spec8a> specs;
    QStringList names;
    if (category->currentIndex() == 0)
    {
        for (const QString &part : unownForms->text().split(',', Qt::SkipEmptyParts))
        {
            int form = part.trimmed().toInt(&ok);
            if (!ok || form < 0 || form > 27)
            {
                QMessageBox msg(QMessageBox::Warning, tr("Invalid forms"), tr("Unown forms are 0-27, separated by commas"));
                msg.exec();
                return;
            }
            specs.emplace_back(Templates8a::unown(static_cast<u16>(form)));
            names.append(tr("Unown %1").arg(Templates8a::unownLetter(static_cast<u16>(form))));
        }
    }
    else
    {
        const auto &rows = category->currentIndex() == 1 ? Templates8a::statics() : Templates8a::gifts();
        const auto &row = rows[pokemon->currentData().toInt()];
        specs.emplace_back(Templates8a::spec(row));
        names.append(QString::fromStdString(Templates8a::speciesName(row.species)));
    }
    if (specs.empty())
    {
        return;
    }

    SharedFilter8a filter;
    filter.ivMin = ivMin->values();
    filter.ivMax = ivMax->values();
    int nat = nature->currentData().toInt();
    if (nat >= 0)
    {
        filter.natures.fill(false);
        filter.natures[nat] = true;
    }
    filter.ability = static_cast<s8>(ability->currentData().toInt());
    filter.shinyOnly = shinyOnly->isChecked();

    SharedGenerator8a generator(bootSeed, initialAdvance->value(), maxAdvances->value(), specs, filter);
    auto states = generator.generate();

    model->removeRows(0, model->rowCount());
    const auto &natures = Translator::getNatures();
    size_t shown = 0;
    for (const auto &result : states)
    {
        if (++shown > 100000)
        {
            break;
        }
        const State8a &s = result.state;
        model->appendRow({ Widgets8a::item(QString::number(result.advance)), Widgets8a::item(names[result.index]),
                           Widgets8a::item(Widgets8a::hex(s.ec, 8)), Widgets8a::item(Widgets8a::hex(s.pid, 8)),
                           Widgets8a::item(s.shiny ? tr("Yes") : tr("No")), Widgets8a::item(Widgets8a::ivText(s.ivs)),
                           Widgets8a::item(Widgets8a::effortText(s.ivs)),
                           Widgets8a::item(QString::number(s.ability + 1)), Widgets8a::item(Widgets8a::genderText(s.gender)),
                           Widgets8a::item(QString::fromStdString(natures[s.nature])), Widgets8a::item(QString::number(s.height)),
                           Widgets8a::item(QString::number(s.weight)), Widgets8a::item(QString::number(result.end)) });
    }
}
