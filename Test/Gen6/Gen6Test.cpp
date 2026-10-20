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

#include "Gen6Test.hpp"
#include <Core/Enum/Game.hpp>
#include <Core/Enum/Lead.hpp>
#include <Core/Gen6/EncounterArea6.hpp>
#include <Core/Gen6/Encounters6.hpp>
#include <Core/Gen6/Generators/EggGenerator6.hpp>
#include <Core/Gen6/Generators/IDGenerator6.hpp>
#include <Core/Gen6/Generators/StaticGenerator6.hpp>
#include <Core/Gen6/Generators/WildGenerator6.hpp>
#include <Core/Gen6/Pokemon6.hpp>
#include <Core/Gen6/Profile6.hpp>
#include <Core/Gen6/Save6.hpp>
#include <Core/Gen6/Searchers/SeedSearcher6.hpp>
#include <Core/Gen6/SeedWindow6.hpp>
#include <Core/Gen6/StaticTemplate6.hpp>
#include <Core/Gen6/States/EggState6.hpp>
#include <Core/Gen6/States/State6.hpp>
#include <Core/Gen6/WC6.hpp>
#include <Core/Parents/PersonalLoader.hpp>
#include <Core/Parents/Slot.hpp>
#include <Core/Util/Translator.hpp>
#include <Core/Parents/States/IDState.hpp>
#include <Core/Util/OpenCL.hpp>
#include <QFile>
#include <QTest>
#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

using IVs = std::array<u8, 6>;
using Outputs = std::vector<u32>;

Q_DECLARE_METATYPE(IVs)
Q_DECLARE_METATYPE(Outputs)

namespace
{
    // Javier's saves
    const Profile6 profileX("X", Game::X, 35323, 15424, false, 0xcc72d50f);
    const Profile6 profileY("Y", Game::Y, 28952, 46381, true, 0xc2a451a2);

    /**
     * @brief Replays recorded MT outputs
     */
    struct ReplayRNG
    {
        const u32 *outputs;
        u32 index = 0;

        u32 next()
        {
            return outputs[index++];
        }
    };

    StateFilter allFilter()
    {
        std::array<bool, 25> natures;
        natures.fill(true);
        std::array<bool, 16> powers;
        powers.fill(true);
        return StateFilter(255, 255, 255, 1, 100, 0, 255, 0, 255, false, { 0, 0, 0, 0, 0, 0 }, { 31, 31, 31, 31, 31, 31 }, natures, powers);
    }

    const StaticTemplate6 *findStatic(u16 specie)
    {
        for (int category = 0; category < 5; category++)
        {
            int size;
            const StaticTemplate6 *templates = Encounters6::getStaticEncounters(category, &size);
            for (int i = 0; i < size; i++)
            {
                if (templates[i].getSpecie() == specie)
                {
                    return &templates[i];
                }
            }
        }
        return nullptr;
    }

    template <class State>
    void compare(const State &state, u32 ec, u32 pid, const IVs &ivs, u8 ability, u8 nature, u8 gender)
    {
        QCOMPARE(state.getEC(), ec);
        QCOMPARE(state.getPID(), pid);
        QCOMPARE(state.getIVs(), ivs);
        QCOMPARE(state.getAbility(), ability);
        QCOMPARE(state.getNature(), nature);
        QCOMPARE(state.getGender(), gender);
    }
}

void Gen6Test::initTestCase()
{
    // Encounter table names come from the translator
    Translator::init("en");
}

