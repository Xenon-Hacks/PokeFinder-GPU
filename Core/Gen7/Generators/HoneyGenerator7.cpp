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

#include "HoneyGenerator7.hpp"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Gen7/FieldTimeline7.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/Util/Utilities.hpp>

HoneyGenerator7::HoneyGenerator7(u32 continueAdvance, u32 minFrame, u32 maxFrame, const Spot7 &spot, const EncounterArea7 &area, Lead lead,
                                 const Profile7 &profile, const StateFilter &filter) :
    spot(spot),
    area(area),
    profile(profile),
    filter(filter),
    continueAdvance(continueAdvance),
    minFrame(minFrame),
    maxFrame(maxFrame),
    menuFrame(0),
    lead(lead),
    boy(true),
    persistFresh(false),
    rain(spot.flags & Spot7::Rain)
{
}

std::vector<HoneyState7> HoneyGenerator7::generate(u32 seed) const
{
    auto advances = FieldTimeline7::continueAdvances(seed, continueAdvance, spot, maxFrame, menuFrame, boy, rain);

    std::vector<HoneyState7> states;
    u32 lastBag = 0xffffffff;
    for (u32 frame = minFrame; frame <= maxFrame && frame < advances.size(); frame++)
    {
        u32 bag = advances[frame];
        if (bag == lastBag && !states.empty() && states.back().getLastFrame() == frame - 1)
        {
            // Same bag advance as the frame before: the same result, a wider press window
            states.back().setFrames(states.back().getFrame(), frame);
            continue;
        }
        lastBag = bag;

        HoneyState7 state = generateAt(seed, bag);
        state.setFrames(frame, frame);
        if (filter.compare(state))
        {
            states.emplace_back(state);
        }
        else
        {
            // Keep merging frames of a filtered-out result without listing it
            lastBag = 0xffffffff;
        }
    }
    return states;
}

