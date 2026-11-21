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

#include "Gen8aTest.hpp"
#include <Core/Gen8a/Generator8a.hpp>
#include <Core/Gen8a/Searchers/BootSeedSearcher8a.hpp>
#include <Core/Gen8a/Searchers/StockBootSeedSearcher8a.hpp>
#include <Core/Gen8a/Searchers/SpawnerSearcher8a.hpp>
#include <Core/Gen8a/SharedGenerator8a.hpp>
#include <Core/Gen8a/SpawnerGenerator8a.hpp>
#include <Core/Gen8a/Templates8a.hpp>
#include <Core/Util/OpenCL.hpp>
#include <QTest>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <thread>

namespace
{
    // Console launch 6 (research section 30): boot seed 3C87FF373909B92C, Unown H then W from the first load at
    // advance 138. Caught on the console: H EC 8116BDC5 PID 1F17F706 11/29/7/10/22/30 Careful,
    // W EC 2ADA1D46 PID 135E7A51 6/29/1/22/21/27 Jolly, both ability slot 0
    constexpr u64 launch6 = 0x3C87FF373909B92CULL;

    std::vector<UnownObservation8a> launch6Unown(bool ability)
    {
        UnownObservation8a h { { 11, 29, 7, 10, 22, 30 }, static_cast<s8>(ability ? 0 : -1), 23 };
        UnownObservation8a w { { 6, 29, 1, 22, 21, 27 }, static_cast<s8>(ability ? 0 : -1), 13 };
        return { h, w };
    }