void Gen6Test::liveStatic_data()
{
    QTest::addColumn<bool>("y");
    QTest::addColumn<u16>("specie");
    QTest::addColumn<Outputs>("outputs");
    QTest::addColumn<u32>("ec");
    QTest::addColumn<u32>("pid");
    QTest::addColumn<IVs>("ivs");
    QTest::addColumn<u8>("ability");
    QTest::addColumn<u8>("nature");
    QTest::addColumn<u8>("gender");

    // MT outputs from the EC on, rebuilt from the logged RAM state (results/*.json)
    QTest::newRow("Zygarde X zy1")
        << false << u16(718)
        << Outputs { 0x60BCEC0A, 0x719436F9, 0x4C0E9B00, 0x6D5589C8, 0xED645133, 0xD6DF858E, 0xA48E8E23, 0x4476431A, 0x0422BDFB,
                     0xB439F573, 0x45B18624, 0xB2E9B6BE }
        << 0x60BCEC0Au << 0x719436F9u << IVs { 26, 31, 31, 20, 8, 31 } << u8(0) << u8(0) << u8(2);
    QTest::newRow("Xerneas X xer2")
        << false << u16(716)
        << Outputs { 0xC480C74E, 0xA29E396C, 0x2C149918, 0x566FB703, 0xCBE9DC16, 0x3C35D6AB, 0x81EF4AEE, 0x35D29967, 0x76207425,
                     0x9C05A0A0, 0x6B6145A4, 0x440502AD }
        << 0xC480C74Eu << 0xA29E396Cu << IVs { 7, 31, 31, 16, 31, 6 } << u8(0) << u8(11) << u8(2);
    QTest::newRow("Yveltal Y yv1 (re-roll)")
        << true << u16(717)
        << Outputs { 0x531460EC, 0xA14004CB, 0x12CD439E, 0x98B277C9, 0x1F1EDBDF, 0x7BD56CD4, 0x9B95325C, 0x602F05D0, 0xC08B949E,
                     0x6E6938CA, 0x97594A84, 0x88D640FF }
        << 0x531460ECu << 0xA14004CBu << IVs { 31, 19, 31, 31, 12, 24 } << u8(0) << u8(10) << u8(2);
    QTest::newRow("Tyrunt X x_fossil")
        << false << u16(696)
        << Outputs { 0xF7B52B23, 0xF1529697, 0x48A3429C, 0x2E1E1717, 0x4D1DCA4C, 0x9524698E, 0x74808FD9, 0x99216600, 0x1374CEC4,
                     0x46436D90, 0x186C900A, 0xFFF295AC }
        << 0xF7B52B23u << 0xF1529697u << IVs { 9, 5, 9, 18, 14, 19 } << u8(0) << u8(6) << u8(1);
    QTest::newRow("Tyrunt Y fossil1 (Shiny Charm, 1 roll)")
        << true << u16(696)
        << Outputs { 0x2339FEA1, 0x81E28810, 0x6D0D6752, 0x48D368E9, 0xEC199A0F, 0xE1A04B29, 0x501C773A, 0xB4DA33AC, 0xB8AF20B5,
                     0x2C15B7DB, 0xF9DE8E6D, 0x58F7AC08 }
        << 0x2339FEA1u << 0x81E28810u << IVs { 13, 9, 29, 28, 10, 22 } << u8(1) << u8(4) << u8(0);
}

void Gen6Test::liveStatic()
{
    QFETCH(bool, y);
    QFETCH(u16, specie);
    QFETCH(Outputs, outputs);
    QFETCH(u32, ec);
    QFETCH(u32, pid);
    QFETCH(IVs, ivs);
    QFETCH(u8, ability);
    QFETCH(u8, nature);
    QFETCH(u8, gender);

    const Profile6 &profile = y ? profileY : profileX;
    const StaticTemplate6 *staticTemplate = findStatic(specie);
    QVERIFY(staticTemplate != nullptr);

    Gen6::Request6 request = StaticGenerator6::getRequest(*staticTemplate, profile, Lead::None);

    ReplayRNG rng { outputs.data() };
    auto result = Gen6::generate(rng, request, staticTemplate->getInfo(), profile.getTID(), profile.getSID());
    State6 state(0, result, staticTemplate->getLevel(), 0, staticTemplate->getInfo());
    compare(state, ec, pid, ivs, ability, nature, gender);
}

