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
#include <Core/Gen7/Gen7.hpp>
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
