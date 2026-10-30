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

#include "TinyTimeline6.hpp"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Parents/PersonalInfo.hpp>
#include <algorithm>

// The 44 X/Y zones Honey and Sweet Scent hordes can be used in (spec 7.4). Live = the pre-advance was measured under GDB; the rest
// follow the rule from the zone header's terrain byte and the zone's ceiling-shadow objects.
constexpr std::array<ZoneModel6, 44> zones = {
    ZoneModel6 { 259, 12, Terrain6::Outdoor, 0, false }, // Route 2
    ZoneModel6 { 260, 16, Terrain6::Outdoor, 0, false }, // Route 3
    ZoneModel6 { 261, 20, Terrain6::Outdoor, 0, false }, // Route 4
    ZoneModel6 { 262, 28, Terrain6::Outdoor, 0, false }, // Route 5
    ZoneModel6 { 263, 34, Terrain6::Outdoor, 0, true }, // Route 6
    ZoneModel6 { 264, 38, Terrain6::Outdoor, 0, true }, // Route 7
    ZoneModel6 { 266, 42, Terrain6::Outdoor, 0, false }, // Route 8
    ZoneModel6 { 267, 46, Terrain6::Outdoor, 0, false }, // Route 9
    ZoneModel6 { 268, 50, Terrain6::Outdoor, 0, false }, // Route 10
    ZoneModel6 { 269, 54, Terrain6::Outdoor, 0, false }, // Route 11
    ZoneModel6 { 270, 62, Terrain6::Outdoor, 0, false }, // Route 12
    ZoneModel6 { 272, 66, Terrain6::Outdoor, 0, false }, // Route 13
    ZoneModel6 { 273, 68, Terrain6::Outdoor, 0, false }, // Route 14
    ZoneModel6 { 275, 74, Terrain6::Outdoor, 0, false }, // Route 15
    ZoneModel6 { 276, 78, Terrain6::Outdoor, 0, false }, // Route 16
    ZoneModel6 { 278, 84, Terrain6::Outdoor, 0, false }, // Route 17
    ZoneModel6 { 279, 88, Terrain6::Outdoor, 0, false }, // Route 18
    ZoneModel6 { 281, 92, Terrain6::Outdoor, 0, false }, // Route 19
    ZoneModel6 { 282, 96, Terrain6::Forest, 0, true }, // Route 20
    ZoneModel6 { 283, 100, Terrain6::Outdoor, 0, false }, // Route 21
    ZoneModel6 { 285, 102, Terrain6::Outdoor, 0, false }, // Route 22
    ZoneModel6 { 286, 14, Terrain6::Forest, 0, true }, // Santalune Forest
    ZoneModel6 { 303, 132, Terrain6::Cave, 0, false }, // Glittering Cave
    ZoneModel6 { 305, 56, Terrain6::Cave, 6, true }, // Reflection Cave (6 shadows)
    ZoneModel6 { 306, 56, Terrain6::Cave, 7, true }, // Reflection Cave (7 shadows)
    ZoneModel6 { 307, 56, Terrain6::Cave, 3, false }, // Reflection Cave (3 shadows)
    ZoneModel6 { 308, 56, Terrain6::Cave, 0, false }, // Reflection Cave bottom floor, quietest Honey spot found
    ZoneModel6 { 314, 82, Terrain6::Cave, 0, false }, // Frost Cavern
    ZoneModel6 { 315, 82, Terrain6::Cave, 0, false }, // Frost Cavern
    ZoneModel6 { 316, 82, Terrain6::Cave, 0, false }, // Frost Cavern
    ZoneModel6 { 317, 82, Terrain6::Cave, 0, false }, // Frost Cavern
    ZoneModel6 { 318, 98, Terrain6::Outdoor, 0, false }, // Pokemon Village
    ZoneModel6 { 322, 104, Terrain6::Cave, 0, false }, // Victory Road
    ZoneModel6 { 324, 104, Terrain6::Cave, 4, false }, // Victory Road (4 shadows)
    ZoneModel6 { 326, 104, Terrain6::Cave, 4, false }, // Victory Road (4 shadows)
    ZoneModel6 { 328, 104, Terrain6::Cave, 0, false }, // Victory Road
    ZoneModel6 { 334, 134, Terrain6::Cave, 0, false }, // Connecting Cave
    ZoneModel6 { 343, 140, Terrain6::Cave, 0, false }, // Terminus Cave
    ZoneModel6 { 344, 140, Terrain6::Cave, 5, true }, // Terminus Cave (5 shadows)
    ZoneModel6 { 345, 140, Terrain6::Cave, 4, false }, // Terminus Cave (4 shadows)
    ZoneModel6 { 346, 140, Terrain6::Cave, 0, true }, // Terminus Cave
    ZoneModel6 { 347, 140, Terrain6::Cave, 0, false }, // Terminus Cave
    ZoneModel6 { 348, 140, Terrain6::Cave, 0, false }, // Terminus Cave
    ZoneModel6 { 349, 142, Terrain6::Cave, 0, false }, // Lost Hotel
};

