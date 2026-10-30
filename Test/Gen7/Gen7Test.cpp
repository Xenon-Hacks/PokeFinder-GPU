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

#include "Gen7Test.hpp"
#include <Core/Enum/Game.hpp>
#include <Core/Gen7/EncounterArea7.hpp>
#include <Core/Gen7/Encounters7.hpp>
#include <Core/Gen7/Generators/EggGenerator7.hpp>
#include <Core/Gen7/Generators/EventGenerator7.hpp>
#include <Core/Gen7/Generators/PelagoGenerator7.hpp>
#include <Core/Gen7/Save7.hpp>
#include <Core/Gen7/States/EggState7.hpp>
#include <Core/Gen7/WC7.hpp>
#include <Core/Parents/Daycare.hpp>
#include <vector>
#include <Core/Gen7/Generators/StaticGenerator7.hpp>
#include <Core/Gen7/StaticTemplate7.hpp>
#include <Core/Gen7/FieldTimeline7.hpp>
#include <Core/Gen7/Gen7.hpp>
#include <Core/Gen7/Generators/HoneyGenerator7.hpp>
#include <Core/Gen7/Profile7.hpp>
#include <Core/Gen7/Spot7.hpp>
#include <Core/Gen7/Timer7.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Parents/Filters/StateFilter.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/Gen7/Generators/IDGenerator7.hpp>
#include <Core/Gen7/Searchers/NeedleSearcher7.hpp>
#include <Core/Parents/Filters/IDFilter.hpp>
#include <Core/RNG/SFMT.hpp>
#include <Core/Util/OpenCL.hpp>
#include <QTest>
#include <chrono>
#include <cstdlib>
#include <thread>

