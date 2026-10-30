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

#include "FishingGenerator7.hpp"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Gen7/FieldTimeline7.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/Util/Utilities.hpp>

namespace
{
    /**
     * @brief Hooked item from the roll: Sun/Moon and the USUM normal list are Pearl 0-49, Big Pearl 50-79, Pearl String 80-99; the USUM
     * rare list is Big Pearl 0-59, Pearl String 60-89, Comet Shard 90-99
     */
    u16 hookedItem(u8 roll, bool usumRare)
    {
        if (usumRare)
        {
            return roll < 60 ? 107 : roll < 90 ? 572 : 583;
        }
        return roll < 50 ? 106 : roll < 80 ? 107 : 572;
    }
}

FishingGenerator7::FishingGenerator7(u32 continueAdvance, u32 minFrame, u32 maxFrame, const FishingSpot7 &spot, const FieldModel7 &model,
                                     const EncounterArea7 &area, Lead lead, const Profile7 &profile, const StateFilter &filter) :
    spot(spot),
    model(model),
    area(area),
    profile(profile),
    filter(filter),
    continueAdvance(continueAdvance),
    minFrame(minFrame),
    maxFrame(maxFrame),
    menuFrame(0),
    lead(lead),
    boy(true),
    rare(false),
    showAll(false)
{
}

std::vector<FishingState7> FishingGenerator7::generate(u32 seed) const
{
    std::vector<FishingState7> states;
    u32 lastCast = 0xffffffff;
    for (u32 frame = std::max<u32>(1, minFrame); frame <= maxFrame; frame++)
    {
        FishingState7 state = generateAt(seed, frame);
        if (state.getAdvances() == lastCast && !states.empty() && states.back().getLastFrame() == frame - 1)
        {
            // Same cast advance as the frame before: the same chain, a wider press window
            states.back().setFrames(states.back().getFrame(), frame);
            continue;
        }
        lastCast = state.getAdvances();
        state.setFrames(frame, frame);

        bool keep;
        if (state.getResult() == FishingResult7::Pokemon)
        {
            keep = filter.compare(state);
        }
        else
        {
            keep = showAll;
        }
        if (keep)
        {
            states.emplace_back(state);
        }
        else
        {
            lastCast = 0xffffffff;
        }
    }
    return states;
}

