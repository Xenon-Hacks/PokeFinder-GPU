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

#include "IVSearcher5GPUTest.hpp"
#include <Core/Enum/DSType.hpp>
#include <Core/Enum/Encounter.hpp>
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Language.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Enum/Method.hpp>
#include <Core/Gen5/EncounterArea5.hpp>
#include <Core/Gen5/Encounters5.hpp>
#include <Core/Gen5/Profile5.hpp>
#include <Core/Gen5/Searchers/StaticSearcher5.hpp>
#include <Core/Gen5/Searchers/WildSearcher5.hpp>
#include <Core/Gen5/States/State5.hpp>
#include <Core/Gen5/States/WildState5.hpp>
#include <Core/Gen5/StaticTemplate5.hpp>
#include <Core/Util/DateTime.hpp>
#include <Core/Util/OpenCL.hpp>
#include <QTest>
#include <Test/Enum.hpp>
#include <algorithm>
#include <chrono>
#include <thread>

using IVs = std::array<u8, 6>;

static Profile5 getProfile(Game version)
{
    // Only searching without keypresses keeps the search short
    return Profile5("-", version, 12345, 54321, "", "", 0x0009bf123456, { true, false, false, false, false, false, false, false, false },
                    0x60, 6, 8, false, 0xc79, 0xc7a, false, false, DSType::DS, Language::English);
}

template <class Searcher>
static auto runSearch(Searcher &searcher)
{
    Date date(2025, 7, 14);
    searcher.setMaxProgress(searcher.getMaxProgress(date, date));
    searcher.startSearch(4, date, date);
    while (searcher.isSearching())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto results = searcher.getResults();
    std::vector<std::tuple<u32, u32, u64, u16, u16, u32, u32, u32, u64, IVs>> keys;
    for (const auto &result : results)
    {
        const auto &state = result.getState();
        keys.emplace_back(result.getDateTime().getDate().getJD(),
                          result.getDateTime().getTime().hour() * 3600u + result.getDateTime().getTime().minute() * 60u
                              + result.getDateTime().getTime().second(),
                          result.getInitialSeed(), toInt(result.getButtons()), result.getTimer0(), state.getAdvances(),
                          state.getIVAdvances(), state.getPID(), state.getLeadMask(), state.getIVs());
    }
    std::ranges::sort(keys);
    return keys;
}

void IVSearcher5GPUTest::initTestCase()
{
    if (!OpenCL::isAvailable())
    {
        QSKIP("No OpenCL device");
    }
}

void IVSearcher5GPUTest::searchWild_data()
{
    QTest::addColumn<Game>("version");
    QTest::addColumn<IVs>("min");
    QTest::addColumn<u8>("shiny");
    QTest::addColumn<u32>("initialIVAdvances");
    QTest::addColumn<u32>("maxIVAdvances");

    QTest::newRow("B2 loose") << Game::Black2 << IVs { 20, 20, 0, 0, 0, 20 } << u8(3) << 0u << 10u;
    QTest::newRow("B2 strict") << Game::Black2 << IVs { 31, 0, 31, 0, 31, 31 } << u8(255) << 0u << 50u;
    QTest::newRow("B2 offset") << Game::Black2 << IVs { 28, 28, 28, 28, 28, 28 } << u8(255) << 5u << 200u;
    QTest::newRow("B loose shiny") << Game::Black << IVs { 20, 0, 20, 0, 20, 0 } << u8(3) << 0u << 10u;
}

void IVSearcher5GPUTest::searchWild()
{
    QFETCH(Game, version);
    QFETCH(IVs, min);
    QFETCH(u8, shiny);
    QFETCH(u32, initialIVAdvances);
    QFETCH(u32, maxIVAdvances);

    Profile5 profile = getProfile(version);
    EncounterSettings5 settings = {};
    auto areas = Encounters5::getEncounters(Encounter::Grass, settings, &profile);
    QVERIFY(!areas.empty());

    std::array<bool, 25> natures;
    natures.fill(true);
    std::array<bool, 16> powers;
    powers.fill(true);
    StackVector<bool, 13> encounterSlots;
    encounterSlots.fill(true);

    WildStateFilter filter(255, 255, shiny, 1, 100, 0, 255, 0, 255, false, min, { 31, 31, 31, 31, 31, 31 }, natures, powers,
                           encounterSlots);
    WildGenerator5 generator(0, 100, 0, Method::Method5, { Lead::None, Lead::Synchronize }, { 0 }, false, false, areas[0], profile, filter,
                             true);

    QVERIFY(WildSearcher5GPU::isSupported(generator, initialIVAdvances, maxIVAdvances, false));
    WildSearcher5GPU gpu(initialIVAdvances, maxIVAdvances, generator, profile);
    QVERIFY2(gpu.isReady(), gpu.getError().data());
    WildSearcher5 cpu(initialIVAdvances, maxIVAdvances, generator, profile);

    auto gpuResults = runSearch(gpu);
    QVERIFY2(gpu.getError().empty(), gpu.getError().data());
    auto cpuResults = runSearch(cpu);
    QVERIFY(!cpuResults.empty());
    QCOMPARE(gpuResults.size(), cpuResults.size());
    QVERIFY(gpuResults == cpuResults);
}

