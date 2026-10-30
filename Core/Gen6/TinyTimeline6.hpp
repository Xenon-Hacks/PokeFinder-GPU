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

#ifndef TINYTIMELINE6_HPP
#define TINYTIMELINE6_HPP

#include <Core/Gen6/EncounterArea6.hpp>
#include <Core/Gen6/Pokemon6.hpp>
#include <Core/RNG/TinyMT.hpp>
#include <array>
#include <vector>

class PersonalInfo;
enum class Game : u32;
enum class Lead : u8;

/**
 * @brief The TinyMT is the Gen 6 overworld RNG: it decides whether an encounter happens, the slot, the lead-ability check, the horde
 * hidden-ability member and held items, while the main MT decides the Pokemon itself. Everything here follows the live-verified draw
 * orders of the Gen 6 research (spec sections 6 and 7, 2026-10-30). Draw counts are exact; what happens between the draws (NPC
 * timers, weather) depends on the map and is modelled with \ref ZoneModel6 and the timers of \ref TinyTimeline6.
 */

/**
 * @brief Terrain class of a zone header (byte 0x20): outdoor maps run the 8 x 3 field-setup draws on every field resume
 */
enum class Terrain6 : u8
{
    Outdoor = 1,
    Cave = 2,
    Forest = 6
};

/**
 * @brief Per-zone data the TinyMT model needs
 */
struct ZoneModel6
{
    u16 zone; ///< Zone id (GARC index)
    u16 location; ///< Location id used by the encounter tables and Translator
    Terrain6 terrain; ///< Zone header terrain class
    u8 shadows; ///< Ceiling-shadow dust effects loaded in the zone (one rand(4) each on field resume)
    bool live; ///< Whether the pre-advance was measured live (otherwise from the rule and the zone header)

    /**
     * @brief Returns the TinyMT draws every field resume makes that do not depend on the party: rand(90), 8 x 3 on outdoor maps,
     * one rand(4) per shadow, rand(8) and rand(60)
     */
    constexpr u32 fieldResumeDraws() const
    {
        return 3 + (terrain == Terrain6::Outdoor ? 24 : 0) + shadows;
    }

    /**
     * @brief TinyMT draws between the Honey Use press and the horde trigger: 3 per party member (bag close) plus the field resume
     *
     * @param party Party size
     */
    constexpr u32 honeyPreAdvance(u8 party) const
    {
        return 3 * party + fieldResumeDraws();
    }

    /**
     * @brief Fixed TinyMT draws between the Sweet Scent press and the horde trigger: the field resume plus one rand(240) Pokemon-display
     * draw. Idle draws that fall in the ~200 ticks before the trigger come on top.
     */
    constexpr u32 sweetScentPreAdvance() const
    {
        return fieldResumeDraws() + 1;
    }
};

namespace ZoneModels6
{
    /**
     * @brief Returns the X/Y zones Honey and Sweet Scent hordes can be used in (44 zones)
     *
     * @param size Number of zones
     */
    const ZoneModel6 *getZones(int *size = nullptr);

    /**
     * @brief Returns the zone with the given id, nullptr if unknown
     */
    const ZoneModel6 *getZone(u16 zone);

    /**
     * @brief Returns the zones of a location (several for caves with more than one floor)
     */
    std::vector<const ZoneModel6 *> getZones(u16 location);
}

/**
 * @brief What the TinyMT decided on a horde trigger frame
 */
struct HordeDecision6
{
    u8 lead; ///< Lead-ability roll (Synchronize passes below 50, Cute Charm below 67)
    u8 slotRoll; ///< Horde group roll, 60 / 35 / 5
    u8 group; ///< Horde group 0-2 picked by the roll
    u8 haMember; ///< 1-5 for the hidden-ability member, 0 when the check (< 20) failed
    u8 extra; ///< Battle-environment "extra" roll
    std::array<u8, 5> flutes; ///< OR/AS flute-boost rolls per member (unused on X/Y)
    std::array<u8, 5> items; ///< Held-item rolls per member, drawn with \ref TinyTimeline6::hordeItems
};

/**
 * @brief What the TinyMT decided on a grass, cave, Surf or Friend Safari step
 */
struct StepDecision6
{
    u8 step; ///< X/Y per-step roll on grass and in caves (not on Surf or in the Safari)
    u8 lead; ///< Lead-ability roll
    u8 encounterRoll; ///< Encounter check roll, an encounter happens below the rate
    bool encounter; ///< Whether the step triggered an encounter
    u8 roamer; ///< X/Y roamer roll on grass encounters
    u8 slotRoll; ///< Slot roll
    u8 slot; ///< Slot picked from the table
    u8 flute; ///< OR/AS flute-boost roll
    u8 extra; ///< Battle-environment "extra" roll
};

