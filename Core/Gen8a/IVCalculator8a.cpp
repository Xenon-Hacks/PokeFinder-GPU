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

#include "IVCalculator8a.hpp"
#include <cmath>

namespace
{
    constexpr const char *statNames[6] = { "HP", "Atk", "Def", "SpA", "SpD", "Spe" };
    constexpr float multiplier[11] = { 0, 2, 3, 4, 7, 8, 9, 14, 15, 16, 25 };
    constexpr u8 bandMin[4] = { 0, 20, 26, 31 };
    constexpr u8 bandMax[4] = { 19, 25, 30, 31 };

    // Per nature: +1 / -1 for Atk, Def, SpA, SpD, Spe
    constexpr s8 natureTable[25][5] = { { 0, 0, 0, 0, 0 },  { 1, -1, 0, 0, 0 }, { 1, 0, 0, 0, -1 }, { 1, 0, -1, 0, 0 }, { 1, 0, 0, -1, 0 },
                                        { -1, 1, 0, 0, 0 }, { 0, 0, 0, 0, 0 },  { 0, 1, 0, 0, -1 }, { 0, 1, -1, 0, 0 }, { 0, 1, 0, -1, 0 },
                                        { -1, 0, 0, 0, 1 }, { 0, -1, 0, 0, 1 }, { 0, 0, 0, 0, 0 },  { 0, 0, -1, 0, 1 }, { 0, 0, 0, -1, 1 },
                                        { -1, 0, 1, 0, 0 }, { 0, -1, 1, 0, 0 }, { 0, 0, 1, 0, -1 }, { 0, 0, 0, 0, 0 },  { 0, 0, 1, -1, 0 },
                                        { -1, 0, 0, 1, 0 }, { 0, -1, 0, 1, 0 }, { 0, 0, 0, 1, -1 }, { 0, 0, -1, 1, 0 }, { 0, 0, 0, 0, 0 } };

    const std::vector<IVCalculator8a::Entry> entries = {
#include "IVCalculator8aData.inc"
    };
}

namespace IVCalculator8a
{
    const std::vector<Entry> &getEntries()
    {
        return entries;
    }

    const Entry *getEntry(u16 specie, u8 form)
    {
        for (const Entry &entry : entries)
        {
            if (entry.specie == specie && entry.form == form)
            {
                return &entry;
            }
        }
        return nullptr;
    }

    u8 bias(u8 iv)
    {
        return iv >= 31 ? 3 : iv >= 26 ? 2 : iv >= 20 ? 1 : 0;
    }

    std::array<u8, 6> effortLevels(const std::array<u8, 6> &ivs)
    {
        std::array<u8, 6> levels;
        for (int i = 0; i < 6; i++)
        {
            levels[i] = bias(ivs[i]);
        }
        return levels;
    }

    u16 calcStat(int stat, u8 base, u8 level, u8 effort, u8 nature)
    {
        volatile float b = static_cast<float>(base);
        volatile float lv = static_cast<float>(level);
        volatile float g = std::sqrt(b);
        g = g * multiplier[effort > 10 ? 10 : effort];
        g = g + lv;
        g = g / 2.5f;
        int result = static_cast<int>(std::floor(g + 0.5f));
        if (stat == 0)
        {
            volatile float h = lv / 100.0f;
            h = h + 1.0f;
            h = h * b;
            h = h + lv;
            return static_cast<u16>(result + static_cast<int>(h));
        }
        volatile float s = lv / 50.0f;
        s = s + 1.0f;
        s = s * b;
        s = s / 1.5f;
        int value = static_cast<int>(s);
        s8 mod = natureTable[nature % 25][stat - 1];
        if (mod)
        {
            value = ((value * (mod > 0 ? 110 : 90)) & 0xfffe) / 100;
        }
        return static_cast<u16>(result + value);
    }

    Result analyze(const std::array<u8, 6> &base, u8 level, u8 nature, const std::array<int, 6> &effort, const std::array<int, 6> &stats,
                   const std::array<int, 6> &grit, int flawless)
    {
        Result result;
        bool biases[6][4] = {};
        for (int i = 0; i < 6; i++)
        {
            std::vector<u8> candidates;
            bool statFits = false;
            for (u8 e = 0; e <= 10; e++)
            {
                if (effort[i] >= 0 && effort[i] != e)
                {
                    continue;
                }
                if (stats[i] >= 0)
                {
                    if (calcStat(i, base[i], level, e, nature) != stats[i])
                    {
                        continue;
                    }
                    statFits = true;
                }
                candidates.emplace_back(e);
            }
            if (effort[i] > 10)
            {
                result.errors.emplace_back(std::string(statNames[i]) + ": effort level is above 10");
            }
            else if (candidates.empty() && stats[i] >= 0 && !statFits)
            {
                bool any = false;
                for (u8 e = 0; e <= 10; e++)
                {
                    any |= calcStat(i, base[i], level, e, nature) == stats[i];
                }
                result.errors.emplace_back(std::string(statNames[i])
                                           + (any ? ": the stat and the effort level do not go together" :
                                                    ": the stat fits no effort level; check species, form, level and nature"));
            }
            for (u8 e : candidates)
            {
                if (grit[i] >= 0)
                {
                    int b = e - grit[i];
                    if (b >= 0 && b <= 3)
                    {
                        biases[i][b] = true;
                    }
                }
                else
                {
                    for (int b = 0; b <= 3 && b <= e; b++)
                    {
                        biases[i][b] = true;
                    }
                }
            }
            if (!candidates.empty() && !(biases[i][0] || biases[i][1] || biases[i][2] || biases[i][3]))
            {
                result.errors.emplace_back(std::string(statNames[i]) + ": the effort level cannot come from that much grit");
            }
            result.stats[i].effort = candidates;
        }

        if (flawless > 0)
        {
            int can = 0;
            for (int i = 0; i < 6; i++)
            {
                can += biases[i][3];
            }
            if (can < flawless)
            {
                result.errors.emplace_back(std::to_string(flawless) + " perfect IVs are guaranteed, but only " + std::to_string(can)
                                           + " stat(s) can be 31");
            }
            else if (can == flawless)
            {
                for (int i = 0; i < 6; i++)
                {
                    if (biases[i][3])
                    {
                        if (biases[i][0] || biases[i][1] || biases[i][2])
                        {
                            result.forced.emplace_back(i);
                        }
                        biases[i][0] = biases[i][1] = biases[i][2] = false;
                    }
                }
            }
        }

        for (int i = 0; i < 6; i++)
        {
            for (int b = 0; b < 4; b++)
            {
                if (!biases[i][b])
                {
                    continue;
                }
                auto &ranges = result.stats[i].ranges;
                if (!ranges.empty() && ranges.back().max + 1 == bandMin[b])
                {
                    ranges.back().max = bandMax[b];
                }
                else
                {
                    ranges.push_back({ bandMin[b], bandMax[b] });
                }
            }
        }
        return result;
    }
}