HoneyState7 HoneyGenerator7::generateAt(u32 seed, u32 bagAdvance) const
{
    FieldTimeline7 timeline(seed, bagAdvance);
    u16 tsv = profile.getTID() ^ profile.getSID();
    bool usum = (profile.getVersion() & Game::USUM) != Game::None;

    // Use: pre-rebuild draws (USUM 1, rain-sound or walker pair 2), then the rebuild frame
    for (u8 i = 0; i < spot.honeyPreDraws; i++)
    {
        timeline.next();
    }

    std::vector<FieldTimeline7::Model> models;
    u32 persist = spot.honeyPersist;
    if (persistFresh)
    {
        // Older reading: the rebuild draws F values, then the persistent models start from a fresh state
        for (u8 i = 0; i < spot.honeyModels; i++)
        {
            timeline.next();
        }
        models.resize(spot.honeyModels);
    }
    else
    {
        models = timeline.rebuild(spot.honeyModels);
    }
    for (u8 f = 1; f <= spot.honeyDelay; f++)
    {
        timeline.frame(models, persist, rain && ((f - 1) % 2 == 0));
    }

    // Special table (Island Scan / UB / Necrozma): the scan roll at the lead frame; on a pass 7 frames of blinks, the lead,
    // 3 frames, then the throwaways and the target (handoff 4.7)
    bool specialHit = false;
    if (special.specie != 0 && special.rate != 0)
    {
        u8 roll = static_cast<u8>(timeline.next() % 100);
        specialHit = roll < special.rate;
    }

    const PersonalInfo *info;
    u16 specie;
    u8 form;
    u8 level;
    u8 slot = 0;
    u8 leadRoll;
    u32 leadAdvance;
    bool legend = false;
    u8 perfect = 0;
    if (specialHit)
    {
        for (int f = 0; f < 7; f++)
        {
            timeline.frame(models, persist, false);
        }
        leadAdvance = timeline.getAdvance();
        leadRoll = static_cast<u8>(timeline.next() % 100);
        for (int f = 0; f < 3; f++)
        {
            timeline.frame(models, persist, false);
        }
        specie = special.specie;
        form = special.form;
        level = special.level;
        info = PersonalLoader::getPersonal(profile.getVersion(), specie, form);
        legend = special.legend;
        perfect = special.perfectIVs;
    }
    else
    {
        leadAdvance = timeline.getAdvance();
        leadRoll = static_cast<u8>(timeline.next() % 100);
        slot = area.getEncounterSlot(static_cast<u8>(timeline.next() % 100));
        const Slot &picked = area.getPokemon(slot);
        specie = picked.getSpecie();
        form = picked.getForm();
        info = picked.getInfo();

        u8 range = picked.getMaxLevel() - picked.getMinLevel() + 1;
        level = picked.getMinLevel() + static_cast<u8>(timeline.next() % range);
        if (lead == Lead::Pressure && leadRoll >= 50)
        {
            level = picked.getMaxLevel();
        }
        timeline.next(); // flute
        if (specie == 774)
        {
            form = static_cast<u8>(timeline.next() % 7); // Minior core form
        }
    }

    // 6 throwaway Pokemon of 10 draws
    for (int i = 0; i < 60; i++)
    {
        timeline.next();
    }

    // The block (handoff 4.1): EC, PID, [perfect IV picks], IVs, [ability], nature, gender
    u32 ec = static_cast<u32>(timeline.next());
    u32 pid;
    if (legend)
    {
        pid = static_cast<u32>(timeline.next());
        if (Utilities::isShiny<false>(pid, tsv))
        {
            pid ^= 0x10000000;
        }
    }
    else
    {
        u8 rolls = profile.getShinyCharm() ? 3 : 1;
        for (u8 roll = 0; roll < rolls; roll++)
        {
            pid = static_cast<u32>(timeline.next());
            if (Utilities::isShiny<false>(pid, tsv))
            {
                break;
            }
        }
    }

    std::array<u8, 6> ivs = { 255, 255, 255, 255, 255, 255 };
    for (u8 i = 0; i < perfect;)
    {
        u8 index = static_cast<u8>(timeline.next() % 6);
        if (ivs[index] == 255)
        {
            ivs[index] = 31;
            i++;
        }
    }
    for (u8 &iv : ivs)
    {
        if (iv == 255)
        {
            iv = static_cast<u8>(timeline.next() % 32);
        }
    }

    u8 ability = legend ? 0 : static_cast<u8>(timeline.next() % 2);
    u8 nature;
    if (lead <= Lead::SynchronizeEnd && leadRoll >= 50)
    {
        nature = toInt(lead);
    }
    else
    {
        nature = static_cast<u8>(timeline.next() % 25);
    }

    u8 gender;
    u8 ratio = info ? info->getGender() : 255;
    if (ratio == 255)
    {
        gender = 2;
    }
    else if (ratio == 254)
    {
        gender = 1;
    }
    else if (ratio == 0)
    {
        gender = 0;
    }
    else if ((lead == Lead::CuteCharmF || lead == Lead::CuteCharmM) && leadRoll < 67)
    {
        gender = lead == Lead::CuteCharmF ? 0 : 1;
    }
    else
    {
        gender = (timeline.next() % 252) + 1 < ratio ? 1 : 0;
    }

    // Held item only when the species' two wild items differ
    u16 item = 0;
    if (!legend && info && info->getItem(0) != info->getItem(1))
    {
        u8 roll = static_cast<u8>(timeline.next() % 100);
        bool compoundEyes = lead == Lead::CompoundEyes;
        if (roll < (compoundEyes ? 60 : 50))
        {
            item = info->getItem(0);
        }
        else if (roll < (compoundEyes ? 80 : 55))
        {
            item = info->getItem(1);
        }
    }

    u8 shiny = Utilities::getShiny<false>(pid, tsv);
    HoneyState7 state(bagAdvance, leadAdvance, ec, pid, ivs, ability, gender, level, nature, shiny, slot, item, specie, form, info);
    state.setLeadRoll(leadRoll);
    state.setSpecial(specialHit);
    return state;
}