void IVSearcher5GPUTest::searchWildCache()
{
    Profile5 profile = getProfile(Game::Black2);
    EncounterSettings5 settings = {};
    auto areas = Encounters5::getEncounters(Encounter::Grass, settings, &profile);

    std::array<bool, 25> natures;
    natures.fill(true);
    std::array<bool, 16> powers;
    powers.fill(true);
    StackVector<bool, 13> encounterSlots;
    encounterSlots.fill(true);

    WildStateFilter filter(255, 255, 3, 1, 100, 0, 255, 0, 255, false, { 25, 0, 25, 0, 25, 25 }, { 31, 31, 31, 31, 31, 31 }, natures,
                           powers, encounterSlots);
    WildGenerator5 generator(0, 300, 0, Method::Method5, { Lead::None }, { 0 }, false, false, areas[0], profile, filter, true);

    // Stand-in IV cache: what the MT path finds for this date, plus entries that never match
    WildSearcher5GPU mt(0, 30, generator, profile);
    auto mtResults = runSearch(mt);
    QVERIFY(!mtResults.empty());

    fph::MetaFphMap<u64, IVs> ivMap;
    for (const auto &result : mtResults)
    {
        ivMap.emplace((static_cast<u64>(std::get<6>(result)) << 32) | (std::get<2>(result) >> 32), std::get<9>(result));
    }
    for (u32 i = 0; i < 10000; i++)
    {
        ivMap.emplace((static_cast<u64>(i % 31) << 32) | (i * 2654435761u), IVs { 31, 31, 31, 31, 31, 31 });
    }

    WildSearcher5GPU gpu(0, 30, ivMap, generator, profile);
    QVERIFY2(gpu.isReady(), gpu.getError().data());
    WildSearcher5Fast cpu(0, 30, ivMap, generator, profile);

    auto gpuResults = runSearch(gpu);
    auto cpuResults = runSearch(cpu);
    QVERIFY(!cpuResults.empty());
    QCOMPARE(gpuResults.size(), cpuResults.size());
    QVERIFY(gpuResults == cpuResults);
}

void IVSearcher5GPUTest::searchStatic_data()
{
    QTest::addColumn<Game>("version");
    QTest::addColumn<bool>("roamer");

    QTest::newRow("B2 static") << Game::Black2 << false;
    QTest::newRow("B roamer") << Game::Black << true;
}

void IVSearcher5GPUTest::searchStatic()
{
    QFETCH(Game, version);
    QFETCH(bool, roamer);

    Profile5 profile = getProfile(version);

    const StaticTemplate5 *staticTemplate = nullptr;
    for (int type = 0; type < 8 && !staticTemplate; type++)
    {
        int size = 0;
        const StaticTemplate5 *templates = Encounters5::getStaticEncounters(type, &size);
        for (int i = 0; i < size; i++)
        {
            if ((templates[i].getVersion() & version) != Game::None && templates[i].getRoamer() == roamer && !templates[i].getEgg())
            {
                staticTemplate = &templates[i];
                break;
            }
        }
    }
    QVERIFY(staticTemplate);

    std::array<bool, 25> natures;
    natures.fill(true);
    std::array<bool, 16> powers;
    powers.fill(true);

    StateFilter filter(255, 255, 3, 1, 100, 0, 255, 0, 255, false, { 20, 0, 20, 20, 0, 20 }, { 31, 31, 31, 31, 31, 31 }, natures, powers);
    StaticGenerator5 generator(0, 200, 0, Method::Method5, { Lead::None }, { 0 }, *staticTemplate, profile, filter);

    StaticSearcher5GPU gpu(0, 15, generator, profile);
    QVERIFY2(gpu.isReady(), gpu.getError().data());
    StaticSearcher5 cpu(0, 15, generator, profile);

    auto gpuResults = runSearch(gpu);
    auto cpuResults = runSearch(cpu);
    QVERIFY(!cpuResults.empty());
    QCOMPARE(gpuResults.size(), cpuResults.size());
    QVERIFY(gpuResults == cpuResults);
}