void Gen6Test::liveEvent()
{
    // Garchomp card received on X 1.0 (results/x_wc.json): random IVs, fixed ability and gender, no ability/gender draws
    Outputs outputs = { 0xADA3AA8B, 0x0007ACBF, 0x04F69296, 0x28C1D388, 0xA014F7A6, 0x01C5A6A9,
                        0xFC6CF4C7, 0xD39215EB, 0x4D972997, 0x1E1B709E, 0x7A7AE117, 0xFB299304 };
    WC6 wc6(445, 0, 48, 0, 0, true, 0, Gen6::PIDType::Random, 0, { 255, 255, 255, 255, 255, 255 }, 0, 0, Gen6::Random, 0, false);

    Gen6::Request6 request;
    request.ivs = wc6.getIVs();
    request.ability = wc6.getAbility();
    request.gender = wc6.getGender();

    const PersonalInfo *info = PersonalLoader::getPersonal(Game::X, 445);
    ReplayRNG rng { outputs.data() };
    auto result = Gen6::generate(rng, request, info, profileX.getTID(), profileX.getSID());

    QCOMPARE(result.ec, 0xADA3AA8Bu);
    QCOMPARE(result.pid, 0x0007ACBFu);
    QCOMPARE(result.ivs, (IVs { 0, 5, 20, 0, 31, 26 }));
    QCOMPARE(result.nature, u8(7)); // Relaxed
    QCOMPARE(rng.index, 9u); // EC, PID, 6 IVs, nature
}

void Gen6Test::liveFriendSafari()
{
    // Magcargo in a Friend Safari on X 1.0 (results/fs1.json): 5 PID rolls, 2 perfect IVs, ability >> 31, nature, gender
    Outputs outputs = { 0x93E42900, 0x70C10283, 0x2289C3C0, 0x2FD095DD, 0x02A23B0D, 0x7F329CDD, 0xB00C2280, 0x3DBF1905,
                        0xCD5722CD, 0xCFCE0B2F, 0xE9212199, 0xB334B86A, 0x9A29F895, 0x0BBC1BAD, 0xDEA2E9AF, 0xE16E91F1 };

    Gen6::Request6 request;
    request.pidRolls = WildGenerator6::getPIDRolls(Encounter6::FriendSafari, false, false);
    request.ivCount = 2;

    const PersonalInfo *info = PersonalLoader::getPersonal(Game::X, 219);
    ReplayRNG rng { outputs.data() };
    auto result = Gen6::generate(rng, request, info, profileX.getTID(), profileX.getSID());
    State6 state(0, result, 30, 0, info);
    compare(state, 0x93E42900, 0x7F329CDD, { 25, 31, 25, 29, 31, 22 }, 1, 1, 0);
    QCOMPARE(rng.index, 15u); // EC, 5 PID, 2 picks, 4 IVs, ability, nature, gender (report 4.2h counts 14)
}

void Gen6Test::liveEgg()
{
    // Y daycare: Snivy (male, Everstone, Timid, hidden ability) + Ditto (Destiny Knot), Masuda + Shiny Charm.
    // The egg accepted live on Y 1.5 matched field for field (report 0.3)
    Daycare daycare({ { { 31, 31, 31, 21, 30, 31 }, { 31, 31, 31, 31, 31, 31 } } }, { 2, 0 }, { 0, 3 }, { 1, 8 }, { 10, 0 }, 495, true, 20);
    EggGenerator6 generator(0, 0, daycare, profileY, allFilter());

    EggState6 state = generator.generate((static_cast<u64>(0xF4338068) << 32) | 0xA5EC7C90);
    compare(state, 0x23B61E66, 0xFABBA292, { 31, 3, 31, 21, 31, 31 }, 1, 10, 0);
    QVERIFY(!state.getMainPID());
}

void Gen6Test::staticGenerator()
{
    // Reference values from an independent Python port of the research recipe (seed = X Misc[0x00] + 0xDDE41DE5 ms, report A2)
    const StaticTemplate6 *zygarde = findStatic(718);
    StaticGenerator6 generator(300, 2, zygarde->getDelay(), Lead::None, *zygarde, profileX, allFilter());
    auto states = generator.generate(0xAA56F2F4);
    QCOMPARE(states.size(), size_t(3));

    compare(states[0], 3512947425, 2641942831, { 31, 8, 31, 31, 26, 27 }, 0, 18, 2);
    compare(states[1], 2641942831, 1913235050, { 31, 31, 26, 31, 27, 23 }, 0, 0, 2);
    compare(states[2], 1913235050, 2172826349, { 31, 31, 27, 23, 31, 0 }, 0, 11, 2);
    QCOMPARE(states[1].getAdvances(), 301u);
}