namespace ZoneModels6
{
    const ZoneModel6 *getZones(int *size)
    {
        if (size)
        {
            *size = static_cast<int>(zones.size());
        }
        return zones.data();
    }

    const ZoneModel6 *getZone(u16 zone)
    {
        auto it = std::ranges::find_if(zones, [zone](const ZoneModel6 &model) { return model.zone == zone; });
        return it != zones.end() ? &*it : nullptr;
    }

    std::vector<const ZoneModel6 *> getZones(u16 location)
    {
        std::vector<const ZoneModel6 *> result;
        for (const auto &zone : zones)
        {
            if (zone.location == location)
            {
                result.emplace_back(&zone);
            }
        }
        return result;
    }
}

TinyTimeline6::TinyTimeline6(const TinyMT &rng, Game version) : rng(rng), draws(0), frame(0), oras((version & Game::ORAS) != Game::None)
{
}

void TinyTimeline6::addTimer(u32 frame, Timer timer)
{
    events.emplace_back(frame, timer);
}

void TinyTimeline6::bagClose(u8 party)
{
    // Friendship/affection event table: rand(100), rand(256), rand(256) per member
    for (u8 i = 0; i < party; i++)
    {
        rand(100);
        rand(256);
        rand(256);
    }
}

std::array<u16, 5> TinyTimeline6::hordeItems(const std::array<const PersonalInfo *, 5> &infos, Lead lead)
{
    std::array<u16, 5> items;
    for (size_t i = 0; i < 5; i++)
    {
        items[i] = heldItem(infos[i], lead);
    }
    return items;
}

u32 TinyTimeline6::elapse(u32 frames)
{
    u32 target = frame + frames;
    u32 fired = 0;
    while (true)
    {
        auto it = std::ranges::min_element(events, [](const Event &left, const Event &right) { return left.frame < right.frame; });
        if (it == events.end() || it->frame > target)
        {
            break;
        }
        Event event = *it;
        events.erase(it);
        fire(event);
        fired++;
    }
    frame = target;
    return fired;
}

void TinyTimeline6::fire(Event event)
{
    frame = event.frame;
    u32 value = next();
    switch (event.timer)
    {
    case Timer::BlinkStart:
        addTimer(frame, value > 0x55555555 ? Timer::BlinkType1 : Timer::BlinkType2);
        break;
    case Timer::BlinkType1:
        addTimer(frame + Gen6::rand(value, 60) * 2 + 124, Timer::BlinkStart);
        break;
    case Timer::BlinkType2:
        addTimer(frame + (value > 0x55555555 ? 12 : 20), Timer::BlinkType1);
        break;
    case Timer::Fidget:
        // Live X/Y: 2 * rand(90) + 720 (100 of 102 calls on Y, 76 of 78 on X); 3DSRNGTool's 780 leaves 11-12 unexplained
        addTimer(frame + Gen6::rand(value, 90) * 2 + 720, Timer::Fidget);
        break;
    case Timer::NPC:
        addTimer(frame + Gen6::rand(value, 90) + 300, Timer::NPC);
        break;
    }
}

void TinyTimeline6::fieldResume(const ZoneModel6 &zone)
{
    rand(90); // player idle-timer reset
    if (zone.terrain == Terrain6::Outdoor)
    {
        for (int i = 0; i < 8; i++)
        {
            rand(1);
            rand(1);
            rand(360);
        }
    }
    for (u8 i = 0; i < zone.shadows; i++)
    {
        rand(4);
    }
    rand(8);
    rand(60);
}

u32 TinyTimeline6::fishingDelay()
{
    return rand(7) * 30 + 60;
}

u16 TinyTimeline6::heldItem(const PersonalInfo *info, Lead lead)
{
    // Only species that can hold an item draw
    if (info->getItem(0) == 0 && info->getItem(1) == 0 && info->getItem(2) == 0)
    {
        return 0;
    }

    u32 roll = rand(100);
    bool compoundEyes = lead == Lead::CompoundEyes;
    if (roll < (compoundEyes ? 60 : 50))
    {
        return info->getItem(0);
    }
    if (roll < (compoundEyes ? 80 : 55))
    {
        return info->getItem(1);
    }
    if (roll < (compoundEyes ? 85 : 56))
    {
        return info->getItem(2);
    }
    return 0;
}