/**
 * @brief Walks a TinyMT through the overworld draws of the Gen 6 games
 */
class TinyTimeline6
{
public:
    /**
     * @brief Timer-driven draws that fire on their own while the player stands still
     */
    enum class Timer : u8
    {
        BlinkStart, ///< Blink engine entry: rand decides the type-1 (> 0x55555555) or type-2 branch
        BlinkType1, ///< Cooldown 2 * rand(60) + 124 frames, back to the entry
        BlinkType2, ///< Cooldown 12 or 20 frames (rand > 0x55555555), to type 1
        Fidget, ///< Player idle animation: fires again after 2 * rand(90) + 720 frames (live X/Y; 3DSRNGTool uses 780)
        NPC ///< Loaded NPC or object idle timer: rand(90) + 300 frames
    };

    /**
     * @brief Construct a new TinyTimeline6 object
     *
     * @param rng TinyMT state to walk from
     * @param version Game version
     */
    TinyTimeline6(const TinyMT &rng, Game version);

    /**
     * @brief Registers a timer that is due at \p frame
     */
    void addTimer(u32 frame, Timer timer);

    /**
     * @brief Bag close after using an item: 3 draws per party member (friendship event table)
     *
     * @param party Party size
     */
    void bagClose(u8 party);

    /**
     * @brief Draws of a horde Pokemon's held items, one per member in order, at each member's EC + block
     *
     * @param infos Personal data of the 5 members
     * @param lead Lead ability (Compound Eyes raises the thresholds)
     *
     * @return Held items (0 = none) per member
     */
    std::array<u16, 5> hordeItems(const std::array<const PersonalInfo *, 5> &infos, Lead lead);

    /**
     * @brief Advances the timeline by \p frames frames, firing the registered timers
     *
     * @return Number of draws the timers made
     */
    u32 elapse(u32 frames);

    /**
     * @brief Field resume (menu closed or map loaded) draws of \p zone
     */
    void fieldResume(const ZoneModel6 &zone);

    /**
     * @brief Fishing delay draw: rand(7) at cast + 132 (X/Y) or Use + 197 (OR/AS)
     *
     * @return Frames from the delay draw to the bite, rand(7) * 30 + 60
     */
    u32 fishingDelay();

    /**
     * @brief Returns the TinyMT draws made so far
     */
    u32 getDraws() const
    {
        return draws;
    }

    /**
     * @brief Returns the current frame of the timeline
     */
    u32 getFrame() const
    {
        return frame;
    }

    /**
     * @brief Returns the current TinyMT
     */
    const TinyMT &getRNG() const
    {
        return rng;
    }

    /**
     * @brief Held-item draw of one Pokemon
     *
     * @param info Personal data of the species (species without items draw nothing)
     * @param lead Lead ability
     *
     * @return Held item, 0 for none
     */
    u16 heldItem(const PersonalInfo *info, Lead lead);

    /**
     * @brief Horde trigger frame draws: lead check, horde group, hidden-ability check and member, extra; on OR/AS also the 5 flute
     * boosts and one more draw. X/Y held items come later (\ref hordeItems); OR/AS items are drawn here into \p items.
     */
    HordeDecision6 hordeTrigger();

    /**
     * @brief One TinyMT output
     */
    u32 next();

    /**
     * @brief One TinyMT output reduced to [0, n)
     */
    u32 rand(u32 n);

    /**
     * @brief Rock Smash decision: 0 Pokemon, 1 item, 2 nothing
     */
    u8 rockSmash();

    /**
     * @brief Step draws of \p encounter on the current game. Grass covers caves too.
     *
     * @param encounter Encounter type (Grass, Surf, FriendSafari, flowers, rough terrain)
     * @param rate Encounter rate the check compares against (Friend Safari 13)
     */
    StepDecision6 step(Encounter6 encounter, u8 rate);

    /**
     * @brief Zone entry draws: the field resume plus the ambush occupancy rolls
     *
     * @param zone Zone entered
     * @param ambushObjects Ambush spots in the zone
     * @param occupancyRate Occupancy rate of the spots (50 or 30)
     *
     * @return Which ambush spots hold a Pokemon
     */
    std::vector<bool> zoneEntry(const ZoneModel6 &zone, u8 ambushObjects, u8 occupancyRate);

    /**
     * @brief Horde group picked by a roll: 60 / 35 / 5
     */
    static constexpr u8 hordeGroup(u8 roll)
    {
        return roll < 60 ? 0 : roll < 95 ? 1 : 2;
    }