void Gen6Test::wildGenerator()
{
    std::vector<Slot> magcargo = { Slot(219, 30, 30, PersonalLoader::getPersonal(Game::X, 219)) };
    WildGenerator6 generator(1000, 2, 0, Encounter6::FriendSafari, Lead::None, magcargo, 0, 2, false, profileX, allFilter());
    auto states = generator.generate(0xAA56F2F4);
    QCOMPARE(states.size(), size_t(3));

    compare(states[0], 2715427869, 1955475440, { 28, 12, 17, 31, 1, 31 }, 1, 6, 0);
    compare(states[1], 284437314, 2308925421, { 17, 1, 31, 21, 7, 31 }, 1, 10, 1);
    compare(states[2], 856004686, 3677865062, { 17, 1, 31, 21, 7, 31 }, 1, 10, 1);
}

void Gen6Test::hordeGenerator()
{
    // Honey horde of Psyduck, hidden-ability member 3: members draw a 2-ability slot, the HA member skips the draw (live X)
    std::vector<Slot> horde(5, Slot(54, 20, 20, PersonalLoader::getPersonal(Game::X, 54)));
    EncounterArea6 route7(38, Encounter6::HordeHoney, horde);
    QCOMPARE(route7.getDelay(), 195u);

    WildGenerator6 generator(500, 0, 200, Encounter6::HordeHoney, Lead::None, horde, 3, 0, false, profileX, allFilter());
    auto states = generator.generate(0xAA56F2F4);
    QCOMPARE(states.size(), size_t(5));

    compare(states[0], 721093831, 3213296868, { 11, 20, 17, 2, 23, 23 }, 0, 2, 1);
    compare(states[1], 1220281124, 2232264663, { 6, 9, 2, 2, 10, 6 }, 1, 19, 0);
    compare(states[2], 3911048680, 3100301348, { 2, 27, 8, 14, 27, 19 }, 2, 1, 1);
    compare(states[3], 4133073760, 3235279697, { 22, 25, 8, 28, 10, 26 }, 1, 10, 1);
    compare(states[4], 1562068325, 1423436343, { 28, 12, 20, 26, 31, 23 }, 0, 10, 0);
    for (u8 i = 0; i < 5; i++)
    {
        QCOMPARE(states[i].getMember(), u8(i + 1));
    }
}

void Gen6Test::hordeSpots()
{
    // Y tutorial hit (2026-10-20): seed B05DDFD3, Route 7 Honey Psyduck x5, Delay 200, Shiny Charm. The shiny Psyduck is member 3 at
    // Advances 2527-2529 and only exists on 5 runs of 3 Advances, 13 apart (live Y)
    std::vector<Slot> horde(5, Slot(54, 20, 20, PersonalLoader::getPersonal(Game::Y, 54)));
    std::array<bool, 25> natures;
    natures.fill(true);
    std::array<bool, 16> powers;
    powers.fill(true);
    StateFilter filter(255, 255, 3, 1, 100, 0, 255, 0, 255, false, { 31, 31, 0, 31, 0, 31 }, { 31, 31, 31, 31, 31, 31 }, natures, powers);
    WildGenerator6 generator(2520, 20, 200, Encounter6::HordeHoney, Lead::None, horde, 0, 0, false, profileY, filter);
    auto states = generator.generate(0xB05DDFD3);
    QVERIFY(!states.empty());

    const State6 &state = states[0];
    QCOMPARE(state.getPID(), 0x1A58DE69u);
    QCOMPARE(state.getMember(), u8(3));
    for (const auto &other : states)
    {
        QCOMPARE(other.getPID(), state.getPID());
    }

    std::array<u32, 5> first = { 2553, 2540, 2527, 2514, 2501 };
    for (u8 member = 1; member <= 5; member++)
    {
        QVERIFY(state.hasSpot(member));
        QCOMPARE(state.getSpotFirst(member), first[member - 1]);
        QCOMPARE(state.getSpotLast(member), first[member - 1] + 2);
    }
}