HordeDecision6 TinyTimeline6::hordeTrigger()
{
    HordeDecision6 decision {};
    decision.lead = rand(100);
    decision.slotRoll = rand(100);
    decision.group = hordeGroup(decision.slotRoll);
    decision.haMember = rand(100) < 20 ? rand(5) + 1 : 0;
    if (oras)
    {
        for (u8 &flute : decision.flutes)
        {
            flute = rand(100);
        }
        next(); // one more draw about 10 frames later
        for (u8 &item : decision.items)
        {
            item = rand(100);
        }
    }
    else
    {
        decision.extra = rand(100);
    }
    return decision;
}

u32 TinyTimeline6::next()
{
    draws++;
    return rng.next();
}

u32 TinyTimeline6::rand(u32 n)
{
    return Gen6::rand(next(), n);
}

u8 TinyTimeline6::rockSmash()
{
    return rand(3);
}

u8 TinyTimeline6::slot(Encounter6 encounter, u8 roll)
{
    auto pick = [roll](const auto &thresholds) {
        u8 slot = 0;
        for (u8 threshold : thresholds)
        {
            if (roll < threshold)
            {
                return slot;
            }
            slot++;
        }
        return slot;
    };

    switch (encounter)
    {
    case Encounter6::Surf:
        return pick(std::array<u8, 5> { 50, 80, 95, 99, 100 });
    case Encounter6::OldRod:
    case Encounter6::GoodRod:
    case Encounter6::SuperRod:
    case Encounter6::HordeHoney:
    case Encounter6::HordeSweetScent:
    case Encounter6::HordeNatural:
        return pick(std::array<u8, 3> { 60, 95, 100 });
    case Encounter6::RockSmash:
        return pick(std::array<u8, 5> { 60, 95, 99, 100, 100 });
    case Encounter6::FriendSafari:
        return pick(std::array<u8, 3> { 34, 67, 100 });
    default:
        // 12-entry grass-type tables: cumulative 20/20/10/10/10/10/5/5/4/4/1/1
        return pick(std::array<u8, 12> { 20, 40, 50, 60, 70, 80, 85, 90, 94, 98, 99, 100 });
    }
}

StepDecision6 TinyTimeline6::step(Encounter6 encounter, u8 rate)
{
    StepDecision6 decision {};
    bool surf = encounter == Encounter6::Surf;
    bool safari = encounter == Encounter6::FriendSafari;
    if (!oras && !surf && !safari)
    {
        decision.step = rand(100);
    }
    decision.lead = rand(100);
    decision.encounterRoll = rand(100);
    decision.encounter = decision.encounterRoll < rate;
    if (decision.encounter)
    {
        if (!oras && !surf && !safari)
        {
            decision.roamer = rand(100);
        }
        decision.slotRoll = rand(100);
        decision.slot = slot(encounter, decision.slotRoll);
        if (oras)
        {
            decision.flute = rand(100);
        }
        decision.extra = rand(100);
    }
    return decision;
}

std::vector<bool> TinyTimeline6::zoneEntry(const ZoneModel6 &zone, u8 ambushObjects, u8 occupancyRate)
{
    rand(90);
    if (zone.terrain == Terrain6::Outdoor)
    {
        for (int i = 0; i < 8; i++)
        {
            rand(1);
            rand(1);
            rand(360);
        }
    }
    rand(8);

    std::vector<bool> occupied(ambushObjects);
    for (u8 i = 0; i < ambushObjects; i++)
    {
        occupied[i] = rand(100) < occupancyRate;
    }
    rand(60);
    return occupied;
}

u8 PokeRadar6::Patch::x() const
{
    switch (direction)
    {
    case 0:
    case 1:
        return 3 - ring + location;
    case 2:
        return 3 - ring;
    default:
        return 5 + ring;
    }
}

u8 PokeRadar6::Patch::y() const
{
    switch (direction)
    {
    case 0:
        return 3 - ring;
    case 1:
        return 5 + ring;
    default:
        return 3 - ring + location;
    }
}

PokeRadar6::PokeRadar6(TinyMT rng, u8 party, u8 chain, bool boost) : patches {}
{
    auto rand = [&rng](u32 n) { return static_cast<u8>(Gen6::rand(rng.next(), n)); };
    constexpr std::array<u8, 4> goodRate = { 23, 43, 63, 83 };

    for (int i = 0; i < 3 * party; i++)
    {
        rng.next();
    }
    music = rand(100);
    boost &= music >= 50;

    for (u8 ring = 0; ring < 4; ring++)
    {
        Patch &patch = patches[ring];
        patch.ring = ring;
        patch.direction = rand(4);
        patch.location = rand(ring * 2 + 3);
        if (rand(100) < goodRate[ring])
        {
            rng.next();
            u64 chance = (boost || chain >= 40) ? 100 : (8100 - static_cast<u32>(chain) * 200);
            patch.state = static_cast<u64>(rng.next()) * chance <= 0xffffffff ? 2 : 1;
        }
    }

    Patch &empty = patches[4];
    empty.ring = rand(3);
    empty.direction = rand(4);
    empty.location = rand(empty.ring * 2 + 3);
    empty.state = 3;
}

