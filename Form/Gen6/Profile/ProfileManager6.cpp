/*
 * This file is part of PokéFinder
 * Copyright (C) 2017-2024 by Admiral_Fish, bumba, and EzPzStreamz
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

#include "ProfileManager6.hpp"
#include "ui_ProfileManager6.h"
#include <Core/Enum/Game.hpp>
#include <Core/Parents/ProfileLoader.hpp>
#include <Form/Gen6/Profile/ProfileEditor6.hpp>
#include <Model/Gen6/ProfileModel6.hpp>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QSettings>

ProfileManager6::ProfileManager6(QWidget *parent) : QWidget(parent), ui(new Ui::ProfileManager6)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose);

    model = new ProfileModel6(ui->tableView);
    model->addItems(ProfileLoader6::getProfiles(Game::Gen6));
    ui->tableView->setModel(model);
    ui->tableView->setAcceptDrops(true);
    ui->tableView->setDefaultDropAction(Qt::MoveAction);
    ui->tableView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tableView->setDragDropOverwriteMode(false);
    ui->tableView->setDragEnabled(true);
    ui->tableView->setDropIndicatorShown(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(ui->pushButtonNew, &QPushButton::clicked, this, &ProfileManager6::create);
    connect(ui->pushButtonEdit, &QPushButton::clicked, this, &ProfileManager6::edit);
    connect(ui->pushButtonDuplicate, &QPushButton::clicked, this, &ProfileManager6::duplicate);
    connect(ui->pushButtonDelete, &QPushButton::clicked, this, &ProfileManager6::remove);
    connect(ui->pushButtonOk, &QPushButton::clicked, this, &ProfileManager6::close);
    connect(model, &ProfileModel6::rowsMoved, this, [this] {
        ProfileLoader6::setProfiles(model->getModel());
        emit profilesChanged(6);
    });

    QSettings setting;
    if (setting.contains("profileManager6/geometry"))
    {
        this->restoreGeometry(setting.value("profileManager6/geometry").toByteArray());
    }
}

ProfileManager6::~ProfileManager6()
{
    QSettings setting;
    setting.setValue("profileManager6/geometry", this->saveGeometry());

    delete ui;
}

void ProfileManager6::create()
{
    std::unique_ptr<ProfileEditor6> dialog(new ProfileEditor6);
    if (dialog->exec() == QDialog::Accepted)
    {
        Profile6 profile = dialog->getProfile();
        ProfileLoader6::addProfile(profile);
        model->addItem(profile);
        emit profilesChanged(6);
    }
}

void ProfileManager6::duplicate()
{
    int row = ui->tableView->currentIndex().row();
    if (row < 0)
    {
        QMessageBox msg(QMessageBox::Warning, tr("No profile selected"), tr("Please select a profile"));
        msg.exec();
        return;
    }

    const Profile6 &profile = model->getItem(row);
    ProfileLoader6::addProfile(profile);
    model->addItem(profile);
    emit profilesChanged(6);
}

void ProfileManager6::edit()
{
    int row = ui->tableView->currentIndex().row();
    if (row < 0)
    {
        QMessageBox msg(QMessageBox::Warning, tr("No profile selected"), tr("Please select a profile"));
        msg.exec();
        return;
    }

    Profile6 original = model->getItem(row);
    std::unique_ptr<ProfileEditor6> dialog(new ProfileEditor6(original));
    if (dialog->exec() == QDialog::Accepted)
    {
        Profile6 update = dialog->getProfile();
        ProfileLoader6::updateProfile(update, original);
        model->updateItem(update, row);
        emit profilesChanged(6);
    }
}

void ProfileManager6::remove()
{
    int row = ui->tableView->currentIndex().row();
    if (row < 0)
    {
        QMessageBox msg(QMessageBox::Warning, tr("No profile selected"), tr("Please select a profile"));
        msg.exec();
        return;
    }

    QMessageBox msg(QMessageBox::Question, tr("Delete profile"), tr("Are you sure you wish to delete this profile?"),
                    QMessageBox::Yes | QMessageBox::No);
    if (msg.exec() == QMessageBox::Yes)
    {
        Profile6 profile = model->getItem(row);
        ProfileLoader6::removeProfile(profile);
        model->removeItem(row);
        emit profilesChanged(6);
    }
}