void Gen6Test::giftGenerator()
{
    // Gifts have no 60-output container and skip every fixed field: Lapras = EC, PID, gender; Lucario = EC, PID; fossil = 11 outputs
    const StaticTemplate6 *lapras = findStatic(131);
    StaticGenerator6 laprasGenerator(100, 0, lapras->getDelay(), Lead::None, *lapras, profileX, allFilter());
    auto states = laprasGenerator.generate(0xAA56F2F4);
    QCOMPARE(states.size(), size_t(1));
    compare(states[0], 2985311265, 463328270, { 31, 20, 20, 20, 20, 20 }, 0, 6, 1);

    const StaticTemplate6 *lucario = findStatic(448);
    StaticGenerator6 lucarioGenerator(100, 0, lucario->getDelay(), Lead::None, *lucario, profileX, allFilter());
    states = lucarioGenerator.generate(0xAA56F2F4);
    compare(states[0], 1310491473, 4047990518, { 6, 25, 16, 25, 19, 31 }, 0, 11, 0);

    const StaticTemplate6 *tyrunt = findStatic(696);
    StaticGenerator6 tyruntGenerator(200, 0, tyrunt->getDelay(), Lead::None, *tyrunt, profileX, allFilter());
    states = tyruntGenerator.generate(0xAA56F2F4);
    compare(states[0], 1042132413, 1098448136, { 8, 22, 14, 28, 28, 25 }, 1, 0, 0);

    // Fixed-ability statics draw no ability: the roaming bird is 9 outputs (EC, PID, 3 picks, 3 IVs, nature)
    const StaticTemplate6 *articuno = findStatic(144);
    StaticGenerator6 articunoGenerator(300, 0, articuno->getDelay(), Lead::None, *articuno, profileX, allFilter());
    states = articunoGenerator.generate(0xAA56F2F4);
    compare(states[0], 3897440333, 3184488952, { 16, 3, 31, 31, 31, 8 }, 0, 20, 2);
}

void Gen6Test::encounterTables()
{
    auto hordes = Encounters6::getEncounters(Encounter6::HordeHoney, Game::X);
    auto route7 = std::ranges::find_if(hordes, [](const EncounterArea6 &area) { return area.getLocation() == 38; });
    QVERIFY(route7 != hordes.end());
    QCOMPARE(route7->getHordeCount(), u8(3));
    QCOMPARE(route7->getHorde(1)[0].getSpecie(), u16(54)); // Psyduck x5
    QCOMPARE(route7->getNames()[1], std::string("Psyduck \u00d75"));

    // Route 5's third horde is Plusle x4 with a Minun on X, Minun x4 with a Plusle on Y
    auto route5 = std::ranges::find_if(hordes, [](const EncounterArea6 &area) { return area.getLocation() == 28; });
    QCOMPARE(route5->getNames()[2], std::string("Plusle \u00d74, Minun"));
    auto hordesY = Encounters6::getEncounters(Encounter6::HordeSweetScent, Game::Y);
    auto route5Y = std::ranges::find_if(hordesY, [](const EncounterArea6 &area) { return area.getLocation() == 28; });
    QCOMPARE(route5Y->getNames()[2], std::string("Minun \u00d74, Plusle"));
    QCOMPARE(route5Y->getDelay(), 339u);

    auto grass = Encounters6::getEncounters(Encounter6::Grass, Game::X);
    auto terminus = std::ranges::find_if(grass, [](const EncounterArea6 &area) { return area.getLocation() == 140; });
    QVERIFY(terminus != grass.end());
    QCOMPARE(terminus->getPokemon(0).getSpecie(), u16(305)); // Lairon on X, Steelix on Y
    QCOMPARE(terminus->getPokemon(0).getMaxLevel(), u8(46));

    auto ambush = Encounters6::getEncounters(Encounter6::Ambush, Game::X);
    auto route6 = std::ranges::find_if(ambush, [](const EncounterArea6 &area) { return area.getLocation() == 34; });
    QCOMPARE(route6->getPokemon(0).getSpecie(), u16(543)); // Venipede bush
    QCOMPARE(route6->getDelay(), 62u);
    auto terminusShadow = std::ranges::find_if(ambush, [](const EncounterArea6 &area) { return area.getLocation() == 140; });
    QCOMPARE(terminusShadow->getDelay(), 32u);

    auto safari = Encounters6::getEncounters(Encounter6::FriendSafari, Game::X);
    QCOMPARE(safari.size(), size_t(1));
    QCOMPARE(safari[0].getPokemon().size(), size_t(189)); // 187 species plus Floette forms 1 and 3

    auto rods = Encounters6::getEncounters(Encounter6::SuperRod, Game::X);
    auto route3 = std::ranges::find_if(rods, [](const EncounterArea6 &area) { return area.getLocation() == 16; });
    QCOMPARE(route3->getPokemon(0).getSpecie(), u16(130)); // Gyarados
    QCOMPARE(Translator::getLocations({ 16, 140 }, Game::X), (std::vector<std::string> { "Route 3", "Terminus Cave" }));
}

