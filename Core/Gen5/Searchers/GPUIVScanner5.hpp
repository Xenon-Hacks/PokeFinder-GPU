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

#ifndef GPUIVSCANNER5_HPP
#define GPUIVSCANNER5_HPP

#include <Core/Gen5/IVPrefilter5.hpp>
#include <Core/Global.hpp>
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Date;
class Profile5;

/**
 * @brief Runs the Gen 5 seed hashing and IV filtering on an OpenCL device
 *
 * Every (keypress, second) of a date and Timer0 is hashed into an initial seed on the device and its IVs are checked
 * against the IV filter, either by running MT19937 directly or by looking the seed up in a pre-filtered IV cache.
 * Only the seeds with passing IV advances come back, so the generator's PID logic runs on the CPU for a tiny fraction
 * of the seeds and results stay identical to the CPU searchers.
 */
class GPUIVScanner5
{
public:
    /**
     * @brief Seed and IV advance that passed the IV filter
     */
    struct Candidate
    {
        u64 seed; ///< Initial seed
        u32 index; ///< Keypress index * 86400 + second of the day
        u32 ivAdvance; ///< IV advance
        std::array<u8, 6> ivs; ///< IVs at that advance
    };

    /**
     * @brief Construct a scanner that runs MT19937 on the device
     *
     * @param profile Profile information
     * @param keypresses SHA1 values of the keypresses to search
     * @param prefilter How the generator reads IVs
     * @param initialAdvances Initial IV advances
     * @param maxAdvances Maximum IV advances
     */
    GPUIVScanner5(const Profile5 &profile, const std::vector<u32> &keypresses, const IVPrefilter5 &prefilter, u32 initialAdvances,
                  u32 maxAdvances);

    /**
     * @brief Construct a scanner that looks seeds up in a pre-filtered IV cache
     *
     * @param profile Profile information
     * @param keypresses SHA1 values of the keypresses to search
     * @param entries IV cache entries as ((IV advance << 32) | MT seed, IVs)
     */
    GPUIVScanner5(const Profile5 &profile, const std::vector<u32> &keypresses,
                  const std::vector<std::pair<u64, std::array<u8, 6>>> &entries);

    ~GPUIVScanner5();

    /**
     * @brief Checks whether the MT device path can handle the IV advance range
     *
     * @param prefilter How the generator reads IVs
     * @param initialAdvances Initial IV advances
     * @param maxAdvances Maximum IV advances
     *
     * @return true Range is supported
     * @return false Range is not supported
     */
    static bool supportsMT(const IVPrefilter5 &prefilter, u32 initialAdvances, u32 maxAdvances);

    /**
     * @brief Returns the error that stopped the scanner, empty if none
     *
     * @return Error message
     */
    std::string getError() const;

    /**
     * @brief Returns the number of keypresses hashed per device launch
     *
     * @return Keypresses per launch
     */
    u32 getKeypressesPerLaunch() const;

    /**
     * @brief Checks whether the device program was built and the scanner can be used
     *
     * @return true Scanner is ready
     * @return false Scanner failed to initialize
     */
    bool isReady() const;

    /**
     * @brief Checks whether seeds are looked up in an IV cache instead of running MT
     *
     * @return true IV cache is used
     * @return false MT runs on the device
     */
    bool usesCache() const
    {
        return cache;
    }

    /**
     * @brief Scans every second of \p date for the keypresses [\p keyStart, \p keyStart + \p keyCount)
     *
     * Not thread safe, callers must serialize calls.
     *
     * @param date Date to scan
     * @param timer0 Timer0 to scan
     * @param keyStart First keypress index
     * @param keyCount Number of keypresses
     * @param candidates Vector the passing seeds are appended to
     *
     * @return true Scan succeeded
     * @return false Device error, see \ref getError()
     */
    bool scan(const Date &date, u16 timer0, u32 keyStart, u32 keyCount, std::vector<Candidate> &candidates);

private:
    struct Device;
    std::unique_ptr<Device> device;
    std::vector<std::pair<u64, std::array<u8, 6>>> entries;
    std::string error;
    bool cache;

    void init(const Profile5 &profile, const std::vector<u32> &keypresses, const std::string &options);
};

#endif // GPUIVSCANNER5_HPP
