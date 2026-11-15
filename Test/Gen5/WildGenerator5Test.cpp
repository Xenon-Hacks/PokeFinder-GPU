/*
 * This file is part of PokéFinder
 * Copyright (C) 2017-2024 by Admiral_Fish, bumba, and EzPzStreamz
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

#include "WildGenerator5Test.hpp"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Gen5/EncounterArea5.hpp>
#include <Core/Gen5/Encounters5.hpp>
#include <Core/Gen5/Generators/WildGenerator5.hpp>
#include <Core/Gen5/Profile5.hpp>
#include <Core/Gen5/States/WildState5.hpp>
#include <QTest>
#include <Test/Data.hpp>
#include <Test/Enum.hpp>

static bool operator==(const WildState5 &left, const json &right)
{
    return left.getPID() == right["pid"].get<u32>() && left.getStats() == right["stats"].get<std::array<u16, 6>>()
        && left.getAbilityIndex() == right["abilityIndex"].get<u16>() && left.getIVs() == right["ivs"].get<std::array<u8, 6>>()
        && left.getAbility() == right["ability"].get<u8>() && left.getCharacteristic() == right["characteristic"].get<u8>()
        && left.getGender() == right["gender"].get<u8>() && left.getHiddenPower() == right["hiddenPower"].get<u8>()
        && left.getHiddenPowerStrength() == right["hiddenPowerStrength"].get<u8>() && left.getLevel() == right["level"].get<u8>()
        && left.getNature() == right["nature"].get<u8>() && left.getShiny() == right["shiny"].get<u8>()
        && left.getItem() == right["item"].get<u16>() && left.getSpecie() == right["specie"].get<u16>()
        && left.getEncounterSlot() == right["encounterSlot"].get<u8>() && left.getForm() == right["form"].get<u8>()
        && left.getAdvances() == right["advances"].get<u32>() && left.isValid() == right["valid"].get<bool>()
        && left.getPhenomenon() == right["phenomenon"].get<bool>() && left.getChatot() == right["chatot"].get<u8>()
        && left.getNeedle() == right["needle"].get<u8>();
}

void WildGenerator5Test::generate_data()
{
    QTest::addColumn<u64>("seed");
    QTest::addColumn<Encounter>("encounter");
    QTest::addColumn<Lead>("lead");
    QTest::addColumn<Game>("version");
    QTest::addColumn<PassPower>("luckyPower");
    QTest::addColumn<bool>("swarm");
    QTest::addColumn<int>("location");
    QTest::addColumn<std::string>("results");

    json data = readData("wild5", "generate");
    for (const auto &d : data)
    {
        QTest::newRow(d["name"].get<std::string>().data())
            << d["seed"].get<u64>() << d["encounter"].get<Encounter>() << d["lead"].get<Lead>() << d["version"].get<Game>()
            << d["luckyPower"].get<PassPower>() << d["swarm"].get<bool>() << d["location"].get<int>() << d["results"].get<json>().dump();
    }
}

void WildGenerator5Test::generate()
{
    QFETCH(u64, seed);
    QFETCH(Encounter, encounter);
    QFETCH(Lead, lead);
    QFETCH(Game, version);
    QFETCH(PassPower, luckyPower);
    QFETCH(bool, swarm);
    QFETCH(int, location);
    QFETCH(std::string, results);

    json j = json::parse(results);

    std::array<u8, 6> min;
    min.fill(0);

    std::array<u8, 6> max;
    max.fill(31);

    std::array<bool, 25> natures;
    natures.fill(true);

    std::array<bool, 16> powers;
    powers.fill(true);

    StackVector<bool, 13> encounterSlots;
    encounterSlots.fill(true);

    Profile5 profile("-", version, 12345, 54321, "", "", 0, { false, false, false, false, false, false, false, false, false }, 0, 0, 0,
                     false, 0, 0, false, false, false, false, DSType::DS, Language::English);
    EncounterSettings5 settings = { };
    settings.swarm = swarm;

    auto areas = Encounters5::getEncounters(encounter, settings, &profile);
    auto area = std::ranges::find_if(areas, [location](const auto &area) { return area.getLocation() == location; });

    WildStateFilter filter(255, 255, 255, 1, 100, 0, 255, 0, 255, false, min, max, natures, powers, encounterSlots);
    // These vectors were made when a Pass Power added the 2 MT outputs that really come from the C-Gear
    WildGenerator5 generator(0, 9, 0, Method::Method5, lead, luckyPower, luckyPower != PassPower::None, *area, profile, filter);

    auto states = generator.generate(seed, 0, 0);
    QCOMPARE(states.size(), j.size());

    for (size_t i = 0; i < states.size(); i++)
    {
        const auto &state = states[i];
        QVERIFY(state == j[i]);
    }
}

static WildStateFilter allFilter()
{
    std::array<u8, 6> min;
    min.fill(0);

    std::array<u8, 6> max;
    max.fill(31);

    std::array<bool, 25> natures;
    natures.fill(true);

    std::array<bool, 16> powers;
    powers.fill(true);

    StackVector<bool, 13> encounterSlots;
    encounterSlots.fill(true);

    return WildStateFilter(255, 255, 255, 1, 100, 0, 255, 0, 255, false, min, max, natures, powers, encounterSlots);
}

static Profile5 makeProfile(Game version)
{
    return Profile5("-", version, 12345, 54321, "", "", 0, { false, false, false, false, false, false, false, false, false }, 0, 0, 0,
                    false, 0, 0, false, false, false, false, DSType::DS, Language::English);
}

static std::vector<WildState5> generateIVs(Game version, PassPower luckyPower, bool cgear, u32 maxIVAdvances)
{
    Profile5 profile = makeProfile(version);
    EncounterSettings5 settings = {};
    auto areas = Encounters5::getEncounters(Encounter::Grass, settings, &profile);

    WildGenerator5 generator(0, 0, 0, Method::Method5, Lead::None, luckyPower, cgear, areas[0], profile, allFilter());
    return generator.generate(0x1234567890ABCDEF, 0, maxIVAdvances);
}

void WildGenerator5Test::passPowerIVs_data()
{
    QTest::addColumn<PassPower>("luckyPower");

    QTest::newRow("Level 1") << PassPower::Level1;
    QTest::newRow("Level 2") << PassPower::Level2;
    QTest::newRow("Level 3") << PassPower::Level3;
}

void WildGenerator5Test::passPowerIVs()
{
    QFETCH(PassPower, luckyPower);

    // ROM + BizHawk (Black 2, Route 20): a Pass Power does not move the IVs, they come from the same MT outputs as without one
    auto none = generateIVs(Game::Black2, PassPower::None, false, 20);
    auto power = generateIVs(Game::Black2, luckyPower, false, 20);
    QCOMPARE(none.size(), size_t(21));
    QCOMPARE(power.size(), none.size());
    for (size_t i = 0; i < none.size(); i++)
    {
        QCOMPARE(power[i].getIVs(), none[i].getIVs());
    }
}

void WildGenerator5Test::cgearIVs()
{
    // Answering yes to the C-Gear prompt at Continue uses 2 MT outputs before the IVs on Black 2/White 2
    auto off = generateIVs(Game::Black2, PassPower::None, false, 22);
    auto on = generateIVs(Game::Black2, PassPower::None, true, 20);
    QCOMPARE(off.size(), size_t(23));
    QCOMPARE(on.size(), size_t(21));
    for (size_t i = 0; i < on.size(); i++)
    {
        QCOMPARE(on[i].getIVs(), off[i + 2].getIVs());
    }

    // Black/White ignore the option (the C-Gear reseeds the MT there instead)
    auto bwOff = generateIVs(Game::Black, PassPower::None, false, 20);
    auto bwOn = generateIVs(Game::Black, PassPower::None, true, 20);
    QCOMPARE(bwOn.size(), bwOff.size());
    for (size_t i = 0; i < bwOn.size(); i++)
    {
        QCOMPARE(bwOn[i].getIVs(), bwOff[i].getIVs());
    }
}

void WildGenerator5Test::ivPrefilterOffset()
{
    for (Game version : { Game::Black, Game::Black2 })
    {
        Profile5 profile = makeProfile(version);
        EncounterSettings5 settings = {};
        auto areas = Encounters5::getEncounters(Encounter::Grass, settings, &profile);

        for (bool cgear : { false, true })
        {
            u8 expected = version == Game::Black2 ? (cgear ? 4 : 2) : 0;
            WildGenerator5 none(0, 0, 0, Method::Method5, Lead::None, PassPower::None, cgear, areas[0], profile, allFilter());
            WildGenerator5 power(0, 0, 0, Method::Method5, Lead::None, PassPower::Level3, cgear, areas[0], profile, allFilter());
            QCOMPARE(none.getIVPrefilter().offset, expected);
            QCOMPARE(power.getIVPrefilter().offset, expected);
        }
    }
}
