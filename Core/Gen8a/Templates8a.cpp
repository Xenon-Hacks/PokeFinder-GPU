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

#include "Templates8a.hpp"
#include <Core/Util/Translator.hpp>

#include "Templates8aData.inc"

namespace Templates8a
{
    Spec8a unown(u16 form)
    {
        Spec8a spec;
        spec.pid = Spec8a::neverShiny;
        spec.species = 201;
        spec.form = form;
        spec.level = 25;
        spec.gender = 2;
        spec.ability = 0xFF;
        spec.height = 0x80;
        spec.weight = 0x100;
        spec.genderRatio = 255;
        spec.formCount = 28;
        return spec;
    }

    const std::vector<RawTemplate8a> &statics()
    {
        static const std::vector<RawTemplate8a> rows(std::begin(::statics), std::end(::statics));
        return rows;
    }

    const std::vector<RawTemplate8a> &gifts()
    {
        static const std::vector<RawTemplate8a> rows(std::begin(::gifts), std::end(::gifts));
        return rows;
    }

    Spec8a spec(const RawTemplate8a &row)
    {
        Spec8a spec;
        spec.pid = row.shinyLocked ? Spec8a::neverShiny : Spec8a::randomPID;
        spec.species = row.species;
        spec.form = row.form;
        spec.level = row.level;
        spec.gender = row.gender;
        spec.nature = row.nature;
        spec.ability = 0xFF;
        spec.rolls = 1;
        for (int i = 0; i < 6; i++)
        {
            spec.ivs[i] = row.ivs[i];
        }
        spec.flawless = row.flawless;
        spec.height = row.height;
        spec.weight = row.weight;
        spec.genderRatio = row.genderRatio;
        return spec;
    }

    std::string speciesName(u16 species)
    {
        static const char *hisui[] = { "Wyrdeer", "Kleavor", "Ursaluna", "Basculegion", "Sneasler", "Overqwil", "Enamorus" };
        if (species >= 899 && species <= 905)
        {
            return hisui[species - 899];
        }
        return Translator::getSpecie(species);
    }

    char unownLetter(u16 form)
    {
        return form < 26 ? static_cast<char>('A' + form) : form == 26 ? '!' : '?';
    }
}