bool PokeRadar6::shiny() const
{
    return std::ranges::any_of(patches, [](const Patch &patch) { return patch.state == 2; });
}

DexNav6::DexNav6(TinyMT rng, const Settings &settings) :
    success(false),
    x(0),
    y(0),
    additionalDelay(0),
    slot(0),
    slotType(settings.encounterType),
    boost(false),
    lead(0),
    levelBoost(0),
    flute(0),
    hiddenAbility(false),
    potential(0),
    eggMove(false),
    heldItem(3),
    forcedShiny(false)
{
    auto rand = [&rng](u32 n) { return Gen6::rand(rng.next(), n); };

    constexpr std::array<u8, 5> gradeRange = { 5, 10, 25, 50, 100 };
    constexpr std::array<s8, 4> slotNum = { 12, 12, 5, 3 };
    constexpr std::array<u8, 6> haRate = { 0, 0, 5, 15, 20, 25 };
    constexpr std::array<u8, 18> ivRate = { 0, 0, 0, 10, 0, 0, 15, 10, 0, 20, 15, 5, 15, 20, 5, 10, 25, 10 };
    constexpr std::array<u8, 6> eggMoveRate = { 20, 50, 55, 60, 65, 80 };
    constexpr std::array<u8, 12> heldItemRate = { 40, 10, 40, 10, 45, 15, 50, 20, 50, 20, 50, 30 };

    u8 grade = 5;
    for (u8 g = 0; g < 5; g++)
    {
        if (settings.searchLevel < gradeRange[g])
        {
            grade = g;
            break;
        }
    }

    bool triggered = settings.activeSearch;
    if (!triggered)
    {
        rng.next(); // lead
        rng.next(); // encounter rate
        triggered = rand(100) < 50;
    }
    if (!triggered)
    {
        return;
    }

    for (additionalDelay = 0; additionalDelay < 8; additionalDelay += 2)
    {
        // Up to 5 patch attempts per visual check; the first attempt always finds a patch (3DSRNGTool to-do)
        switch (rand(4))
        {
        case 0:
            x = -9 + static_cast<s8>(rand(18));
            y = -9 + static_cast<s8>(rand(3));
            break;
        case 1:
            x = -9 + static_cast<s8>(rand(3));
            y = -7 + static_cast<s8>(rand(14));
            break;
        case 2:
            x = 7 + static_cast<s8>(rand(3));
            y = -7 + static_cast<s8>(rand(14));
            break;
        default:
            x = -9 + static_cast<s8>(rand(18));
            y = 7 + static_cast<s8>(rand(3));
            break;
        }

        // Generate
        slotType = (rand(100) < 30 && settings.hasDexNav) ? 3 : settings.encounterType;
        boost = (settings.chain > 0 && (settings.chain + 1) % 5 == 0) || rand(100) < 4;
        lead = rand(100);
        s8 pick = 0;
        if (!settings.activeSearch)
        {
            for (pick = slotNum[slotType] - 1; pick >= 0; pick--)
            {
                if (rand(100) < 30)
                {
                    break;
                }
            }
        }
        slot = pick < 0 ? 0 : pick;
        rng.next();
        levelBoost = settings.chain / 5 + (boost ? 10 : 0);
        flute = rand(100);
        hiddenAbility = rand(100) < haRate[grade];
        int index;
        for (index = 2; index >= 0; index--)
        {
            if (rand(100) < ivRate[3 * grade + index])
            {
                break;
            }
        }
        index += boost ? 2 : 1;
        potential = static_cast<u8>(std::min(3, index));
        eggMove = rand(100) < eggMoveRate[grade] || boost;
        int tmp = static_cast<int>(rand(100));
        for (index = 0; index < 2; index++)
        {
            tmp -= heldItemRate[grade * 2 + index];
            if (settings.compoundEyes)
            {
                tmp -= 5;
            }
            if (tmp < 0)
            {
                break;
            }
        }
        heldItem = index >= 2 ? 3 : index;
        int checks = (settings.shinyCharm ? 3 : 1) + (boost ? 4 : 0) + (settings.chain == 49 ? 5 : settings.chain == 99 ? 10 : 0);
        u32 target = settings.searchLevel > 200 ? settings.searchLevel + 600
            : settings.searchLevel > 100  ? 2 * settings.searchLevel + 400
                                          : 6 * settings.searchLevel;
        for (int i = 0; i < checks; i++)
        {
            if (rand(10000) * 100 < target)
            {
                forcedShiny = true;
            }
        }

        // Post check: 2 draws per patch, the first passes (3DSRNGTool to-do)
        rng.next();
        rng.next();
        success = true;
        return;
    }
}