void Gen6Test::eggChain()
{
    // Accepting at frame N seeds the next egg with MT outputs N (low) and N + 1 (high)
    Daycare daycare({ { { 31, 31, 31, 21, 30, 31 }, { 31, 31, 31, 31, 31, 31 } } }, { 2, 0 }, { 0, 3 }, { 1, 8 }, { 10, 0 }, 495, true, 20);
    EggGenerator6 generator(10, 2, daycare, profileY, allFilter());
    auto states = generator.generateChain(0xAA56F2F4);
    QCOMPARE(states.size(), size_t(3));
    QCOMPARE(states[0].getSeed(), 0xA1990B43FE4B06BCull);
    QCOMPARE(states[1].getSeed(), 0x08DAE683A1990B43ull);
    QCOMPARE(states[2].getSeed(), 0x74390C5708DAE683ull);

    // Each chained egg matches a direct generation from its seed
    EggState6 direct = generator.generate(states[1].getSeed());
    QCOMPARE(direct.getEC(), states[1].getEC());
    QCOMPARE(direct.getPID(), states[1].getPID());
}

void Gen6Test::idGenerator()
{
    IDFilter filter({}, {}, {}, {}, {}, {});

    // TinyMT state logged on X 1.0 (results/x_fossil.json), outputs from the research's Python port
    IDGenerator6 generator(0, 2, filter);
    auto states = generator.generate(std::array<u32, 4> { 0x431eb1bc, 0xda93a709, 0x96c34a81, 0x1afdbddc });
    QCOMPARE(states.size(), size_t(3));
    QCOMPARE(states[0].getTID(), u16(3316));
    QCOMPARE(states[0].getSID(), u16(60201));
    QCOMPARE(states[1].getTID(), u16(23455));
    QCOMPARE(states[1].getSID(), u16(16160));
    QCOMPARE(states[2].getTID(), u16(13867));
    QCOMPARE(states[2].getSID(), u16(6987));

    auto seeded = generator.generate(0x12345678u);
    QCOMPARE(seeded[0].getTID(), u16(21268));
    QCOMPARE(seeded[0].getSID(), u16(37859));
}

void Gen6Test::wc6Parse()
{
    std::array<u8, WC6::size> data {};
    data[0x68] = 0x39; // TID 12345
    data[0x69] = 0x30;
    data[0x82] = 251; // Celebi
    data[0xa0] = 0xff; // random nature
    data[0xa1] = 2; // genderless
    data[0xa2] = 0; // fixed ability 0
    data[0xa3] = 3; // never shiny
    data[0xaf] = 0xfe; // 3 perfect IVs
    for (int i = 1; i < 6; i++)
    {
        data[0xaf + i] = 0xff;
    }
    data[0xb2] = 10; // Spe (card order HP Atk Def Spe SpA SpD)
    data[0xd0] = 10;

    WC6 wc6 = WC6::parse(data.data());
    QCOMPARE(wc6.getSpecie(), u16(251));
    QCOMPARE(wc6.getTID(), u16(12345));
    QCOMPARE(wc6.getLevel(), u8(10));
    QCOMPARE(wc6.getIVCount(), u8(3));
    QCOMPARE(wc6.getIV(5), u8(10));
    QCOMPARE(wc6.getIV(3), Gen6::Random);
    QVERIFY(wc6.getPIDType() == Gen6::PIDType::Locked);
    QCOMPARE(wc6.getNature(), Gen6::Random);
    QCOMPARE(wc6.getGender(), u8(2));
}

