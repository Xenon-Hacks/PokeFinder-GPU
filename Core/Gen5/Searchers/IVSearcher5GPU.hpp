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

#ifndef IVSEARCHER5GPU_HPP
#define IVSEARCHER5GPU_HPP

#include <Core/Gen5/Searchers/GPUIVScanner5.hpp>
#include <Core/Gen5/Searchers/SearcherBase5.hpp>
#include <fph/meta_fph_table.h>
#include <memory>
#include <mutex>

/**
 * @brief Searcher for Static/Wild Gen 5 generators that hashes seeds and filters IVs on the GPU
 *
 * The GPU returns only the seeds whose IVs pass the filter and the generator finishes them on the CPU,
 * so results match \ref IVSearcher5 and \ref IVSearcher5Fast.
 *
 * @tparam Generator State generator class to use
 * @tparam State State class to use
 */
template <class Generator, class State>
class IVSearcher5GPU final : public SearcherBase5<Generator, State>
{
public:
    /**
     * @brief Construct a new IVSearcher5GPU object that runs MT on the GPU
     *
     * @param initialAdvances Minimum IV advances
     * @param maxAdvances Maximum IV advances
     * @param generator State generator
     * @param profile Profile information
     */
    IVSearcher5GPU(u32 initialAdvances, u32 maxAdvances, const Generator &generator, const Profile5 &profile);

    /**
     * @brief Construct a new IVSearcher5GPU object that looks seeds up in the IV cache on the GPU
     *
     * @param initialAdvances Minimum IV advances
     * @param maxAdvances Maximum IV advances
     * @param ivCache Fast search IV cache
     * @param generator State generator
     * @param profile Profile information
     */
    IVSearcher5GPU(u32 initialAdvances, u32 maxAdvances, const fph::MetaFphMap<u64, std::array<u8, 6>> &ivCache,
                   const Generator &generator, const Profile5 &profile);

    /**
     * @brief Checks whether the GPU can run a search for \p generator over the IV advance range
     *
     * @param generator State generator
     * @param initialAdvances Minimum IV advances
     * @param maxAdvances Maximum IV advances
     * @param ivCache Whether the IV cache will be used
     *
     * @return true GPU search is possible
     * @return false CPU search has to be used
     */
    static bool isSupported(const Generator &generator, u32 initialAdvances, u32 maxAdvances, bool ivCache);

    /**
     * @brief Returns the error that stopped the GPU search, empty if none
     *
     * @return Error message
     */
    std::string getError() const;

    /**
     * @brief Checks whether the GPU was initialized. If not, \ref getError() says why
     *
     * @return true GPU is ready
     * @return false GPU failed to initialize
     */
    bool isReady() const;

private:
    std::unique_ptr<GPUIVScanner5> scanner;
    std::mutex scannerMutex;
    u32 initialAdvances;
    u32 maxAdvances;

    /**
     * @brief Searches between the \p start and \p end dates
     *
     * @param start Start date
     * @param end End date
     */
    void search(const Date &start, const Date &end) override;
};

#endif // IVSEARCHER5GPU_HPP
