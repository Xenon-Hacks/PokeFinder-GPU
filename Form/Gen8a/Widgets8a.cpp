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

#include "Widgets8a.hpp"
#include <Core/Util/Translator.hpp>
#include <QStandardItem>

namespace Widgets8a
{
    IVRow::IVRow(QGridLayout *layout, int row, int column, int value)
    {
        static const char *names[] = { "HP", "Atk", "Def", "SpA", "SpD", "Spe" };
        for (int i = 0; i < 6; i++)
        {
            boxes[i] = spinBox(0, 31, value);
            boxes[i]->setToolTip(QString(names[i]));
            boxes[i]->setPrefix(QString(names[i]) + " ");
            layout->addWidget(boxes[i], row, column + i);
        }
    }

    std::array<u8, 6> IVRow::values() const
    {
        std::array<u8, 6> result;
        for (int i = 0; i < 6; i++)
        {
            result[i] = static_cast<u8>(boxes[i]->value());
        }
        return result;
    }

    QComboBox *natureBox(bool any)
    {
        auto *box = new QComboBox();
        if (any)
        {
            box->addItem(QObject::tr("Any"), -1);
        }
        const auto &natures = Translator::getNatures();
        for (int i = 0; i < 25; i++)
        {
            box->addItem(QString::fromStdString(natures[i]), i);
        }
        return box;
    }

    QComboBox *choiceBox(const QStringList &entries, const QString &unknown)
    {
        auto *box = new QComboBox();
        box->addItem(unknown, -1);
        for (int i = 0; i < entries.size(); i++)
        {
            box->addItem(entries[i], i);
        }
        return box;
    }

    QSpinBox *spinBox(int min, int max, int value)
    {
        auto *box = new QSpinBox();
        box->setRange(min, max);
        box->setValue(value);
        return box;
    }

    QString hex(u64 value, int width)
    {
        return QString::number(value, 16).toUpper().rightJustified(width, '0');
    }

    QStandardItem *item(const QString &text)
    {
        auto *result = new QStandardItem(text);
        result->setEditable(false);
        return result;
    }

    QString ivText(const std::array<u8, 6> &ivs)
    {
        QStringList parts;
        for (u8 iv : ivs)
        {
            parts.append(QString::number(iv));
        }
        return parts.join('/');
    }

    QString genderText(u8 gender)
    {
        return gender == 0 ? "M" : gender == 1 ? "F" : "-";
    }
}