    /**
     * @brief Slot picked by a roll for an encounter table
     *
     * @param encounter Encounter type
     * @param roll Roll in [0, 100)
     */
    static u8 slot(Encounter6 encounter, u8 roll);

private:
    struct Event
    {
        u32 frame;
        Timer timer;
    };

    TinyMT rng;
    std::vector<Event> events;
    u32 draws;
    u32 frame;
    bool oras;

    /**
     * @brief Fires the next due timer, consuming one draw
     */
    void fire(Event event);
};

/**
 * @brief After-battle Pokerus draws on the main MT (every battle end, including running away)
 */
namespace Pokerus6
{
    struct Result
    {
        u32 draws; ///< Main-MT draws the battle end made
        u8 member; ///< Infected party member (when infected)
        u8 strain; ///< Pokerus strain (when infected)
        bool infected;
    };

    /**
     * @brief Runs the after-battle Pokerus routine: rand(0x10000) infect check (0x4000, 0x8000 or 0xC000 hits), rand(3) spread, and
     * on a hit the member rand(count) and strain rand(256) picks
     *
     * @param rng Main MT positioned at the battle end
     * @param party Party size
     */
    template <class RNG>
    Result battleEnd(RNG &rng, u8 party)
    {
        Result result { 2, 0, 0, false };
        u32 infect = Gen6::rand(rng.next(), 0x10000);
        rng.next(); // spread rand(3), always drawn
        if (infect == 0x4000 || infect == 0x8000 || infect == 0xC000)
        {
            result.infected = true;
            result.member = Gen6::rand(rng.next(), party);
            result.draws++;
            do
            {
                result.strain = Gen6::rand(rng.next(), 256);
                result.draws++;
            } while ((result.strain & 7) == 0);
        }
        return result;
    }
}

/**
 * @brief Poke Radar patch layout (X/Y), 3DSRNGTool PokeRadar.cs, exact against 18 logged calls (live X)
 */
struct PokeRadar6
{
    struct Patch
    {
        u8 ring; ///< 0-3
        u8 direction; ///< 0 up, 1 down, 2 left, 3 right
        u8 location; ///< Position along the ring side
        u8 state; ///< 0 bad, 1 good, 2 shiny, 3 empty

        /**
         * @brief Column of the patch on the 9 x 9 grid, the player at (4, 4)
         */
        u8 x() const;

        /**
         * @brief Row of the patch on the 9 x 9 grid
         */
        u8 y() const;
    };

    u8 music; ///< Music roll: < 2 and > 49 play the radar jingles
    std::array<Patch, 5> patches; ///< 4 Pokemon patches by ring, then the empty patch

    /**
     * @brief Runs the Use burst: 3 x party draws, music, 4 rings, empty patch
     *
     * @param rng TinyMT at the Use press
     * @param party Party size
     * @param chain Chain length
     * @param boost Whether the shiny chance is boosted
     */
    PokeRadar6(TinyMT rng, u8 party, u8 chain, bool boost);

    /**
     * @brief Whether a shiny patch appeared
     */
    bool shiny() const;
};

/**
 * @brief DexNav search (OR/AS), 3DSRNGTool DexNav.cs; the patch search is exact live, the generate sequence is consistent but
 * was not traced call by call
 */
struct DexNav6
{
    bool success; ///< Whether a patch appeared
    s8 x; ///< Shaking patch column relative to the player (right positive)
    s8 y; ///< Shaking patch row relative to the player (down positive)
    u8 additionalDelay; ///< Visual checks are 2 frames apart; frames added before the patch appeared
    u8 slot; ///< Encounter slot
    u8 slotType; ///< 0 grass, 1 long grass, 2 Surf, 3 DexNav
    bool boost;
    u8 lead; ///< Lead-ability roll (< 50 passes)
    u8 levelBoost;
    u8 flute; ///< Flute boost roll
    bool hiddenAbility;
    u8 potential; ///< Perfect IVs, 0-3
    bool eggMove;
    u8 heldItem; ///< 0 item 1, 1 item 2, 3 none
    bool forcedShiny;

    /**
     * @brief Search settings
     */
    struct Settings
    {
        u8 encounterType; ///< 0 grass, 1 long grass, 2 Surf
        u16 searchLevel;
        u8 chain;
        bool activeSearch; ///< Searching from the DexNav app rather than a step trigger
        bool hasDexNav; ///< The DexNav-only slots match the encounter type
        bool shinyCharm;
        bool compoundEyes;
    };

    /**
     * @brief Runs the search from \p rng
     */
    DexNav6(TinyMT rng, const Settings &settings);
};

#endif // TINYTIMELINE6_HPP