FishingState7 FishingGenerator7::generateAt(u32 seed, u32 castFrame) const
{
    FieldTimeline7 timeline(seed, continueAdvance);
    u16 tsv = profile.getTID() ^ profile.getSID();
    bool usum = (profile.getVersion() & Game::USUM) != Game::None;

    for (u8 i = 0; i < model.loadDraws; i++)
    {
        timeline.next();
    }

    std::vector<FieldTimeline7::Model> models;
    u32 nextFidget = model.fidget;
    u32 castAdvance = 0;
    u32 biteFrame = 0;
    u32 hookFrame = 0;
    u32 leadFrame = 0;
    u32 itemFrame = 0;
    u8 wait = 0;
    u32 reelFrame = 0;
    u32 biteAdvance = 0;
    u8 biteRoll = 0;
    u32 hookAdvance = 0;
    u8 hookRoll = 0;
    u8 hookGap = rare ? 21 : 16;
    bool alwaysBites = lead == Lead::SuctionCups;

    // Frame order (handoff 3.8 and fishsim2): the hook / lead / item draws at the start of their frame, the model blinks, the cast
    // draws after the models of the cast frame, the fidget if due, the bite roll at the end of its frame
    for (u32 f = 1;; f++)
    {
        if (hookFrame != 0 && f == hookFrame)
        {
            hookAdvance = timeline.getAdvance();
            hookRoll = static_cast<u8>(timeline.next() % 100);
            if (hookRoll < 80)
            {
                leadFrame = f + 12;
            }
            else
            {
                itemFrame = f + 46;
            }
        }
        if (leadFrame != 0 && f == leadFrame)
        {
            break;
        }
        if (itemFrame != 0 && f == itemFrame)
        {
            u8 roll = static_cast<u8>(timeline.next() % 100);
            FishingState7 state(castAdvance, FishingResult7::Item, hookedItem(roll, usum && rare), area.getPokemon(0).getInfo());
            state.setWait(wait, reelFrame);
            state.setBite(biteAdvance, biteFrame, biteRoll);
            state.setHook(hookAdvance, hookFrame, hookRoll);
            return state;
        }

        if (f == 1)
        {
            models = timeline.rebuild(model.continueModels);
        }
        else
        {
            timeline.frame(models, model.persist, false);
        }

        if (f == castFrame)
        {
            castAdvance = timeline.getAdvance();
            wait = static_cast<u8>(timeline.next() % 60) + 30;
            u8 window = static_cast<u8>(timeline.next() % 15) + 15;
            biteFrame = f + spot.castDelay + wait;
            reelFrame = biteFrame + window;
            nextFidget = 0;
        }

        if (nextFidget != 0 && f == nextFidget && (menuFrame == 0 || f <= menuFrame))
        {
            u32 r = static_cast<u32>(timeline.next() % 90);
            u32 animation = static_cast<u32>(timeline.next() % 2);
            nextFidget = model.fidgetOnce ? 0 : f + r + (boy ? (animation ? 412 : 407) : (animation ? 407 : 402));
        }

        if (biteFrame != 0 && f == biteFrame)
        {
            if (alwaysBites)
            {
                biteAdvance = timeline.getAdvance();
                biteRoll = 0;
            }
            else
            {
                biteAdvance = timeline.getAdvance();
                biteRoll = static_cast<u8>(timeline.next() % 100);
                if (biteRoll >= 50)
                {
                    FishingState7 state(castAdvance, FishingResult7::NoBite, 0, area.getPokemon(0).getInfo());
                    state.setWait(wait, reelFrame);
                    state.setBite(biteAdvance, biteFrame, biteRoll);
                    return state;
                }
            }
            hookFrame = f + hookGap;
        }
    }

    // The lead frame: lead roll, slot, level, flute, Minior, 6 throwaways, the wild block (as Honey after its delay)
    u32 leadAdvance = timeline.getAdvance();
    u8 leadRoll = static_cast<u8>(timeline.next() % 100);
    u8 slot = area.getEncounterSlot(static_cast<u8>(timeline.next() % 100));
    const Slot &picked = area.getPokemon(slot);
    u16 specie = picked.getSpecie();
    u8 form = picked.getForm();
    const PersonalInfo *info = picked.getInfo();
    u8 range = picked.getMaxLevel() - picked.getMinLevel() + 1;
    u8 level = picked.getMinLevel() + static_cast<u8>(timeline.next() % range);
    if (lead == Lead::Pressure && leadRoll >= 50)
    {
        level = picked.getMaxLevel();
    }
    timeline.next(); // flute
    if (specie == 774)
    {
        form = static_cast<u8>(timeline.next() % 7);
    }
    for (int i = 0; i < 60; i++)
    {
        timeline.next();
    }

    u32 ec = static_cast<u32>(timeline.next());
    u32 pid = 0;
    u8 rolls = profile.getShinyCharm() ? 3 : 1;
    for (u8 roll = 0; roll < rolls; roll++)
    {
        pid = static_cast<u32>(timeline.next());
        if (Utilities::isShiny<false>(pid, tsv))
        {
            break;
        }
    }
    std::array<u8, 6> ivs;
    for (u8 &iv : ivs)
    {
        iv = static_cast<u8>(timeline.next() % 32);
    }
    u8 ability = static_cast<u8>(timeline.next() % 2);
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
    u16 item = 0;
    if (info && info->getItem(0) != info->getItem(1))
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

    FishingState7 state(castAdvance, leadAdvance, ec, pid, ivs, ability, gender, level, nature, Utilities::getShiny<false>(pid, tsv), slot,
                        item, specie, form, info);
    state.setWait(wait, reelFrame);
    state.setBite(biteAdvance, biteFrame, biteRoll);
    state.setHook(hookAdvance, hookFrame, hookRoll);
    state.setLeadRoll(leadRoll);
    return state;
}
