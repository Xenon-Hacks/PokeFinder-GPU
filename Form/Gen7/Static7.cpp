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

#include "Static7.hpp"
#include "ui_Static7.h"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Gen7/Encounters7.hpp>
#include <Core/Gen7/Generators/StaticGenerator7.hpp>
#include <Core/Gen7/Profile7.hpp>
#include <Core/Gen7/StaticTemplate7.hpp>
#include <Core/Util/Translator.hpp>
#include <Form/Controls/Controls.hpp>
#include <Model/Gen7/StaticModel7.hpp>
#include <QSettings>

static const QString settingPrefix = QStringLiteral("static7");

Static7::Static7(QWidget *parent) : QWidget(parent), ui(new Ui::Static7), currentProfile(nullptr)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->profileDisplay->setup(settingPrefix, Game::Gen7);

    model = new StaticModel7(ui->tableView);
    ui->tableView->setModel(model);

    ui->textBoxSeed->setValues(InputType::Seed32Bit);
    ui->textBoxInitialAdvances->setValues(InputType::Advance32Bit);
    ui->textBoxMaxAdvances->setValues(InputType::Advance32Bit);

    ui->comboMenuLead->addAction(tr("None"), toInt(Lead::None));
    ui->comboMenuLead->addMenu(tr("Synchronize"), Translator::getNatures());

    ui->filter->disableControls(Controls::Height | Controls::Weight | Controls::Wild);

    ui->labelInitialAdvances->setToolTip(tr("Main-RNG advance of the first draw: the lead roll for statics (the script's final A press, "
                                            "about 2 advances after the last message), the EC for gifts, the first IV for trades"));
    ui->labelBlinkDraws->setToolTip(tr("Statics only: draws the on-screen models' blinks make in the 3 frames between the lead roll and the "
                                       "60 throwaways. 3 with only the player on screen (EC = lead + 64); 12-14 at the Lunala/Solgaleo altar"));
    ui->comboMenuLead->setToolTip(tr("Synchronize: statics fix the nature when the lead roll is 50 or more; gifts take it with no roll"));

    connect(ui->profileDisplay, &ProfileDisplay7::profileChanged, this, &Static7::profileChanged);
    connect(ui->profileDisplay, &ProfileDisplay7::profilesChanged, this, &Static7::profilesChanged);
    connect(ui->pushButtonGenerate, &QPushButton::clicked, this, &Static7::generate);
    connect(ui->comboBoxCategory, &QComboBox::currentIndexChanged, this, &Static7::categoryIndexChanged);
    connect(ui->comboBoxPokemon, &QComboBox::currentIndexChanged, this, &Static7::pokemonIndexChanged);
    connect(ui->filter, &Filter::showStatsChanged, model, &StaticModel7::setShowStats);

    updateProfiles();

    QSettings setting;
    setting.beginGroup(settingPrefix);
    if (setting.contains("geometry"))
    {
        this->restoreGeometry(setting.value("geometry").toByteArray());
    }
    setting.endGroup();
}

Static7::~Static7()
{
    QSettings setting;
    setting.beginGroup(settingPrefix);
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void Static7::updateProfiles()
{
    ui->profileDisplay->updateProfiles();
}

void Static7::categoryIndexChanged(int index)
{
    if (index >= 0 && currentProfile)
    {
        int size;
        const StaticTemplate7 *templates = Encounters7::getStaticEncounters(index, &size);

        ui->comboBoxPokemon->clear();
        for (int i = 0; i < size; i++)
        {
            if ((currentProfile->getVersion() & templates[i].getVersion()) != Game::None)
            {
                QString name = QString::fromStdString(Translator::getSpecie(templates[i].getSpecie(), templates[i].getForm()));
                if (!templates[i].getMeasured())
                {
                    name += tr(" [unverified]");
                }
                ui->comboBoxPokemon->addItem(name, i);
            }
        }
    }
}

void Static7::pokemonIndexChanged(int index)
{
    if (index >= 0)
    {
        const StaticTemplate7 *staticTemplate
            = Encounters7::getStaticEncounter(ui->comboBoxCategory->currentIndex(), ui->comboBoxPokemon->getCurrentInt());
        ui->spinBoxLevel->setValue(staticTemplate->getLevel());
        ui->spinBoxIVCount->setValue(staticTemplate->getIVCount());

        StaticKind7 kind = staticTemplate->getKind();
        bool isStatic = kind == StaticKind7::Legend || kind == StaticKind7::Totem;
        ui->spinBoxBlinkDraws->setEnabled(isStatic);
        ui->comboMenuLead->setEnabled(kind != StaticKind7::Trade);

        QString shape;
        switch (kind)
        {
        case StaticKind7::Legend:
            shape = staticTemplate->getAbilityDraw() ? tr("Static: lead, blinks, +60, legend block with ability draw")
                                                     : tr("Static: lead, blinks, +60, legend block (shiny-locked, fixed ability)");
            break;
        case StaticKind7::Totem:
            shape = tr("Totem: lead, blinks, +60, fixed IVs except Atk, fixed nature");
            break;
        case StaticKind7::LegendGift:
            shape = staticTemplate->getAbilityDraw() ? tr("Gift: EC first, legend block with ability draw") : tr("Gift: EC first, legend block, fixed ability");
            break;
        case StaticKind7::WildGift:
            shape = tr("Gift: EC first, wild-style block (can be shiny)");
            break;
        case StaticKind7::Trade:
            shape = tr("Trade: rand(32) per unset IV, EC, PID locked against the trade OT");
            break;
        }
        ui->labelShape->setText(shape);
    }
}

void Static7::profileChanged(const Profile7 &profile)
{
    currentProfile = &profile;
    categoryIndexChanged(ui->comboBoxCategory->currentIndex());
}

void Static7::generate()
{
    if (!currentProfile || !ui->filter->isValid() || ui->comboBoxPokemon->currentIndex() < 0)
    {
        return;
    }

    model->clearModel();

    u32 seed = ui->textBoxSeed->getUInt();
    u32 initialAdvances = ui->textBoxInitialAdvances->getUInt();
    u32 maxAdvances = ui->textBoxMaxAdvances->getUInt();
    auto lead = ui->comboMenuLead->getEnum<Lead>();
    const StaticTemplate7 *staticTemplate
        = Encounters7::getStaticEncounter(ui->comboBoxCategory->currentIndex(), ui->comboBoxPokemon->getCurrentInt());

    auto filter = ui->filter->getFilter<StateFilter>();
    StaticGenerator7 generator(initialAdvances, maxAdvances, static_cast<u8>(ui->spinBoxBlinkDraws->value()), lead, *staticTemplate,
                               *currentProfile, filter);
    model->addItems(generator.generate(seed));
}