    std::vector<BootSeedResult8a> runBoot(BootSeedSearcher8a &searcher, int threads, bool gpu)
    {
        searcher.startSearch(threads, gpu);
        std::vector<BootSeedResult8a> found;
        while (true)
        {
            bool searching = searcher.isSearching();
            auto part = searcher.getResults();
            found.insert(found.end(), part.begin(), part.end());
            if (!searching)
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::sort(found.begin(), found.end(), [](const auto &a, const auto &b) { return a.advance < b.advance; });
        return found;
    }

    int threads()
    {
        return std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
    }
}

void Gen8aTest::unownConsole()
{
    Xoro8a rng(launch6);
    rng.advance(138);
    State8a h = Generator8a::generate(rng, Templates8a::unown(7), false);
    State8a w = Generator8a::generate(rng, Templates8a::unown(22), false);

    QCOMPARE(h.ec, 0x8116BDC5u);
    QCOMPARE(h.pid, 0x1F17F706u);
    QCOMPARE(h.ivs, (std::array<u8, 6> { 11, 29, 7, 10, 22, 30 }));
    QCOMPARE(h.nature, 23);
    QCOMPARE(h.ability, 0);
    QCOMPARE(h.gender, 2);
    QCOMPARE(h.height, 128);
    QCOMPARE(h.weight, 92);
    QCOMPARE(h.draws, 16u);

    QCOMPARE(w.ec, 0x2ADA1D46u);
    QCOMPARE(w.pid, 0x135E7A51u);
    QCOMPARE(w.ivs, (std::array<u8, 6> { 6, 29, 1, 22, 21, 27 }));
    QCOMPARE(w.nature, 13);
    QCOMPARE(w.weight, 233);
}

void Gen8aTest::giftStarter()
{
    // Kept Oshawott (shoki3_mizu), research section 22: logged state at the generator call -> the party Pokémon
    const RawTemplate8a *row = nullptr;
    for (const auto &gift : Templates8a::gifts())
    {
        if (std::strcmp(gift.name, "shoki3_mizu") == 0)
        {
            row = &gift;
        }
    }
    QVERIFY(row != nullptr);
    Xoro8a rng(0x8645B5A716B49EE5ULL, 0x6BE2B30CB80EE201ULL);
    State8a s = Generator8a::generate(rng, Templates8a::spec(*row), false);
    QCOMPARE(s.ec, 0xCEC380E6u);
    QCOMPARE(s.pid, 0xA900F888u);
    QCOMPARE(s.ivs, (std::array<u8, 6> { 29, 0, 12, 7, 22, 16 }));
    QCOMPARE(s.nature, 12);
    QCOMPARE(s.ability, 1);
    QCOMPARE(s.gender, 0);
    QCOMPARE(s.height, 127);
    QCOMPARE(s.weight, 127);
}

void Gen8aTest::templates()
{
    QCOMPARE(Templates8a::gifts().size(), size_t(8));
    QVERIFY(Templates8a::statics().size() > 150);
    bool chimchar = false;
    for (const auto &row : Templates8a::statics())
    {
        if (std::strcmp(row.name, "hikozaru_01") == 0)
        {
            chimchar = true;
            QCOMPARE(row.species, u16(390));
            QCOMPARE(row.genderRatio, u8(31));
            QCOMPARE(row.shinyLocked, u8(1));
        }
    }
    QVERIFY(chimchar);
    QCOMPARE(Templates8a::speciesName(899), std::string("Wyrdeer"));
}

void Gen8aTest::sharedGenerator()
{
    SharedFilter8a filter;
    filter.ivMin = { 11, 29, 7, 10, 22, 30 };
    filter.ivMax = filter.ivMin;
    SharedGenerator8a generator(launch6, 100, 100, { Templates8a::unown(7), Templates8a::unown(22) }, filter);
    auto states = generator.generate();

    // The same draws make H as the first Unown at 138, and as the second Unown of a batch started 16 draws earlier, etc.
    auto first = std::find_if(states.begin(), states.end(), [](const SharedState8a &s) { return s.index == 0; });
    QVERIFY(first != states.end());
    QCOMPARE(first->advance, 138u);
    QCOMPARE(first->end, 154u);
    QCOMPARE(std::count_if(states.begin(), states.end(), [](const SharedState8a &s) { return s.index == 0; }), 1);
}

void Gen8aTest::spawnerChain()
{
    // Reference: PLA-RNG-Research/scripts/pla_model.py fill/slot/generate from group seed 1234ABCD5678EF01, 1 per fill
    auto states = SpawnerGenerator8a::generate(0x1234ABCD5678EF01ULL, 3, 1, 1, 127, false);
    QCOMPARE(states.size(), size_t(3));
    QCOMPARE(states[0].slotSeed, 0x94D75D427916595CULL);
    QCOMPARE(states[0].alphaSeed, 0xE428C205037108FFULL);
    QCOMPARE(states[0].entitySeed, 0x29A202C802D66B45ULL);
    QCOMPARE(states[0].state.ec, 0x2573D5A0u);
    QCOMPARE(states[0].state.ivs, (std::array<u8, 6> { 1, 12, 24, 27, 21, 5 }));
    QCOMPARE(states[0].state.nature, 7);
    QCOMPARE(states[0].state.gender, 1);
    QCOMPARE(states[0].state.height, 90);
    QCOMPARE(states[0].state.weight, 211);
    QCOMPARE(states[1].groupSeed, 0x22B5D23B6EEE5B05ULL);
    QCOMPARE(states[1].entitySeed, 0x0883F17B7BCF506EULL);
    QCOMPARE(states[1].state.ec, 0x9E6CBAC9u);
    QCOMPARE(states[2].groupSeed, 0x57194883E59C30D3ULL);
    QCOMPARE(states[2].entitySeed, 0x3F3B8A5F58315C11ULL);
    QCOMPARE(states[2].state.ivs, (std::array<u8, 6> { 5, 26, 20, 14, 31, 5 }));
    QCOMPARE(states[2].state.nature, 18);
}

void Gen8aTest::slotInversion()
{
    u64 seeds[] = { 0x94D75D427916595CULL, 0xA55883B0918BC560ULL, 0x0123456789ABCDEFULL };
    for (u64 slot : seeds)
    {
        Xoro8a rng(slot);
        rng.next();
        u64 entity = rng.next();
        auto found = SpawnerSearcher8a::slotSeeds(entity);
        QVERIFY(std::find(found.begin(), found.end(), slot) != found.end());
        for (u64 s : found)
        {
            Xoro8a check(s);
            check.next();
            QCOMPARE(check.next(), entity);
        }
    }
}

void Gen8aTest::bootSearchCPU()
{
    BootSeedSearcher8a searcher(launch6Unown(true), 136, 140, 14);
    auto found = runBoot(searcher, threads(), false);
    QCOMPARE(found.size(), size_t(1));
    QCOMPARE(found[0].seed, launch6);
    QCOMPARE(found[0].advance, 138u);
    QCOMPARE(found[0].end, 171u);
    QCOMPARE(found[0].weights, (std::vector<u8> { 92, 233 }));
}

void Gen8aTest::bootSearchGPU()
{
    if (!OpenCL::isAvailable())
    {
        QSKIP("No OpenCL device");
    }
    // Console run (section 30): the blind search over 115-170 found the true seed and one false one at 118, which the
    // weights tell apart
    BootSeedSearcher8a searcher(launch6Unown(true), 115, 170, 14);
    auto found = runBoot(searcher, threads(), true);
    QVERIFY(searcher.getUsedGPU());
    QCOMPARE(found.size(), size_t(2));
    QCOMPARE(found[0].seed, 0x77BE613FB058EC8EULL);
    QCOMPARE(found[0].advance, 118u);
    QCOMPARE(found[1].seed, launch6);
    QCOMPARE(found[1].advance, 138u);

    // Weight of W (233) leaves only the true seed
    auto unown = launch6Unown(true);
    unown[1].weightMin = 230;
    unown[1].weightMax = 236;
    BootSeedSearcher8a weighed(unown, 115, 170, 14);
    found = runBoot(weighed, threads(), true);
    QCOMPARE(found.size(), size_t(1));
    QCOMPARE(found[0].seed, launch6);
}

void Gen8aTest::spawnerSearchLong()
{
    if (std::getenv("POKEFINDER_LONG_TESTS") == nullptr)
    {
        QSKIP("Set POKEFINDER_LONG_TESTS=1 to run the spawner solve (about 2 minutes)");
    }
    auto observe = [](u64 entity) {
        Spec8a spec;
        spec.genderRatio = 127;
        Xoro8a rng(entity);
        State8a s = Generator8a::generate(rng, spec, true);
        WildObservation8a o;
        o.ivs = s.ivs;
        o.ability = static_cast<s8>(s.ability);
        o.gender = static_cast<s8>(s.gender);
        o.nature = static_cast<s8>(s.nature);
        o.shiny = s.shiny ? 1 : 0;
        o.heightMin = o.heightMax = s.height;
        o.weightMin = o.weightMax = s.weight;
        return o;
    };
    auto states = SpawnerGenerator8a::generate(0x1234ABCD5678EF01ULL, 2, 1, 1, 127, false);
    SpawnerSearcher8a searcher(observe(states[0].entitySeed), observe(states[1].entitySeed));
    searcher.startSearch(threads(), OpenCL::isAvailable(), 200000);
    std::vector<SpawnerResult8a> found;
    while (true)
    {
        bool searching = searcher.isSearching();
        auto part = searcher.getResults();
        found.insert(found.end(), part.begin(), part.end());
        if (!searching)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    QCOMPARE(found.size(), size_t(1));
    QCOMPARE(found[0].groupSeed, 0x1234ABCD5678EF01ULL);
    QCOMPARE(found[0].relation, SpawnerRelation8a::NextFill);
    QCOMPARE(found[0].position, u8(1));
}

// Five Mirelands Unown logged in Eden on 2026-11-21 (boot seed B7261D36C4B3AD2E, first EC 148 draws after boot),
// reduced to what a stock console shows: effort levels, nature, weight in 0.1 kg
static std::vector<StockUnown8a> stockBatch()
{
    return { { { 0, 0, 0, 0, 0, 0 }, 7, 57 }, { { 0, 2, 0, 1, 0, 3 }, 11, 56 }, { { 1, 0, 0, 0, 0, 2 }, 10, 48 },
             { { 0, 0, 2, 1, 2, 0 }, 11, 57 }, { { 0, 0, 0, 0, 0, 1 }, 21, 50 } };
}

void Gen8aTest::stockBootVerify()
{
    StockBootSeedSearcher8a searcher(stockBatch(), 148, 148, 4);
    QVERIFY(searcher.verify(0xB7261D36C4B3AD2EULL, 148));
    QVERIFY(!searcher.verify(0xB7261D36C4B3AD2EULL, 147));
    QVERIFY(!searcher.verify(0xB7261D36C4B3AD2FULL, 148));
    QCOMPARE(StockBootSeedSearcher8a::displayedWeight(213), static_cast<u16>(57));
    QCOMPARE(searcher.getAlignments(), static_cast<u64>(3 * 625 * 2));
}

void Gen8aTest::stockBootSearchGPU()
{
    if (std::getenv("POKEFINDER_LONG_TESTS") == nullptr)
    {
        QSKIP("Set POKEFINDER_LONG_TESTS=1 to run the stock boot seed search (GPU, several minutes)");
    }
    if (!OpenCL::isAvailable())
    {
        QSKIP("No OpenCL GPU");
    }
    StockBootSeedSearcher8a searcher(stockBatch(), 148, 148, 4);
    searcher.startSearch();
    std::vector<StockBootSeedResult8a> found;
    while (searcher.isSearching())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        auto part = searcher.getResults();
        found.insert(found.end(), part.begin(), part.end());
    }
    auto part = searcher.getResults();
    found.insert(found.end(), part.begin(), part.end());
    QVERIFY2(searcher.getError().empty(), searcher.getError().c_str());
    QCOMPARE(found.size(), static_cast<size_t>(1));
    QCOMPARE(found[0].seed, 0xB7261D36C4B3AD2EULL);
    QCOMPARE(found[0].advance, 148u);
}