static std::vector<u32> runSearch(NeedleSearcher7 &searcher, int threads, bool gpu, u32 start, u64 count)
{
    searcher.startSearch(threads, gpu, start, count);
    std::vector<u32> seeds;
    while (true)
    {
        bool searching = searcher.isSearching();
        auto found = searcher.getResults();
        seeds.insert(seeds.end(), found.begin(), found.end());
        if (!searching)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return seeds;
}

void Gen7Test::sfmtReference()
{
    // SFMT-19937 reference vector: init_gen_rand(1234) -> 3440181298, 1564997079, 1510669302 (handoff 1.2, sfmt.py)
    SFMT sfmt(1234);
    QCOMPARE(sfmt.nextUInt(), 3440181298u);
    QCOMPARE(sfmt.nextUInt(), 1564997079u);
    QCOMPARE(sfmt.nextUInt(), 1510669302u);

    // The period certification flips bit 0 of s[0] for seed 9FBDEDCF, so the first word differs from the seed
    SFMT certified(0x9FBDEDCF);
    QVERIFY(certified.nextUInt() != 0x9FBDEDCF);

    QCOMPARE(Gen7::titleAdvance(Game::Moon), 417u);
    QCOMPARE(Gen7::titleAdvance(Game::US), 477u);
    QCOMPARE(Gen7::idAdvance(Game::Sun), 1012u);
    QCOMPARE(Gen7::idAdvance(Game::UM), 1132u);
    QVERIFY(Gen7::idAdvanceVerified(Game::Sun));
    QVERIFY(!Gen7::idAdvanceVerified(Game::US));
}

void Gen7Test::needles()
{
    // Seed A7C27EA3: advance 417 -> needle 8, 418 -> 4 (live Moon, handoff 2.3)
    auto values = NeedleSearcher7::needlesFor(0xA7C27EA3, Gen7::titleAdvance(Game::Moon), 2);
    QCOMPARE(values.size(), size_t(2));
    QCOMPARE(values[0], u8(8));
    QCOMPARE(values[1], u8(4));

    // One advance is one u64: the needle is the full 64-bit value % 17
    SFMT sfmt(0xA7C27EA3, 417);
    QCOMPARE(Gen7::needle(sfmt.next()), u8(8));
    QCOMPARE(Gen7::needle(sfmt.next()), u8(4));
}

void Gen7Test::idGenerator()
{
    // Sun/Moon: ID = low word of draw 1012 when the name is confirmed (live, 2 runs): seed 7F0A5701 -> 3E922ECE (TID 11982, SID 16018),
    // seed 565F7430 -> F7FA7C1B (TID 31771, SID 63482)
    IDFilter filter({}, {}, {}, {}, {}, {});
    IDGenerator7 generator(Gen7::idAdvance(Game::Moon), 0, filter);

    auto first = generator.generate(0x7F0A5701);
    QCOMPARE(first.size(), size_t(1));
    QCOMPARE(first[0].getTID(), u16(11982));
    QCOMPARE(first[0].getSID(), u16(16018));
    QCOMPARE(first[0].getDisplayTID(), 0x3E922ECEu % 1000000);

    auto second = generator.generate(0x565F7430);
    QCOMPARE(second[0].getTID(), u16(31771));
    QCOMPARE(second[0].getSID(), u16(63482));
}

void Gen7Test::needleSearcherCPU()
{
    // 8 needles of a seed pin it down inside a 2^16 window
    u32 seed = 0xA7C27EA3;
    auto values = NeedleSearcher7::needlesFor(seed, 417, 8);
    NeedleSearcher7 searcher(417, values);
    auto seeds = runSearch(searcher, 4, false, seed - 0x8000, 0x10000);
    QCOMPARE(seeds.size(), size_t(1));
    QCOMPARE(seeds[0], seed);
    QVERIFY(!searcher.getUsedGPU());
}

void Gen7Test::needleSearcherGPU()
{
    if (!OpenCL::isAvailable())
    {
        QSKIP("No OpenCL device");
    }

    // The same search on the GPU over 2^24 seeds, for both title advances
    for (u32 title : { 417u, 477u })
    {
        u32 seed = 0xCF902E0D;
        auto values = NeedleSearcher7::needlesFor(seed, title, 9);
        NeedleSearcher7 searcher(title, values);
        auto seeds = runSearch(searcher, 1, true, seed - 0x800000, 0x1000000);
        QVERIFY2(searcher.getUsedGPU(), searcher.getError().data());
        QCOMPARE(seeds.size(), size_t(1));
        QCOMPARE(seeds[0], seed);
    }

    // Fewer needles give more candidates; every candidate must reproduce them
    auto two = NeedleSearcher7::needlesFor(0x12345678, 417, 2);
    NeedleSearcher7 loose(417, two);
    auto candidates = runSearch(loose, 1, true, 0x12340000, 0x10000);
    QVERIFY(candidates.size() > 100);
    for (u32 candidate : candidates)
    {
        QCOMPARE(NeedleSearcher7::needlesFor(candidate, 417, 2), two);
    }
}

void Gen7Test::needleSearcherGPUFull()
{
    // The real search: every seed, 8 needles (about 65-80 s on an RTX 4090). Set POKEFINDER_LONG_TESTS=1 to run it.
    if (!OpenCL::isAvailable())
    {
        QSKIP("No OpenCL device");
    }
    if (std::getenv("POKEFINDER_LONG_TESTS") == nullptr)
    {
        QSKIP("Set POKEFINDER_LONG_TESTS=1 to run the full 2^32 needle search");
    }

    u32 seed = 0xA7C27EA3;
    auto values = NeedleSearcher7::needlesFor(seed, 417, 8);
    NeedleSearcher7 searcher(417, values);
    auto start = std::chrono::steady_clock::now();
    auto seeds = runSearch(searcher, 1, true, 0, 1ULL << 32);
    auto seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    qInfo("Full needle search: %zu seed(s) in %.1f s", seeds.size(), seconds);
    QVERIFY2(searcher.getUsedGPU(), searcher.getError().data());
    QVERIFY(std::ranges::find(seeds, seed) != seeds.end());
    QVERIFY(seeds.size() <= 3); // 8 needles: unique in most boots, a few extras in the rest
}

static StateFilter allFilter7()
{
    std::array<bool, 25> natures;
    natures.fill(true);
    std::array<bool, 16> powers;
    powers.fill(true);
    return StateFilter(255, 255, 255, 1, 100, 0, 255, 0, 255, false, { 0, 0, 0, 0, 0, 0 }, { 31, 31, 31, 31, 31, 31 }, natures, powers);
}

static Spot7 customSpot(Game games, u8 models, u32 persist, u8 pre, u8 delay)
{
    return Spot7 { games, 0, 3, "test", { 0, 0, 0 }, models, persist, 0, models, persist, pre, delay, 4, 0, false, 0.0f, 29.913f, false, 0.0f, "" };
}

void Gen7Test::spots()
{
    // PRE-Moon 61 tiles (Sun = Moon), PRE-US / PRE-UM 52 usable tiles each, 35 USUM-only areas
    QCOMPARE(Spots7::getSpots(Game::Moon).size(), size_t(61));
    QCOMPARE(Spots7::getSpots(Game::Sun).size(), size_t(61));
    QCOMPARE(Spots7::getSpots(Game::US).size(), size_t(52 + 35));
    QCOMPARE(Spots7::getSpots(Game::UM).size(), size_t(52 + 35));

    const Spot7 *route1 = Spots7::getSpot(Game::Moon, 0, 3);
    QVERIFY(route1);
    QCOMPARE(route1->continueModels, u8(23));
    QVERIFY(route1->continuePersists(0) && route1->continuePersists(21) && route1->continuePersists(22) && !route1->continuePersists(1));
    QCOMPARE(route1->honeyDelay, u8(93));
    QCOMPARE(route1->honeyPreDraws, u8(0));

    const Spot7 *kalae = Spots7::getSpot(Game::US, 8, 0); // US Kala'e Bay keeps encounter id 0 at (17563, 3527)
    QVERIFY(kalae);
    QCOMPARE(kalae->continueModels, u8(2));
    QCOMPARE(kalae->honeyModels, u8(19));
    QCOMPARE(kalae->honeyDelay, u8(63));
    QCOMPARE(kalae->honeyPreDraws, u8(1));

    const Spot7 *tenCarat = Spots7::getSpot(Game::Moon, 19, 0);
    QVERIFY(tenCarat && tenCarat->hardwareTimer);
    QCOMPARE(tenCarat->hardwareOffset, -100.7f);
    QCOMPARE(tenCarat->azaharOffset, -62.2f);

    const Spot7 *route17 = Spots7::getSpot(Game::US, 172, 0);
    QVERIFY(route17 && (route17->flags & Spot7::Rain) && route17->honeyPreDraws == 3 && route17->fidget == 3);
    QVERIFY(Spots7::getSpot(Game::Moon, 999, 0) == nullptr);
}

void Gen7Test::encounterTables()
{
    // Moon Route 1 beach grass (zone 0, table 3): Lv 5-7, rates 20/20/10/10/10/10/10/5/4/1, day Yungoos/Wingull/Slowpoke, night
    // Alolan Rattata for Yungoos (handoff proof run 14.3). The step trace's slot roll 91 picked Slowpoke (4.2).
    auto day = Encounters7::getEncounters(Game::Moon, 0, 3, false);
    QCOMPARE(day.size(), size_t(1));
    const auto &table = day[0];
    QCOMPARE(table.getRates(), (std::array<u8, 10> { 20, 20, 10, 10, 10, 10, 10, 5, 4, 1 }));
    QCOMPARE(table.getPokemon(0).getSpecie(), u16(734)); // Yungoos
    QCOMPARE(table.getPokemon(1).getSpecie(), u16(278)); // Wingull
    QCOMPARE(table.getPokemon(0).getMinLevel(), u8(5));
    QCOMPARE(table.getPokemon(0).getMaxLevel(), u8(7));
    QCOMPARE(table.getEncounterSlot(91), u8(7));
    QCOMPARE(table.getPokemon(7).getSpecie(), u16(79)); // Slowpoke
    QCOMPARE(table.getEncounterSlot(0), u8(0));
    QCOMPARE(table.getEncounterSlot(99), u8(9));

    auto night = Encounters7::getEncounters(Game::Moon, 0, 3, true);
    QCOMPARE(night[0].getPokemon(0).getSpecie(), u16(19)); // Rattata
    QCOMPARE(night[0].getPokemon(0).getForm(), u8(1)); // Alolan
    QVERIFY(night[0].getPokemon(0).getInfo() != nullptr);

    // Ultra Sun has its own zone numbering (Route 1 beach = zone 0 too) and no table at an unknown zone
    QCOMPARE(Encounters7::getEncounters(Game::US, 0, 3, false).size(), size_t(1));
    QVERIFY(Encounters7::getEncounters(Game::Moon, 9999, 0, false).empty());
}

void Gen7Test::honeyUSUM()
{
    // Ultra Sun Route 1 beach (F 18, persist [0, 16, 17], pre 1, D 63): seed E6FB0960, bag 1787 -> lead 1931, Wingull EC B84EC798,
    // PID 8589A51A, IVs 16/12/7/25/19/10, nature 9, female = battle RAM (handoff 3.9)
    // The logged runs used the first-pass preset (F 18, M 2 persistent models) and were recorded before the Use draw was found
    // (UR V.2, superseded by the per-tile work): their bag advance already includes it, so pre 0 here. The clean-save preset in
    // the table lists 3 persistent models and pre 1 for that tile.
    Profile7 us("-", Game::US, 12345, 54321, false, false);
    Spot7 spot = customSpot(Game::US, 18, 0x3, 0, 63);
    auto areas = Encounters7::getEncounters(Game::US, 0, 2, false); // US Route 1 beach grass is table 2 (Moon: 3)
    QCOMPARE(areas.size(), size_t(1));

    HoneyGenerator7 generator(477, 1, 1, spot, areas[0], Lead::None, us, allFilter7());
    generator.setPersistFresh(true);
    HoneyState7 state = generator.generateAt(0xE6FB0960, 1787);
    QCOMPARE(state.getLeadAdvance(), 1931u);
    QCOMPARE(state.getSpecie(), u16(278));
    QCOMPARE(state.getEC(), 0xB84EC798u);
    QCOMPARE(state.getPID(), 0x8589A51Au);
    QCOMPARE(state.getIVs(), (std::array<u8, 6> { 16, 12, 7, 25, 19, 10 }));
    QCOMPARE(state.getNature(), u8(9));
    QCOMPARE(state.getGender(), u8(1));

    // Ultra Moon: seed AD8BE716, bag 1639 -> lead 1754, Wingull EC C867107B, PID 682E2830, IVs 1/25/29/5/5/0, nature 0, item 571
    Profile7 um("-", Game::UM, 12345, 54321, false, false);
    HoneyGenerator7 generatorUM(477, 1, 1, spot, Encounters7::getEncounters(Game::UM, 0, 2, false)[0], Lead::None, um, allFilter7());
    generatorUM.setPersistFresh(true);
    HoneyState7 moon = generatorUM.generateAt(0xAD8BE716, 1639);
    QCOMPARE(moon.getLeadAdvance(), 1754u);
    QCOMPARE(moon.getEC(), 0xC867107Bu);
    QCOMPARE(moon.getPID(), 0x682E2830u);
    QCOMPARE(moon.getIVs(), (std::array<u8, 6> { 1, 25, 29, 5, 5, 0 }));
    QCOMPARE(moon.getNature(), u8(0));
    QCOMPARE(moon.getItem(), u16(571));
}

void Gen7Test::honeySpecial()
{
    // Necrozma at Ten Carat Hill, Farthest Hollow (19/0: F 1, persist [0], D 93), 5 %: seed 3D7A71E9, bag 7875 -> roll 3 at 7969 ->
    // EC B3269580 at 8041 = roll + 72, PID BE10670D, IVs 26/31/31/27/31/21, Relaxed (handoff 4.7)
    Profile7 moon("-", Game::Moon, 12345, 54321, false, false);
    const Spot7 *spot = Spots7::getSpot(Game::Moon, 19, 0);
    QVERIFY(spot);
    auto areas = Encounters7::getEncounters(Game::Moon, 19, 0, false);
    QCOMPARE(areas.size(), size_t(1));

    HoneyGenerator7 generator(417, 1, 1, *spot, areas[0], Lead::None, moon, allFilter7());
    generator.setSpecial(SpecialTarget7 { 800, 0, 75, 5, 3, true });
    HoneyState7 necrozma = generator.generateAt(0x3D7A71E9, 7875);
    QVERIFY(necrozma.getSpecial());
    QCOMPARE(necrozma.getSpecie(), u16(800));
    QCOMPARE(necrozma.getEC(), 0xB3269580u);
    QCOMPARE(necrozma.getPID(), 0xBE10670Du);
    QCOMPARE(necrozma.getIVs(), (std::array<u8, 6> { 26, 31, 31, 27, 31, 21 }));
    QCOMPARE(necrozma.getNature(), u8(7)); // Relaxed

    // gen7sim's logged cases with the older "persistent models start fresh" reading: Nihilego (F 11, M 2, D 94, 80 %) EC 0A09123D,
    // Pheromosa (F 12, M 7, D 93, 50 %) EC 8EF5DA2D
    Spot7 nihilegoSpot = customSpot(Game::Moon, 11, 0x3, 0, 94);
    HoneyGenerator7 nihilego(417, 1, 1, nihilegoSpot, areas[0], Lead::None, moon, allFilter7());
    nihilego.setPersistFresh(true);
    nihilego.setSpecial(SpecialTarget7 { 793, 0, 55, 80, 3, true });
    QCOMPARE(nihilego.generateAt(0x4EB9CF4E, 1210).getEC(), 0x0A09123Du);

    Spot7 pheromosaSpot = customSpot(Game::Moon, 12, 0x7f, 0, 93);
    HoneyGenerator7 pheromosa(417, 1, 1, pheromosaSpot, areas[0], Lead::None, moon, allFilter7());
    pheromosa.setPersistFresh(true);
    pheromosa.setSpecial(SpecialTarget7 { 795, 0, 60, 50, 3, true });
    QCOMPARE(pheromosa.generateAt(0xCE557808, 1764).getEC(), 0x8EF5DA2Du);
}

void Gen7Test::honeyProofRun()
{
    // Shiny Yungoos by needles + Honey, no RAM reads (Azahar, 2026-10-20): seed 33FFEA15 from 8 needles, Route 1 beach grass preset
    // L 23 / persist [0, 9, 19, 22] (that save), X menu about 9 s after Continue, bag frame 974 (35.55 s): shiny Yungoos F Lv 7,
    // EC 3A884008, PID 878355C4, IVs 30/13/25/6/26/1, Naughty (handoff 3.9, proof run 14)
    QCOMPARE(NeedleSearcher7::needlesFor(0x33FFEA15, 417, 8), (std::vector<u8> { 1, 7, 6, 1, 14, 11, 16, 7 }));

    Profile7 moon("-", Game::Moon, 12345, 54321, false, false);
    Spot7 spot = customSpot(Game::Moon, 23, (1u << 0) | (1u << 9) | (1u << 19) | (1u << 22), 0, 93);
    auto areas = Encounters7::getEncounters(Game::Moon, 0, 3, false);

    Timer7 azahar { -89.3f, 29.91f };
    QVERIFY(std::abs(azahar.seconds(974) - 35.55) < 0.01);
    u32 menuFrame = static_cast<u32>(azahar.frame(9.0));
    QVERIFY(menuFrame > 4 && menuFrame < 406);

    HoneyGenerator7 generator(417 + 8, 974, 974, spot, areas[0], Lead::None, moon, allFilter7());
    generator.setMenuFrame(menuFrame);
    auto states = generator.generate(0x33FFEA15);
    QCOMPARE(states.size(), size_t(1));
    const HoneyState7 &yungoos = states[0];
    QCOMPARE(yungoos.getFrame(), 974u);
    QCOMPARE(yungoos.getSpecie(), u16(734));
    QCOMPARE(yungoos.getLevel(), u8(7));
    QCOMPARE(yungoos.getEC(), 0x3A884008u);
    QCOMPARE(yungoos.getPID(), 0x878355C4u);
    QCOMPARE(yungoos.getIVs(), (std::array<u8, 6> { 30, 13, 25, 6, 26, 1 }));
    QCOMPARE(yungoos.getNature(), u8(4)); // Naughty
    QCOMPARE(yungoos.getGender(), u8(1));

    // The player's profile makes it shiny: the run's TSV matched the PID
    u16 psv = ((0x878355C4u >> 16) ^ (0x878355C4u & 0xffff)) >> 4;
    Profile7 player("-", Game::Moon, 0, static_cast<u16>(psv << 4), false, false);
    HoneyGenerator7 shiny(417 + 8, 974, 974, spot, areas[0], Lead::None, player, allFilter7());
    shiny.setMenuFrame(menuFrame);
    QVERIFY(shiny.generate(0x33FFEA15)[0].getShiny() != 0);
}

void Gen7Test::timer()
{
    // Hardware: Ten Carat Hill O = -100.7 at 29.9 fps; one-press calibration recovers the offset
    Timer7 timer { -100.7f, 29.9f };
    double t = timer.seconds(1050);
    QVERIFY(std::abs(t - (1050 + 100.7) / 29.9) < 1e-6);
    QVERIFY(std::abs(timer.frame(t) - 1050) < 1e-3);
    QVERIFY(std::abs(timer.calibrate(t, 1050) - (-100.7f)) < 1e-3);
    QVERIFY(std::abs(timer.calibrate(t, 1049) - (-101.7f)) < 1e-3);
    QVERIFY(std::abs(Timer7::fitFps(10.0, 199, 40.0, 1096) - 29.9f) < 1e-3);
}

static const StaticTemplate7 *findStatic7(int category, Game version, u16 specie, u8 form = 0)
{
    int size = 0;
    const StaticTemplate7 *templates = Encounters7::getStaticEncounters(category, &size);
    for (int i = 0; i < size; i++)
    {
        if (templates[i].getSpecie() == specie && templates[i].getForm() == form && (templates[i].getVersion() & version) != Game::None)
        {
            return &templates[i];
        }
    }
    return nullptr;
}

void Gen7Test::statics()
{
    // Ultra Sun Tapu Koko (seed 727FAFFC): EC 0201FA19 at 1299 = lead + 64, PID 3EF6E797 (shiny lock), rand(6) picks 1, 5, 2,
    // HP 19 / SpA 5 / SpD 14, Careful (handoff 4.8)
    Profile7 us("-", Game::US, 12345, 54321, false, false);
    const StaticTemplate7 *koko = findStatic7(0, Game::US, 785);
    QVERIFY(koko);
    StaticGenerator7 generator(1299 - 64, 0, 3, Lead::None, *koko, us, allFilter7());
    auto states = generator.generate(0x727FAFFC);
    QCOMPARE(states.size(), size_t(1));
    QCOMPARE(states[0].getEC(), 0x0201FA19u);
    QCOMPARE(states[0].getPID(), 0x3EF6E797u);
    QCOMPARE(states[0].getIVs(), (std::array<u8, 6> { 19, 31, 31, 5, 14, 31 }));
    QCOMPARE(states[0].getNature(), u8(23));
    QCOMPARE(states[0].getLevel(), u8(60));
    QCOMPARE(states[0].getGender(), u8(2));

    // Ultra Sun Tapu Bulu (seed 88AAF0E8): lead 1243, EC BE2F18FD at 1307
    const StaticTemplate7 *bulu = findStatic7(0, Game::US, 787);
    QVERIFY(bulu);
    StaticGenerator7 buluGenerator(1243, 0, 3, Lead::None, *bulu, us, allFilter7());
    auto buluStates = buluGenerator.generate(0x88AAF0E8);
    QCOMPARE(buluStates.size(), size_t(1));
    QCOMPARE(buluStates[0].getEC(), 0xBE2F18FDu);

    // The lock never gives a shiny for the player
    Profile7 lucky("-", Game::US, 0x0201, 0xFA19 ^ 0x3EF6, false, false);
    StaticGenerator7 locked(1299 - 64, 0, 3, Lead::None, *koko, lucky, allFilter7());
    QCOMPARE(locked.generate(0x727FAFFC)[0].getShiny(), u8(0));
}

void Gen7Test::gifts()
{
    // Moon Cosmog (seed 18CA1E24, advance 5084): EC 7991331C, PID 724C0413, IVs 31/7/8/9/31/31, Lax; the first draw is the EC and
    // Cosmog draws no ability (handoff 4.9)
    Profile7 moon("-", Game::Moon, 12345, 54321, false, false);
    const StaticTemplate7 *cosmog = findStatic7(3, Game::Moon, 789);
    QVERIFY(cosmog && !cosmog->getAbilityDraw());
    StaticGenerator7 generator(5084, 0, 3, Lead::None, *cosmog, moon, allFilter7());
    auto states = generator.generate(0x18CA1E24);
    QCOMPARE(states.size(), size_t(1));
    QCOMPARE(states[0].getEC(), 0x7991331Cu);
    QCOMPARE(states[0].getPID(), 0x724C0413u);
    QCOMPARE(states[0].getIVs(), (std::array<u8, 6> { 31, 7, 8, 9, 31, 31 }));
    QCOMPARE(states[0].getNature(), u8(9)); // Lax
    QCOMPARE(states[0].getLevel(), u8(5));

    // Moon Type: Null (seed 35594014, advances 17230..): EC 6B2FBD4B, IVs 23/31/29/20/31/31, with the ability draw. The research
    // log records the nature draw as 21 (Gentle); its "Careful" label is a slip
    const StaticTemplate7 *typeNull = findStatic7(3, Game::Moon, 772);
    QVERIFY(typeNull && typeNull->getAbilityDraw());
    StaticGenerator7 nullGenerator(17230, 0, 3, Lead::None, *typeNull, moon, allFilter7());
    auto nullStates = nullGenerator.generate(0x35594014);
    QCOMPARE(nullStates.size(), size_t(1));
    QCOMPARE(nullStates[0].getEC(), 0x6B2FBD4Bu);
    QCOMPARE(nullStates[0].getIVs(), (std::array<u8, 6> { 23, 31, 29, 20, 31, 31 }));
    QCOMPARE(nullStates[0].getNature(), u8(21)); // Gentle

    // Sun Cosmog (seed F9EB5EAA): EC 242158C5, IVs 31/13/10/23/31/31, Lonely at 6520 or 6521
    Profile7 sun("-", Game::Sun, 12345, 54321, false, false);
    const StaticTemplate7 *cosmogSun = findStatic7(3, Game::Sun, 789);
    StaticGenerator7 sunGenerator(6520, 1, 3, Lead::None, *cosmogSun, sun, allFilter7());
    auto sunStates = sunGenerator.generate(0xF9EB5EAA);
    bool found = false;
    for (const auto &state : sunStates)
    {
        if (state.getEC() == 0x242158C5u)
        {
            found = true;
            QCOMPARE(state.getIVs(), (std::array<u8, 6> { 31, 13, 10, 23, 31, 31 }));
            QCOMPARE(state.getNature(), u8(1)); // Lonely
        }
    }
    QVERIFY(found);

    // Synchronize fixes a gift's nature with no roll: the IVs stay the same, the nature follows the lead
    StaticGenerator7 synced(5084, 0, 3, Lead::Synchronize, *cosmog, moon, allFilter7());
    auto syncedStates = synced.generate(0x18CA1E24);
    QCOMPARE(syncedStates[0].getIVs(), states[0].getIVs());
    QCOMPARE(syncedStates[0].getNature(), u8(0));
}

void Gen7Test::trades()
{
    // Ultra Sun Hawlucha (seed A6070DB3): 5 x rand(32) at 7712-7716 = 10/1/4/31/30 (HP, Def, SpA, SpD, Spe; Atk fixed 31),
    // EC F1EE3B64 at 7717, PID F67F2226, Brave (handoff 4.11)
    Profile7 us("-", Game::US, 12345, 54321, false, false);
    const StaticTemplate7 *hawlucha = findStatic7(5, Game::US, 701);
    QVERIFY(hawlucha && hawlucha->getKind() == StaticKind7::Trade);
    StaticGenerator7 generator(7712, 0, 3, Lead::None, *hawlucha, us, allFilter7());
    auto states = generator.generate(0xA6070DB3);
    QCOMPARE(states.size(), size_t(1));
    QCOMPARE(states[0].getIVs(), (std::array<u8, 6> { 10, 31, 1, 4, 31, 30 }));
    QCOMPARE(states[0].getEC(), 0xF1EE3B64u);
    QCOMPARE(states[0].getPID(), 0xF67F2226u);
    QCOMPARE(states[0].getNature(), u8(2)); // Brave
    QCOMPARE(states[0].getGender(), u8(1));
    QCOMPARE(states[0].getLevel(), u8(8));

    // SM table: 7 trades, each with one fixed 31 and a TSV from the trade's OT ID
    int size = 0;
    const StaticTemplate7 *sm = Encounters7::getStaticEncounters(5, &size);
    QCOMPARE(size, 14);
    QCOMPARE(sm[1].getSpecie(), u16(761));
    QCOMPARE(sm[1].getTradeTSV(), u16(20683 ^ 9));
    QCOMPARE(sm[1].getIVs()[1], u8(31));
}

void Gen7Test::eggs()
{
    // Audit retests 2026-10-24 (Moon, copy 2), every egg from the daycare TinyMT 81DD662D F401F495 79C9DF86 AC2B733F. Parent 0 is the
    // father role (the Ditto here), parent 1 the mother role (Yungoos F, Naughty, ability slot 1, 30/13/25/6/26/1)
    const std::array<u32, 4> base = { 0x81DD662D, 0xF401F495, 0x79C9DF86, 0xAC2B733F };
    Profile7 moon("-", Game::Moon, 12345, 54321, false, false);
    Profile7 charm("-", Game::Moon, 12345, 54321, true, false);

    // E1: Masuda (Ditto JPN) + Everstone on Yungoos + Destiny Knot on Ditto -> F Naughty, slot 1, 31/31/31/6/26/18, EC E03ED89D,
    // PID 5BF13EDF (6th roll), 26 draws, accept 28, state after 1933D695 C36B3F09 AFB6F04E 343CC6C5
    {
        Daycare daycare({ { { 31, 31, 31, 31, 31, 31 }, { 30, 13, 25, 6, 26, 1 } } }, { 0, 0 }, { 3, 1 }, { 8, 1 }, { 0, 4 }, 734, true, 20);
        EggGenerator7 generator(daycare, false, moon, allFilter7());
        EggState7 egg = generator.generate(base);
        QCOMPARE(egg.getGender(), u8(1));
        QCOMPARE(egg.getNature(), u8(4));
        QCOMPARE(egg.getAbility(), u8(0));
        QCOMPARE(egg.getIVs(), (std::array<u8, 6> { 31, 31, 31, 6, 26, 18 }));
        QCOMPARE(egg.getEC(), 0xE03ED89Du);
        QCOMPARE(egg.getPID(), 0x5BF13EDFu);
        QVERIFY(!egg.getMainPID());
        QCOMPARE(egg.getDraws(), u8(26));
        QCOMPARE(egg.getState(), (std::array<u32, 4> { 0x1933D695, 0xC36B3F09, 0xAFB6F04E, 0x343CC6C5 }));
    }

    // E2: Shiny Charm, Everstone on both (Yungoos Naughty slot 0 -> here ability index 0, Ditto Hardy) -> Naughty (pick 1 = mother),
    // ability slot 2, 30/13/20/31/23/8, EC E84DEB5D, PID 5276A2B2 (2nd Charm roll), 19 draws, state after F8AA7482 929ABD3E E6A86D2A 76A3C77D
    {
        Daycare daycare({ { { 31, 31, 31, 31, 31, 31 }, { 30, 13, 25, 6, 26, 1 } } }, { 0, 0 }, { 3, 1 }, { 1, 1 }, { 0, 4 }, 734, false, 20);
        EggGenerator7 generator(daycare, false, charm, allFilter7());
        EggState7 egg = generator.generate(base);
        QCOMPARE(egg.getNature(), u8(4));
        QCOMPARE(egg.getAbility(), u8(1));
        QCOMPARE(egg.getIVs(), (std::array<u8, 6> { 30, 13, 20, 31, 23, 8 }));
        QCOMPARE(egg.getEC(), 0xE84DEB5Du);
        QCOMPARE(egg.getPID(), 0x5276A2B2u);
        QCOMPARE(egg.getDraws(), u8(19));
        QCOMPARE(egg.getState(), (std::array<u32, 4> { 0xF8AA7482, 0x929ABD3E, 0xE6A86D2A, 0x76A3C77D }));
    }

    // E3: Masuda + Charm + Power Belt (Def) on Yungoos, Power Weight (HP) on Ditto -> pick 0 = father's Weight -> HP 31,
    // 31/13/30/31/20/21, EC AB1222D7, PID 2FB00E56 (8th roll), 23 draws
    {
        Daycare daycare({ { { 31, 31, 31, 31, 31, 31 }, { 30, 13, 25, 6, 26, 1 } } }, { 0, 0 }, { 3, 1 }, { 2, 4 }, { 0, 4 }, 734, true, 20);
        EggGenerator7 generator(daycare, false, charm, allFilter7());
        EggState7 egg = generator.generate(base);
        QCOMPARE(egg.getIVs(), (std::array<u8, 6> { 31, 13, 30, 31, 20, 21 }));
        QCOMPARE(egg.getEC(), 0xAB1222D7u);
        QCOMPARE(egg.getPID(), 0x2FB00E56u);
        QCOMPARE(egg.getDraws(), u8(23));
        QCOMPARE(egg.getState(), (std::array<u32, 4> { 0xB70AC9E1, 0x25300D6E, 0xBA141F53, 0xDE9AABA1 }));
    }

    // E5: Nidoran F (slot 1) + Ditto, no rolls -> the species coin replaces the gender draw: Nidoran M, Gentle, 15/30/31/6/26/23,
    // EC 680C3488, PID from the main RNG, 16 draws, accept 18
    {
        Daycare daycare({ { { 31, 31, 31, 31, 31, 31 }, { 30, 13, 25, 6, 26, 1 } } }, { 0, 0 }, { 3, 1 }, { 0, 0 }, { 0, 4 }, 29, false, 20);
        EggGenerator7 generator(daycare, false, moon, allFilter7());
        EggState7 egg = generator.generate(base);
        QCOMPARE(egg.getGender(), u8(0));
        QCOMPARE(egg.getNature(), u8(21)); // Gentle
        QCOMPARE(egg.getIVs(), (std::array<u8, 6> { 15, 30, 31, 6, 26, 23 }));
        QCOMPARE(egg.getEC(), 0x680C3488u);
        QVERIFY(egg.getMainPID());
        QCOMPARE(egg.getDraws(), u8(16));
        QCOMPARE(egg.getPID(), 0u);
    }

    // E4: same species (Yungoos M 0/1/2/3/4/5 Great Ball, Yungoos F Poke Ball) + Power Lens (SpA) on the female -> 15/30/2/6/26/23,
    // EC 680C3488, ball roll rand(100) = 37 after the EC, accept = 17 + 2
    {
        Daycare daycare({ { { 0, 1, 2, 3, 4, 5 }, { 30, 13, 25, 6, 26, 1 } } }, { 1, 0 }, { 0, 1 }, { 0, 5 }, { 0, 4 }, 734, false, 20);
        EggGenerator7 generator(daycare, true, moon, allFilter7());
        EggState7 egg = generator.generate(base);
        QCOMPARE(egg.getIVs(), (std::array<u8, 6> { 15, 30, 2, 6, 26, 23 }));
        QCOMPARE(egg.getEC(), 0x680C3488u);
        QCOMPARE(egg.getDraws(), u8(17));

        // The chain walks the state by the accept cost; a reject costs one draw
        auto chain = generator.generateChain(base, 0, 2);
        QCOMPARE(chain.size(), size_t(2));
        QCOMPARE(chain[0].getEC(), 0x680C3488u);
        QCOMPARE(chain[1].getAdvances(), 2u);
        QVERIFY(chain[1].getEC() != chain[0].getEC());

        // The main RNG PID: Moon egg logged at advance 4324 as 7C562599 from the live seed is not in the notes; check the shape
        auto mainPID = generator.generateMainPID(base, 0x35594014, 100, 2);
        QCOMPARE(mainPID.size(), size_t(3));
        QCOMPARE(mainPID[0].getAdvances(), 100u);
        QCOMPARE(mainPID[0].getIVs(), (std::array<u8, 6> { 15, 30, 2, 6, 26, 23 }));
        QCOMPARE(mainPID[0].getPID(), static_cast<u32>(SFMT(0x35594014, 100).next()));
    }
}

void Gen7Test::wonderCards()
{
    // Ultra Sun Munchlax card 1601 (random IVs, nature and gender, fixed ability): seed A6070DB3, kept block at 7723 -> EC 2D9578EC,
    // IVs 3/2/6/12/29/4, Sassy, gender roll 139 -> M (usum-findings-log)
    Profile7 us("-", Game::US, 12345, 54321, false, false);
    std::array<u8, 6> random;
    random.fill(Gen7::Random);
    WC7 munchlax(446, 0, 5, 0, 0, true, 0, Gen7::PIDType7::Random, 0, random, 0, 0, Gen7::Random, 3, false);
    EventGenerator7 generator(7723, 0, 0, munchlax, us, allFilter7());
    auto states = generator.generate(0xA6070DB3);
    QCOMPARE(states.size(), size_t(1));
    QCOMPARE(states[0].getEC(), 0x2D9578ECu);
    QCOMPARE(states[0].getIVs(), (std::array<u8, 6> { 3, 2, 6, 12, 29, 4 }));
    QCOMPARE(states[0].getNature(), u8(22)); // Sassy
    QCOMPARE(states[0].getGender(), u8(0));
    QCOMPARE(states[0].getLevel(), u8(5));

    // The discarded blocks consume the same shape: with 1 block before, the kept block starts 10 draws later for this card
    EventGenerator7 twice(7713, 0, 1, munchlax, us, allFilter7());
    auto twiceStates = twice.generate(0xA6070DB3);
    QCOMPARE(twiceStates.size(), size_t(1));
    QCOMPARE(twiceStates[0].getEC(), 0x2D9578ECu);

    // Forced shiny: the low half is kept and the high half becomes low ^ TID ^ SID (square)
    WC7 magikarp(129, 0, 5, 39056, 2, false, 0, Gen7::PIDType7::Shiny, 0, random, 0, 0, Gen7::Random, 3, false);
    EventGenerator7 shiny(0, 5, 0, magikarp, us, allFilter7());
    for (const auto &state : shiny.generate(0xA6070DB3))
    {
        QCOMPARE(state.getShiny(), u8(2));
        QCOMPARE(((state.getPID() >> 16) ^ (state.getPID() & 0xffff) ^ 39056 ^ 2), 0u);
    }

    // Shiny lock never yields a shiny
    WC7 raikou(243, 0, 60, 0, 0, true, 0, Gen7::PIDType7::Locked, 0, random, 3, 0, Gen7::Random, 2, false);
    Profile7 lucky("-", Game::Moon, 0, 0, false, false);
    EventGenerator7 locked(0, 2000, 1, raikou, lucky, allFilter7());
    for (const auto &state : locked.generate(0x12345678))
    {
        QCOMPARE(state.getShiny(), u8(0));
        int perfect = 0;
        for (u8 iv : state.getIVs())
        {
            perfect += iv == 31;
        }
        QVERIFY(perfect >= 3);
    }

    // .wc7 layout: the same bytes as a .wc6 (gift data at file + 0x68)
    std::vector<u8> data(WC7::size, 0);
    data[0x82] = 446 & 0xff;
    data[0x83] = 446 >> 8;
    data[0xa0] = 0xff;
    data[0xa1] = 3;
    data[0xa2] = 4;
    data[0xa3] = 2;
    data[0xaf] = 0xfe;
    data[0xd0] = 50;
    WC7 parsed = WC7::parse(data.data());
    QCOMPARE(parsed.getSpecie(), u16(446));
    QCOMPARE(parsed.getLevel(), u8(50));
    QCOMPARE(parsed.getNature(), Gen7::Random);
    QCOMPARE(parsed.getGender(), u8(3));
    QCOMPARE(parsed.getAbility(), Gen7::AbilityRandomHidden);
    QCOMPARE(parsed.getPIDType(), Gen7::PIDType7::Shiny);
    QCOMPARE(parsed.getIVCount(), u8(3));
}

void Gen7Test::pelago()
{
    // Moon, seed A98C7463: the two visitors are the last blocks built on entry, 12 draws each (EC, discarded OT ID, PID, 6 IVs, ability,
    // nature, gender). Spearow at 3745: EC 5A264E11, PID 9C2CF670, 29/31/13/16/3/15, Modest, F; Pikipek at 3757: EC 6FEC8023,
    // PID 5C1D0E8D, 20/28/28/13/14/17, Bold, M (Moon reference 22.4n)
    Profile7 moon("-", Game::Moon, 12345, 54321, false, false);
    PelagoGenerator7 spearow(3745, 0, 21, 5, moon, allFilter7());
    auto states = spearow.generate(0xA98C7463);
    QCOMPARE(states.size(), size_t(1));
    QCOMPARE(states[0].getEC(), 0x5A264E11u);
    QCOMPARE(states[0].getPID(), 0x9C2CF670u);
    QCOMPARE(states[0].getIVs(), (std::array<u8, 6> { 29, 31, 13, 16, 3, 15 }));
    QCOMPARE(states[0].getNature(), u8(15)); // Modest
    QCOMPARE(states[0].getGender(), u8(1));

    PelagoGenerator7 pikipek(3757, 0, 731, 5, moon, allFilter7());
    auto pikipekStates = pikipek.generate(0xA98C7463);
    QCOMPARE(pikipekStates.size(), size_t(1));
    QCOMPARE(pikipekStates[0].getEC(), 0x6FEC8023u);
    QCOMPARE(pikipekStates[0].getPID(), 0x5C1D0E8Du);
    QCOMPARE(pikipekStates[0].getIVs(), (std::array<u8, 6> { 20, 28, 28, 13, 14, 17 }));
    QCOMPARE(pikipekStates[0].getNature(), u8(5)); // Bold
    QCOMPARE(pikipekStates[0].getGender(), u8(0));
}

void Gen7Test::save()
{
    // A synthetic Ultra Sun main: MyStatus at 0x1400, Sodateya at 0x6BA00, ResortSave at 0x3B800
    std::vector<u8> data(Save7::sizeUSUM, 0);
    auto put32 = [&data](size_t offset, u32 value) {
        for (int i = 0; i < 4; i++)
        {
            data[offset + i] = static_cast<u8>(value >> (8 * i));
        }
    };
    data[0x1400] = 9459 & 0xff;
    data[0x1401] = 9459 >> 8;
    data[0x1402] = 63166 & 0xff;
    data[0x1403] = 63166 >> 8;
    data[0x1404] = 32;
    data[0x6ba00 + 0x1d8] = 1;
    const u32 egg[4] = { 0x81DD662D, 0xF401F495, 0x79C9DF86, 0xAC2B733F };
    const u32 pelagoState[4] = { 0x563D2CEF, 0x82224F1F, 0x762769DD, 0xFFDA7125 };
    for (size_t i = 0; i < 4; i++)
    {
        put32(0x6ba00 + 0x1dc + 4 * i, egg[i]);
        put32(0x3b800 + 4 * i, pelagoState[i]);
    }

    Save7 save = Save7::parse(data.data(), data.size());
    QVERIFY(save.valid);
    QCOMPARE(save.version, Game::US);
    QCOMPARE(save.tid, u16(9459));
    QCOMPARE(save.sid, u16(63166));
    QVERIFY(save.eggReady);
    QCOMPARE(save.eggState, (std::array<u32, 4> { 0x81DD662D, 0xF401F495, 0x79C9DF86, 0xAC2B733F }));
    QCOMPARE(save.pelagoState, (std::array<u32, 4> { 0x563D2CEF, 0x82224F1F, 0x762769DD, 0xFFDA7125 }));

    QVERIFY(!Save7::parse(data.data(), 100).valid);
}