static std::vector<std::pair<u32, u32>> runSeedSearch(bool gpu, bool &usedGPU)
{
    // Zygarde at seed 0xAA56F2F4, frame 301: 3 perfect IVs, no ability draw, nature right after the IVs
    SeedSearch6Settings settings;
    settings.ivMin = { 31, 31, 26, 31, 27, 23 };
    settings.ivMax = settings.ivMin;
    settings.minFrame = 290;
    settings.maxFrame = 310;
    settings.skip = 2 + 60 + 2; // delay, skip, EC and 1 PID roll
    settings.ivCount = 3;
    settings.nature = 0;
    settings.natureOffset = 0;

    u64 clockStart = 0xDDE41DE5 - 5000;
    SeedSearcher6 searcher(settings);
    searcher.startSearch(4, gpu, profileX.getSaveSeed() + static_cast<u32>(clockStart), 10000, clockStart);
    while (searcher.isSearching())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    usedGPU = searcher.getUsedGPU();

    std::vector<std::pair<u32, u32>> found;
    for (const auto &result : searcher.getResults())
    {
        // Clock searches report the console ms of each seed; encode a wrong one as an impossible frame
        bool clockOk = result.seed != 0xAA56F2F4 || result.ms == 0xDDE41DE5;
        found.emplace_back(result.seed, clockOk ? result.frame : 0xffffffff);
    }
    std::ranges::sort(found);
    return found;
}

void Gen6Test::seedSearcherCPU()
{
    bool usedGPU;
    auto found = runSeedSearch(false, usedGPU);
    QVERIFY(!usedGPU);
    QVERIFY(std::ranges::find(found, std::pair<u32, u32>(0xAA56F2F4, 301)) != found.end());
}

void Gen6Test::seedSearcherGPU()
{
    if (!OpenCL::isAvailable())
    {
        QSKIP("No OpenCL device");
    }

    bool usedCPU;
    auto cpu = runSeedSearch(false, usedCPU);
    bool usedGPU;
    auto gpu = runSeedSearch(true, usedGPU);
    QVERIFY(usedGPU);
    QCOMPARE(gpu, cpu);
}

void Gen6Test::saveParse()
{
    // Javier's Y save copy (not in the repo); skipped elsewhere
    QFile file("D:/Claude/Gen6-RNG-Research/Y_main_wc.sav");
    if (!file.open(QIODevice::ReadOnly))
    {
        QSKIP("Y save not available");
    }
    QByteArray data = file.readAll();

    Save6 save = Save6::parse(reinterpret_cast<const u8 *>(data.constData()), static_cast<size_t>(data.size()));
    QVERIFY(save.valid);
    QVERIFY(save.version == Game::Y);
    QCOMPARE(save.tid, u16(28952));
    QCOMPARE(save.sid, u16(46381));
    QVERIFY(save.shinyCharm);
    QCOMPARE(save.saveSeed, 0xC2A451A2u);
    QCOMPARE(save.eggSeed, 0xF4338068A5EC7C90ull);
}

void Gen6Test::seedWindow()
{
    // Zygarde IVs at frame 301 of seed 0xAA56F2F4 = X Misc[0x00] + 0xDDE41DE5 ms
    std::array<bool, 25> natures;
    natures.fill(true);
    std::array<bool, 16> powers;
    powers.fill(true);
    StateFilter filter(255, 255, 255, 1, 100, 0, 255, 0, 255, false, { 31, 31, 26, 31, 27, 23 }, { 31, 31, 26, 31, 27, 23 }, natures,
                       powers);

    const StaticTemplate6 *zygarde = findStatic(718);
    StaticGenerator6 generator(290, 20, zygarde->getDelay(), Lead::None, *zygarde, profileX, filter);
    auto window = SeedWindow6::generate(generator, profileX.getSaveSeed(), 0xDDE41DE5 - 50, 0xDDE41DE5 + 50, 4);

    QCOMPARE(window.seeds, u64(101));
    QVERIFY(window.seedsWithHits >= 1);
    auto it = std::ranges::find_if(window.states, [](const State6 &state) { return state.getSeed() == 0xAA56F2F4; });
    QVERIFY(it != window.states.end());
    QCOMPARE(it->getMs(), u64(0xDDE41DE5));
    QCOMPARE(it->getAdvances(), 301u);
    QVERIFY(std::ranges::is_sorted(window.states, {}, &State6::getMs));
}

void Gen6Test::seedTimes()
{
    auto times = SeedWindow6::timesFor(0xAA56F2F4, profileX.getSaveSeed(), 0, 2);
    QCOMPARE(times[0], u64(0xDDE41DE5));
    QCOMPARE(times[1], u64(0xDDE41DE5) + SeedWindow6::period);

    // Starting after the first time skips to the next cycle
    times = SeedWindow6::timesFor(0xAA56F2F4, profileX.getSaveSeed(), 0xDDE41DE6, 1);
    QCOMPARE(times[0], u64(0xDDE41DE5) + SeedWindow6::period);
    QCOMPARE(SeedWindow6::seedAt(profileX.getSaveSeed(), times[0]), 0xAA56F2F4u);
}

