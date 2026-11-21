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
#include <Core/Gen8a/IVCalculator8a.hpp>
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

void Gen8aTest::ivCalculator()
{
    // stat, base, level, effort, nature, expected: from PLA-RNG-Research/ivcalc/pla_ivcalc.py, which matches the stats
    // the game stores for party Pokemon
    static const int vectors[][6] = { { 2, 37, 49, 2, 6, 75 }, { 0, 90, 18, 3, 16, 146 }, { 3, 75, 83, 0, 14, 165 }, { 3, 150, 50, 7, 18, 289 },
                                      { 3, 72, 12, 7, 7, 111 }, { 5, 25, 35, 8, 13, 74 }, { 3, 105, 93, 1, 21, 245 }, { 0, 75, 9, 6, 19, 125 },
                                      { 0, 45, 85, 0, 10, 202 }, { 5, 65, 12, 7, 20, 103 }, { 1, 120, 75, 2, 19, 243 }, { 4, 35, 5, 7, 22, 62 },
                                      { 1, 75, 75, 7, 23, 203 }, { 3, 55, 37, 8, 11, 122 }, { 1, 20, 21, 9, 3, 56 }, { 5, 120, 41, 5, 20, 196 },
                                      { 4, 110, 80, 3, 9, 220 }, { 2, 48, 68, 4, 16, 114 }, { 4, 55, 90, 8, 7, 182 }, { 1, 105, 2, 4, 10, 93 },
                                      { 2, 70, 5, 6, 8, 88 }, { 2, 64, 91, 6, 5, 197 }, { 0, 50, 22, 10, 0, 163 }, { 1, 125, 39, 1, 0, 173 },
                                      { 2, 47, 22, 9, 13, 98 }, { 1, 45, 53, 9, 14, 125 }, { 3, 115, 11, 9, 2, 166 }, { 5, 60, 13, 0, 2, 50 },
                                      { 4, 75, 68, 5, 4, 161 }, { 3, 100, 91, 2, 19, 254 }, { 1, 35, 3, 2, 10, 29 }, { 5, 33, 74, 1, 23, 88 },
                                      { 2, 51, 42, 5, 0, 102 }, { 1, 120, 8, 9, 1, 174 }, { 1, 125, 23, 5, 13, 166 }, { 0, 75, 11, 3, 19, 112 },
                                      { 3, 85, 61, 2, 12, 160 }, { 0, 35, 75, 7, 20, 199 }, { 5, 30, 57, 8, 23, 98 }, { 5, 105, 61, 9, 12, 245 },
                                      { 3, 60, 29, 0, 6, 75 }, { 3, 30, 15, 1, 6, 36 }, { 5, 35, 77, 9, 0, 128 }, { 3, 90, 68, 1, 23, 161 },
                                      { 4, 95, 91, 9, 6, 277 }, { 1, 70, 42, 8, 12, 152 }, { 5, 120, 26, 0, 9, 131 }, { 2, 65, 11, 5, 12, 82 },
                                      { 5, 85, 38, 1, 17, 112 }, { 5, 85, 48, 5, 16, 160 }, { 5, 45, 46, 4, 20, 94 }, { 1, 110, 63, 5, 12, 224 },
                                      { 2, 35, 27, 4, 12, 62 }, { 3, 100, 88, 8, 22, 279 }, { 5, 39, 78, 9, 11, 143 }, { 1, 20, 63, 8, 16, 82 },
                                      { 2, 120, 21, 1, 15, 130 }, { 3, 45, 100, 1, 19, 144 }, { 2, 105, 36, 8, 8, 208 }, { 5, 61, 53, 3, 11, 125 },
                                      { 1, 65, 65, 7, 7, 170 }, { 4, 25, 68, 3, 11, 74 }, { 0, 150, 15, 6, 2, 237 }, { 4, 48, 55, 0, 4, 82 },
                                      { 4, 80, 25, 10, 8, 179 }, { 0, 40, 60, 7, 21, 183 }, { 4, 56, 12, 2, 19, 55 }, { 5, 39, 9, 5, 1, 54 },
                                      { 1, 5, 84, 5, 2, 49 }, { 5, 20, 5, 9, 10, 46 }, { 2, 90, 96, 5, 10, 244 }, { 4, 55, 15, 9, 11, 100 },
                                      { 1, 70, 21, 4, 12, 98 }, { 3, 72, 94, 1, 3, 168 }, { 1, 120, 11, 4, 5, 122 }, { 5, 70, 26, 1, 1, 87 },
                                      { 4, 80, 95, 2, 5, 203 }, { 5, 105, 42, 4, 7, 160 }, { 3, 75, 15, 9, 22, 126 }, { 1, 5, 96, 2, 7, 50 },
                                      { 1, 79, 53, 2, 5, 129 }, { 3, 85, 8, 7, 18, 120 }, { 2, 60, 54, 6, 18, 132 }, { 5, 71, 73, 7, 11, 203 },
                                      { 1, 120, 40, 4, 21, 191 }, { 1, 65, 36, 7, 20, 126 }, { 4, 90, 66, 4, 6, 192 }, { 4, 70, 51, 2, 15, 124 },
                                      { 0, 50, 38, 6, 1, 148 }, { 2, 15, 65, 4, 11, 57 }, { 5, 112, 4, 9, 4, 149 }, { 0, 73, 24, 7, 18, 171 },
                                      { 3, 40, 73, 10, 18, 157 }, { 2, 65, 65, 0, 19, 125 }, { 2, 84, 14, 5, 20, 106 }, { 3, 110, 22, 10, 7, 219 },
                                      { 2, 107, 74, 5, 11, 221 }, { 2, 50, 96, 10, 21, 196 }, { 5, 120, 19, 10, 20, 227 }, { 0, 97, 5, 3, 16, 124 },
                                      { 3, 60, 47, 7, 17, 146 }, { 2, 48, 18, 10, 8, 123 }, { 3, 60, 95, 4, 18, 176 }, { 3, 50, 38, 4, 12, 93 },
                                      { 3, 100, 85, 10, 2, 314 }, { 5, 90, 57, 5, 4, 181 }, { 5, 101, 14, 5, 22, 115 }, { 5, 60, 59, 5, 15, 135 },
                                      { 3, 110, 27, 8, 16, 197 }, { 1, 69, 19, 8, 1, 126 }, { 4, 80, 86, 4, 6, 204 }, { 5, 48, 52, 4, 6, 105 },
                                      { 0, 85, 5, 7, 22, 148 }, { 2, 40, 41, 0, 0, 64 }, { 5, 55, 17, 9, 9, 103 }, { 0, 67, 6, 3, 10, 92 },
                                      { 0, 60, 54, 3, 0, 180 }, { 4, 100, 66, 3, 15, 196 }, { 0, 74, 55, 8, 4, 243 }, { 1, 50, 85, 10, 22, 195 },
                                      { 4, 53, 56, 10, 1, 169 }, { 3, 76, 89, 10, 6, 263 }, { 2, 45, 9, 7, 10, 76 }, { 1, 72, 8, 6, 22, 89 },
                                      { 2, 65, 40, 8, 8, 149 }, { 0, 103, 52, 3, 22, 245 }, { 1, 50, 15, 10, 24, 120 }, { 1, 160, 92, 5, 7, 379 },
                                      { 3, 35, 45, 8, 17, 101 }, { 3, 120, 79, 5, 19, 293 }, { 4, 85, 22, 7, 3, 141 }, { 4, 95, 44, 9, 5, 199 },
                                      { 1, 35, 29, 6, 23, 69 }, { 1, 55, 44, 2, 18, 94 }, { 0, 90, 57, 5, 18, 251 }, { 5, 48, 31, 9, 12, 108 },
                                      { 2, 55, 52, 5, 24, 119 }, { 0, 60, 16, 6, 21, 119 }, { 3, 120, 77, 0, 15, 254 }, { 4, 102, 92, 3, 0, 246 },
                                      { 5, 109, 78, 8, 10, 298 }, { 5, 71, 62, 9, 3, 185 }, { 1, 92, 15, 0, 14, 85 }, { 0, 44, 71, 0, 6, 174 },
                                      { 5, 65, 10, 0, 10, 61 }, { 3, 85, 91, 5, 15, 240 }, { 1, 135, 63, 4, 20, 240 }, { 3, 95, 60, 9, 24, 225 },
                                      { 4, 90, 86, 0, 16, 197 }, { 5, 100, 52, 7, 16, 213 }, { 5, 86, 21, 1, 18, 97 }, { 4, 80, 90, 2, 10, 196 },
                                      { 4, 40, 70, 6, 19, 108 }, { 3, 70, 71, 2, 9, 150 }, { 5, 48, 46, 5, 16, 102 }, { 5, 90, 52, 9, 8, 204 },
                                      { 1, 30, 71, 7, 14, 107 }, { 5, 50, 25, 7, 17, 95 }, { 5, 95, 9, 10, 9, 175 }, { 1, 65, 51, 1, 17, 114 },
                                      { 1, 40, 24, 7, 10, 80 }, { 1, 83, 91, 3, 14, 207 }, { 3, 100, 4, 8, 11, 134 }, { 0, 80, 68, 8, 15, 283 },
                                      { 0, 79, 75, 1, 14, 250 }, { 2, 84, 43, 1, 3, 129 }, { 3, 72, 55, 9, 14, 176 }, { 5, 65, 17, 0, 11, 70 },
                                      { 3, 30, 58, 5, 21, 84 }, { 5, 120, 55, 9, 3, 259 }, { 3, 41, 87, 10, 13, 165 }, { 0, 70, 93, 2, 7, 275 },
                                      { 1, 95, 36, 9, 17, 185 }, { 1, 72, 67, 0, 10, 127 }, { 0, 150, 22, 8, 10, 287 }, { 3, 80, 93, 4, 4, 214 },
                                      { 3, 80, 2, 9, 3, 107 }, { 1, 72, 9, 2, 20, 64 }, { 3, 72, 37, 8, 12, 149 }, { 3, 55, 91, 4, 2, 160 },
                                      { 3, 24, 64, 6, 10, 79 }, { 5, 115, 30, 4, 1, 164 }, { 5, 101, 14, 0, 17, 83 }, { 2, 110, 84, 6, 0, 267 },
                                      { 5, 65, 48, 0, 20, 103 }, { 4, 48, 9, 8, 15, 82 }, { 4, 45, 3, 0, 23, 35 }, { 2, 60, 18, 3, 11, 68 },
                                      { 4, 62, 85, 8, 11, 192 }, { 1, 5, 83, 3, 16, 45 }, { 2, 70, 13, 10, 13, 147 }, { 1, 115, 3, 1, 24, 91 },
                                      { 2, 70, 19, 8, 8, 128 }, { 0, 45, 42, 5, 16, 143 }, { 4, 41, 21, 9, 10, 87 }, { 1, 85, 3, 4, 23, 87 },
                                      { 4, 40, 6, 9, 19, 69 }, { 4, 60, 19, 4, 17, 84 }, { 1, 112, 4, 2, 22, 94 }, { 5, 45, 67, 9, 4, 140 },
                                      { 2, 48, 57, 10, 8, 166 }, { 5, 35, 39, 10, 4, 116 }, { 3, 90, 57, 9, 22, 212 }, { 4, 135, 87, 10, 17, 397 },
                                      { 0, 40, 69, 4, 18, 181 }, { 2, 40, 44, 8, 3, 106 }, { 0, 75, 1, 9, 14, 132 }, { 5, 102, 90, 9, 16, 291 },
                                      { 0, 70, 89, 0, 4, 257 }, { 0, 70, 43, 5, 17, 187 }, { 3, 80, 72, 9, 6, 216 }, { 3, 150, 97, 4, 12, 367 },
                                      { 4, 120, 55, 7, 23, 266 }, { 3, 15, 68, 5, 13, 60 }, { 4, 100, 49, 9, 21, 229 }, { 4, 70, 100, 10, 6, 264 },
                                      { 4, 25, 21, 6, 13, 49 }, { 1, 52, 34, 8, 10, 109 }, { 0, 120, 39, 3, 13, 238 }, { 4, 65, 34, 7, 10, 131 },
                                      { 3, 120, 80, 3, 18, 258 }, { 2, 48, 22, 2, 24, 63 }, { 1, 45, 8, 10, 20, 100 }, { 4, 41, 67, 3, 18, 100 },
                                      { 1, 65, 85, 4, 9, 174 }, { 1, 20, 89, 7, 2, 101 }, { 0, 150, 79, 8, 16, 452 }, { 5, 45, 15, 9, 21, 87 },
                                      { 5, 95, 92, 0, 20, 216 }, { 2, 58, 37, 5, 23, 106 }, { 1, 109, 88, 9, 1, 322 }, { 3, 72, 2, 0, 14, 50 },
                                      { 2, 77, 52, 3, 21, 128 }, { 2, 55, 76, 10, 14, 197 }, { 2, 50, 71, 6, 4, 134 }, { 3, 95, 9, 1, 7, 85 },
                                      { 1, 69, 95, 8, 12, 221 }, { 3, 65, 38, 3, 9, 104 }, { 2, 45, 53, 0, 0, 82 }, { 3, 50, 100, 0, 20, 140 },
                                      { 1, 72, 70, 1, 2, 161 }, { 4, 107, 79, 9, 4, 263 }, { 3, 72, 64, 10, 2, 219 }, { 4, 80, 34, 4, 1, 128 },
                                      { 2, 60, 11, 4, 11, 69 }, { 2, 100, 60, 8, 7, 244 }, { 0, 28, 10, 3, 14, 52 }, { 0, 60, 15, 6, 17, 118 },
                                      { 3, 80, 93, 10, 3, 263 }, { 5, 85, 97, 5, 13, 250 }, { 3, 87, 49, 7, 21, 186 }, { 4, 100, 10, 6, 10, 120 },
                                      { 2, 47, 54, 4, 9, 112 }, { 3, 105, 1, 3, 2, 88 }, { 0, 70, 81, 8, 18, 290 }, { 1, 80, 5, 0, 3, 65 },
                                      { 1, 140, 68, 7, 14, 313 }, { 1, 45, 12, 4, 17, 61 }, { 2, 118, 63, 2, 19, 215 }, { 2, 130, 34, 4, 5, 205 },
                                      { 2, 40, 4, 1, 0, 35 }, { 5, 70, 53, 8, 6, 167 }, { 1, 100, 18, 9, 3, 170 }, { 2, 48, 59, 7, 5, 137 },
                                      { 1, 65, 88, 6, 9, 183 }, { 4, 42, 54, 0, 5, 80 }, { 0, 120, 100, 7, 3, 441 }, { 2, 42, 96, 7, 3, 156 },
                                      { 2, 65, 13, 0, 2, 59 }, { 5, 50, 24, 6, 10, 88 }, { 3, 120, 43, 0, 16, 179 }, { 4, 90, 18, 7, 20, 149 },
                                      { 5, 95, 85, 7, 4, 260 }, { 3, 80, 67, 6, 8, 170 }, { 2, 168, 75, 4, 10, 346 }, { 1, 45, 94, 2, 12, 132 },
                                      { 1, 65, 12, 4, 9, 80 }, { 1, 100, 61, 4, 18, 200 }, { 4, 42, 80, 7, 13, 140 }, { 5, 48, 26, 8, 13, 104 },
                                      { 2, 118, 6, 4, 16, 112 }, { 4, 80, 89, 6, 16, 216 }, { 3, 72, 11, 9, 6, 117 }, { 0, 70, 15, 2, 8, 111 },
                                      { 5, 30, 27, 10, 1, 96 }, { 1, 106, 64, 10, 24, 290 }, { 4, 50, 39, 6, 6, 100 }, { 3, 95, 56, 6, 16, 204 },
                                      { 0, 77, 53, 3, 11, 205 }, { 2, 70, 11, 7, 3, 107 }, { 4, 60, 60, 8, 5, 158 }, { 1, 52, 43, 2, 4, 96 },
                                      { 2, 44, 89, 10, 5, 191 }, { 0, 250, 67, 5, 14, 561 }, { 4, 80, 80, 0, 11, 170 }, { 1, 130, 64, 2, 20, 216 },
                                      { 5, 30, 49, 10, 5, 113 }, { 2, 75, 17, 5, 11, 95 }, { 4, 60, 44, 1, 6, 99 }, { 3, 83, 63, 5, 16, 191 },
                                      { 5, 48, 98, 2, 18, 142 }, { 4, 36, 50, 7, 7, 102 }, { 5, 30, 87, 6, 0, 109 }, { 4, 36, 42, 5, 4, 75 },
                                      { 5, 95, 66, 8, 3, 231 }, { 5, 40, 14, 3, 1, 50 }, { 1, 72, 50, 2, 9, 126 }, { 4, 55, 42, 0, 22, 90 },
                                      { 0, 120, 64, 2, 9, 299 }, { 0, 79, 86, 1, 14, 274 }, { 1, 135, 54, 4, 8, 241 }, { 5, 115, 53, 9, 10, 262 },
                                      { 0, 85, 71, 7, 4, 296 }, { 3, 92, 53, 8, 4, 205 }, { 0, 25, 57, 6, 20, 137 }, { 3, 25, 67, 5, 16, 85 },
                                      { 5, 48, 7, 2, 4, 47 }, { 0, 120, 38, 0, 19, 218 }, { 0, 60, 66, 2, 17, 201 }, { 1, 55, 47, 0, 20, 82 },
                                      { 1, 120, 31, 3, 19, 159 }, { 2, 120, 46, 3, 22, 189 }, { 3, 72, 9, 5, 5, 87 }, { 2, 120, 13, 10, 12, 215 },
                                      { 0, 75, 78, 6, 2, 273 }, { 5, 70, 35, 2, 11, 110 }, { 1, 115, 49, 6, 2, 224 }, { 5, 40, 86, 4, 17, 116 },
                                      { 3, 15, 92, 10, 22, 104 }, { 1, 35, 69, 9, 11, 120 }, { 0, 48, 40, 4, 0, 142 }, { 3, 37, 62, 6, 3, 96 },
                                      { 5, 60, 57, 2, 16, 117 }, { 4, 48, 52, 0, 8, 86 }, { 1, 72, 20, 9, 9, 129 }, { 5, 95, 10, 8, 0, 138 },
                                      { 2, 110, 62, 8, 23, 252 }, { 1, 93, 58, 10, 10, 239 }, { 4, 60, 92, 6, 15, 178 }, { 5, 80, 62, 9, 21, 201 },
                                      { 5, 35, 19, 4, 24, 56 }, { 4, 41, 38, 8, 4, 97 }, { 5, 65, 38, 4, 23, 114 }, { 5, 120, 13, 3, 21, 123 },
                                      { 1, 125, 40, 6, 17, 206 }, { 2, 35, 92, 1, 2, 108 }, { 4, 48, 61, 6, 1, 120 }, { 3, 72, 13, 9, 7, 120 },
                                      { 1, 65, 68, 6, 5, 147 }, { 3, 85, 98, 3, 8, 204 }, { 2, 45, 36, 0, 10, 65 }, { 2, 107, 72, 10, 8, 323 },
                                      { 1, 66, 6, 0, 17, 51 }, { 1, 25, 91, 4, 7, 97 }, { 5, 36, 10, 3, 18, 42 }, { 5, 90, 20, 10, 0, 187 },
                                      { 4, 130, 84, 1, 22, 298 }, { 1, 25, 28, 3, 4, 47 }, { 3, 60, 44, 1, 23, 91 }, { 3, 65, 78, 1, 19, 159 },
                                      { 0, 40, 38, 1, 13, 113 }, { 4, 50, 83, 7, 17, 161 }, { 4, 96, 40, 7, 17, 186 }, { 5, 85, 36, 1, 3, 119 },
                                      { 1, 105, 40, 9, 5, 195 }, { 2, 160, 25, 10, 16, 280 }, { 3, 62, 55, 2, 3, 108 }, { 1, 83, 19, 10, 16, 175 },
                                      { 1, 100, 30, 0, 18, 118 }, { 3, 120, 34, 0, 19, 161 }, { 4, 49, 29, 6, 16, 88 }, { 5, 78, 48, 5, 23, 148 },
                                      { 3, 120, 49, 5, 20, 213 }, { 5, 36, 15, 1, 1, 42 }, { 5, 50, 9, 2, 5, 51 }, { 4, 80, 65, 8, 0, 202 },
                                      { 1, 100, 68, 3, 4, 215 }, { 3, 40, 10, 8, 21, 74 }, { 4, 44, 8, 4, 17, 56 }, { 4, 45, 72, 6, 3, 126 },
                                      { 2, 40, 55, 2, 16, 80 }, { 2, 70, 100, 7, 1, 213 }, { 2, 70, 95, 3, 17, 186 }, { 0, 50, 62, 1, 24, 173 },
                                      { 3, 45, 12, 0, 12, 42 }, { 0, 85, 98, 8, 0, 361 }, { 4, 60, 84, 8, 19, 176 }, { 1, 20, 12, 5, 16, 35 },
                                      { 2, 37, 29, 3, 9, 62 }, { 2, 48, 33, 9, 14, 111 }, { 2, 80, 10, 4, 13, 93 }, { 2, 48, 19, 6, 6, 77 },
                                      { 1, 72, 97, 0, 3, 194 }, { 4, 80, 22, 5, 0, 113 }, { 4, 50, 82, 0, 3, 121 }, { 2, 70, 95, 2, 18, 183 },
                                      { 0, 48, 30, 4, 10, 123 }, { 3, 85, 70, 4, 13, 176 }, { 5, 80, 99, 4, 2, 207 }, { 2, 80, 27, 3, 23, 107 } };
    for (const auto &v : vectors)
    {
        QCOMPARE(static_cast<int>(IVCalculator8a::calcStat(v[0], static_cast<u8>(v[1]), static_cast<u8>(v[2]), static_cast<u8>(v[3]),
                                                           static_cast<u8>(v[4]))),
                 v[5]);
    }

    QCOMPARE(IVCalculator8a::getEntries().size(), static_cast<size_t>(318));
    QCOMPARE(IVCalculator8a::effortLevels({ 0, 29, 0, 21, 7, 31 }), (std::array<u8, 6> { 0, 2, 0, 1, 0, 3 }));

    // Unown level 25, fresh catch (grit 0) with effort levels 0/2/0/1/0/3: exact bands
    const auto *unown = IVCalculator8a::getEntry(201, 0);
    QVERIFY(unown != nullptr);
    std::array<int, 6> unknown = { -1, -1, -1, -1, -1, -1 };
    auto fresh = IVCalculator8a::analyze(unown->stats, 25, 11, { 0, 2, 0, 1, 0, 3 }, unknown, { 0, 0, 0, 0, 0, 0 }, 0);
    QVERIFY(fresh.errors.empty());
    QCOMPARE(fresh.stats[0].ranges.size(), static_cast<size_t>(1));
    QCOMPARE(static_cast<int>(fresh.stats[0].ranges[0].max), 19);
    QCOMPARE(static_cast<int>(fresh.stats[1].ranges[0].min), 26);
    QCOMPARE(static_cast<int>(fresh.stats[1].ranges[0].max), 30);
    QCOMPARE(static_cast<int>(fresh.stats[5].ranges[0].min), 31);

    // Grit unknown: effort level 2 can be IV 0-30 (bands 0, 1, 2 merged)
    auto loose = IVCalculator8a::analyze(unown->stats, 25, 11, { 0, 2, 0, 1, 0, 3 }, unknown, unknown, 0);
    QCOMPARE(static_cast<int>(loose.stats[1].ranges[0].min), 0);
    QCOMPARE(static_cast<int>(loose.stats[1].ranges[0].max), 30);

    // Three guaranteed 31s and exactly three stats that can be 31: they are 31
    auto legend = IVCalculator8a::analyze(unown->stats, 25, 11, { 3, 2, 4, 1, 0, 3 }, unknown, unknown, 3);
    QVERIFY(legend.errors.empty());
    QCOMPARE(legend.forced.size(), static_cast<size_t>(3));
    QCOMPARE(static_cast<int>(legend.stats[2].ranges[0].min), 31);

    // A stat that no effort level gives is reported
    std::array<int, 6> stats = unknown;
    stats[0] = 999;
    QVERIFY(!IVCalculator8a::analyze(unown->stats, 25, 11, unknown, stats, unknown, 0).errors.empty());
}
