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

#ifndef SPAWNERSEARCHER8A_HPP
#define SPAWNERSEARCHER8A_HPP

#include <Core/Parents/Searchers/SearcherBase.hpp>
#include <array>
#include <string>
#include <vector>

/**
 * @brief What the summary screen shows of one wild Pokémon from a spawner (not an alpha)
 */
struct WildObservation8a
{
    std::array<u8, 6> ivs;
    s8 ability = -1; ///< slot 0/1, -1 unknown
    s8 gender = -1; ///< 0 male, 1 female, -1 unknown
    s8 nature = -1; ///< -1 unknown
    s8 shiny = 0; ///< 0 no, 1 yes, -1 unknown
    u8 rolls = 1; ///< shiny rolls of the spawn (1 base, +1 research 10, +2 perfect, +3 charm)
    u8 genderRatio = 127; ///< species ratio: 0 male, 254 female, 255 genderless
    u8 heightMin = 0;
    u8 heightMax = 255;
    u8 weightMin = 0;
    u8 weightMax = 255;

    /**
     * @brief Generates the Pokémon of entity seed \p entity and compares it
     */
    bool matches(u64 entity) const;
};

/**
 * @brief How the second Pokémon relates to the first in the spawner's seed chain
 */
enum class SpawnerRelation8a : u8
{
    SameFill, ///< made in the same fill, \ref SpawnerResult8a::position = its slot (2-4)
    NextFill ///< made in the next fill, after a fill of \ref SpawnerResult8a::position Pokémon (1-4)
};

struct SpawnerResult8a
{
    u64 groupSeed; ///< group seed that made the first Pokémon's fill
    u64 slot1;
    u64 entity1;
    u64 slot2;
    u64 entity2;
    SpawnerRelation8a relation;
    u8 position;
};

/**
 * @brief Recovers a wild spawner's group seed from what the summary screen shows of two of its Pokémon.
 *
 * A fill takes r = Xoroshiro(group seed): per new Pokémon slot seed = r.next(), alpha seed = r.next(), then the next
 * group seed = r.next(). Xoroshiro(slot seed) gives the slot roll, then the entity seed, which makes the Pokémon.
 *
 * Step 1 lists the entity seeds that fit the first Pokémon (\ref Linear8a tables, GPU or CPU). Step 2 finds the slot
 * seeds behind each of them (a carry-guessing search, about half a second per entity seed and CPU thread), so step 1
 * must leave few candidates: IVs, ability, nature and gender leave about 10^8, height and weight ranges cut that to
 * thousands. Step 3 checks the second Pokémon for every relation.
 *
 * Built from the research model (section 31: spawner fill, slot seed, entity seed checked live in Eden); the
 * visible-only recovery itself has not been tried on a console.
 */
class SpawnerSearcher8a : public SearcherBase<SpawnerResult8a>
{
public:
    SpawnerSearcher8a(const WildObservation8a &first, const WildObservation8a &second);

    /**
     * @brief Entity seeds that fit \p obs, at most \p cap (step 1, CPU)
     */
    static std::vector<u64> entityCandidates(const WildObservation8a &obs, int threads, u64 cap, bool *overflow);

    /**
     * @brief Every slot seed s with Xoroshiro(s).next() twice == \p entity (step 2)
     */
    static std::vector<u64> slotSeeds(u64 entity);

    std::string getError() const;
    bool getUsedGPU() const;
    u64 getCandidates() const;

    /**
     * @brief Starts the search
     *
     * @param threads CPU threads
     * @param useGPU Whether to try the GPU for step 1
     * @param maxCandidates Stop after step 1 when more entity seeds than this fit
     */
    void startSearch(int threads, bool useGPU, u64 maxCandidates);

private:
    WildObservation8a first;
    WildObservation8a second;
    std::string error;
    std::atomic<bool> usedGPU;
    std::atomic<u64> candidates;

    bool candidatesGPU(std::vector<u64> &out, u64 cap, bool *overflow);
    void checkSecond(u64 slot1, u64 entity1);
};

#endif // SPAWNERSEARCHER8A_HPP
