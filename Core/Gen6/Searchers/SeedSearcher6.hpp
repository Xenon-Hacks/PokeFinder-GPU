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

#ifndef SEEDSEARCHER6_HPP
#define SEEDSEARCHER6_HPP

#include <Core/Parents/Searchers/SearcherBase.hpp>
#include <array>
#include <string>

/**
 * @brief Seed and trigger frame that reproduce the target
 */
struct SeedResult6
{
    u64 ms; ///< Console clock in ms since 2000 (clock searches only)
    u32 seed; ///< Initial MT seed
    u32 frame; ///< Trigger frame
};

/**
 * @brief What the seed searcher looks for
 */
struct SeedSearch6Settings
{
    std::array<u8, 6> ivMin; ///< Minimum IVs (PokeFinder order)
    std::array<u8, 6> ivMax; ///< Maximum IVs (PokeFinder order)
    u32 minFrame; ///< First trigger frame
    u32 maxFrame; ///< Last trigger frame
    u32 skip; ///< MT outputs from the trigger frame to the first IV draw (delay + 60 + EC + PID rolls)
    u8 ivCount; ///< Guaranteed perfect IVs drawn with rand(6)
    u8 nature; ///< Nature to match, 255 for any
    u8 natureOffset; ///< MT outputs between the last IV and the nature (1 when an ability is drawn)
};

/**
 * @brief Finds X/Y initial MT seeds from a Pokemon's IVs (and optionally nature)
 *
 * Seeds can be the full 32bit range or Misc[0x00] + console ms over a time window. The scan runs on an OpenCL GPU when one is
 * available and falls back to CPU threads.
 */
class SeedSearcher6 : public SearcherBase<SeedResult6>
{
public:
    /**
     * @brief Construct a new SeedSearcher6 object
     *
     * @param settings Search settings
     */
    SeedSearcher6(const SeedSearch6Settings &settings);

    /**
     * @brief Checks whether \p outputs (starting at the first IV draw) match the settings
     *
     * @param settings Search settings
     * @param outputs MT outputs starting at trigger frame + skip
     * @param size Number of outputs available
     *
     * @return true Outputs match
     * @return false Outputs do not match
     */
    static bool matches(const SeedSearch6Settings &settings, const u32 *outputs, u32 size);

    /**
     * @brief Keeps the candidates that also produce the next catch of the same boot
     *
     * The next catch is searched from frame + \p minGap to frame + \p maxGap of each candidate. Its settings' frame range is
     * ignored. A wrong seed survives with odds of about (frames searched) / 2^30, so one more catch nearly always leaves only the
     * real seed.
     *
     * @param settings Next catch (IVs, nature, skip, perfect IVs)
     * @param candidates Seeds and frames that matched the earlier catches
     * @param minGap Fewest MT frames between the previous trigger and this one
     * @param maxGap Most MT frames between the previous trigger and this one
     *
     * @return Matching candidates with the frame of the new catch
     */
    static std::vector<SeedResult6> narrow(const SeedSearch6Settings &settings, const std::vector<SeedResult6> &candidates, u32 minGap,
                                           u32 maxGap);

    /**
     * @brief Returns the error that stopped the GPU path, empty if none
     *
     * @return Error message
     */
    std::string getError() const;

    /**
     * @brief Returns whether the GPU did the search
     *
     * @return true GPU searched
     * @return false CPU searched
     */
    bool getUsedGPU() const;

    /**
     * @brief Starts the search over seeds start .. start + count - 1 (wrapping at 2^32)
     *
     * @param threads Number of CPU threads
     * @param useGPU Whether to try the GPU first
     * @param start First seed
     * @param count Number of seeds (1 .. 2^32)
     * @param clockStart Console ms of \p start for clock searches, 0 otherwise
     */
    void startSearch(int threads, bool useGPU, u32 start, u64 count, u64 clockStart);

private:
    SeedSearch6Settings settings;
    std::string error;
    u64 clockStart;
    std::atomic<bool> usedGPU;

    void addResult(u32 seed, u32 frame, u32 start);

    void searchCPU(u32 start, u64 count, int threads);

    bool searchGPU(u32 start, u64 count);
};

#endif // SEEDSEARCHER6_HPP