void Gen6Test::narrowSameBoot()
{
    // Second catch of the same boot: a grass Psyduck triggered 500 frames after the Zygarde
    std::vector<Slot> psyduck = { Slot(54, 20, 20, PersonalLoader::getPersonal(Game::X, 54)) };
    WildGenerator6 wild(801, 0, 0, Encounter6::Grass, Lead::None, psyduck, 0, 0, false, profileX, allFilter());
    auto second = wild.generate(0xAA56F2F4);
    QCOMPARE(second.size(), size_t(1));

    SeedSearch6Settings settings;
    settings.ivMin = second[0].getIVs();
    settings.ivMax = settings.ivMin;
    settings.minFrame = 0;
    settings.maxFrame = 0;
    settings.skip = 60 + 1 + 1;
    settings.ivCount = 0;
    settings.nature = second[0].getNature();
    settings.natureOffset = 1;

    std::vector<SeedResult6> candidates = { { 0xDDE41DE5, 0xAA56F2F4, 301 }, { 0, 0x12345678, 301 }, { 0, 0xAA56F2F5, 290 } };
    auto results = SeedSearcher6::narrow(settings, candidates, 400, 600);
    QCOMPARE(results.size(), size_t(1));
    QCOMPARE(results[0].seed, 0xAA56F2F4u);
    QCOMPARE(results[0].frame, 801u);
    QCOMPARE(results[0].ms, u64(0xDDE41DE5));
}

void Gen6Test::timerPlan()
{
    // Aim for 0xAA56F2F4 with a 3 s press-to-title delay and at least 30 s between the clock confirm and the press
    auto plan = SeedWindow6::planTimer(0xAA56F2F4, profileX.getSaveSeed(), 0, 3000, 30000);
    QCOMPARE(plan.target, u64(0xDDE41DE5));
    QVERIFY(plan.valid);
    QCOMPARE(plan.clockSet % 60000, u64(0));
    QVERIFY(plan.timer >= 30000 && plan.timer < 90000);
    QCOMPARE(SeedWindow6::seedAt(profileX.getSaveSeed(), plan.clockSet + plan.timer + 3000), 0xAA56F2F4u);

    // An attempt that seeded 25 ms late measures a delay of 3025 ms
    QCOMPARE(SeedWindow6::measuredDelay(plan.target + 25, plan.clockSet, plan.timer), s64(3025));

    // Past the latest 3DS clock setting (2099-12-31 23:59:59) there is nothing to aim for
    QVERIFY(!SeedWindow6::planTimer(0xAA56F2F4, profileX.getSaveSeed(), SeedWindow6::maxConsoleMs - 1000, 3000, 30000).valid);
}

void Gen6Test::recoverSaveSeed()
{
    // Two boots of the X save, title times noted to the second (up to 999 ms early)
    u32 saveSeed = profileX.getSaveSeed();
    u64 bootA = 0xDDE41DE5;
    u64 bootB = 0xDDE41DE5 + 123456789;
    std::vector<SeedWindow6::Boot> boots = { { bootA / 1000 * 1000, SeedWindow6::seedAt(saveSeed, bootA) },
                                             { bootB / 1000 * 1000, SeedWindow6::seedAt(saveSeed, bootB) } };

    auto recovery = SeedWindow6::recoverSaveSeed(boots, 2000);
    QVERIFY(recovery.consistent);
    QCOMPARE(static_cast<u32>(recovery.saveSeed - saveSeed), u32(bootA % 1000)); // Off by the noting error only

    // The error is a clock shift: the estimate still maps both boots' title times to their seeds within that error
    s64 errorB = static_cast<s32>(SeedWindow6::seedAt(recovery.saveSeed, bootB / 1000 * 1000) - boots[1].seed);
    QVERIFY(errorB >= -1000 && errorB <= 1000);

    // A wrong seed for boot B is caught
    boots[1].seed ^= 0x00100000;
    QVERIFY(!SeedWindow6::recoverSaveSeed(boots, 2000).consistent);
}
