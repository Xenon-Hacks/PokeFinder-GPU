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

#ifndef STATICTEMPLATE7_HPP
#define STATICTEMPLATE7_HPP

#include <Core/Parents/StaticTemplate.hpp>
#include <array>

/**
 * @brief How a Gen 7 static, gift or trade Pokemon is generated (handoff sections 4.1, 4.8, 4.9, 4.11)
 */
enum class StaticKind7 : u8
{
    Legend, ///< Lead rand(100), 3 frames of blinks, 60 throwaways, then EC, PID (shiny-locked), rand(6) picks, IVs, [ability], nature
    Totem, ///< Like Legend but the IVs are fixed except one rand(32) for Atk, and the nature is fixed
    LegendGift, ///< No lead, no throwaways: EC, PID (shiny-locked), 3 picks, 3 IVs, [ability rand(2)], nature (no roll with Synchronize)
    WildGift, ///< No lead, no throwaways: EC, PID (1 or 3 rolls), 6 IVs, ability rand(2), nature (no roll with Synchronize), gender
    Trade ///< rand(32) per unset IV (HP, Atk, Def, SpA, SpD, Spe), EC, PID shiny-locked against the trade's OT; the rest fixed
};

class StaticTemplate7 : public StaticTemplate
{
public:
    /**
     * @brief Construct a new StaticTemplate7 object
     *
     * @param version Games
     * @param specie Species
     * @param form Form
     * @param kind Generation shape
     * @param level Level
     * @param ivCount Guaranteed 31s drawn with rand(6) (legend blocks)
     * @param abilityDraw Whether the block draws the ability (rand(2)); otherwise \p ability is fixed with no draw
     * @param ability Fixed ability slot when there is no draw
     * @param nature Fixed nature (255 random)
     * @param gender Fixed gender (255 draws from the ratio; 2 genderless)
     * @param ivs Fixed IVs (255 random)
     * @param tradeTSV TSV of the trade's OT (the PID is locked against it)
     * @param measured Whether the block was run live by the research
     */
    constexpr StaticTemplate7(Game version, u16 specie, u8 form, StaticKind7 kind, u8 level, u8 ivCount, bool abilityDraw, u8 ability = 0,
                              u8 nature = 255, u8 gender = 255, const std::array<u8, 6> &ivs = { 255, 255, 255, 255, 255, 255 },
                              u16 tradeTSV = 0, bool measured = true) :
        StaticTemplate(version, specie, form, kind == StaticKind7::WildGift ? Shiny::Random : Shiny::Never, ability, gender, ivCount, level),
        ivs(ivs),
        tradeTSV(tradeTSV),
        kind(kind),
        nature(nature),
        abilityDraw(abilityDraw),
        measured(measured)
    {
    }

    bool getAbilityDraw() const
    {
        return abilityDraw;
    }

    const std::array<u8, 6> &getIVs() const
    {
        return ivs;
    }

    StaticKind7 getKind() const
    {
        return kind;
    }

    bool getMeasured() const
    {
        return measured;
    }

    u8 getNature() const
    {
        return nature;
    }

    u16 getTradeTSV() const
    {
        return tradeTSV;
    }

private:
    std::array<u8, 6> ivs;
    u16 tradeTSV;
    StaticKind7 kind;
    u8 nature;
    bool abilityDraw;
    bool measured;
};

#endif // STATICTEMPLATE7_HPP
