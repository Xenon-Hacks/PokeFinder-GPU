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

#include "Linear8a.hpp"
#include <Core/Gen8a/Rng8a.hpp>
#include <bit>
#include <bitset>

namespace
{
    constexpr int maxRows = 220; // row ids 0..219, free symbols from 220
    constexpr int maxFree = 256 - maxRows;
    using Comb = std::bitset<256>;

    struct Basis
    {
        std::array<u64, 64> row {};
        std::array<Comb, 64> comb {};
        u64 have = 0;

        int rank() const
        {
            return std::popcount(have);
        }

        /**
         * @brief Inserts a row. Returns 1 when it was independent, 0 when it reduced to nothing
         */
        int add(u64 mask, int id)
        {
            Comb c;
            c.set(id);
            while (mask)
            {
                int p = 63 - std::countl_zero(mask);
                if (!(have >> p & 1))
                {
                    row[p] = mask;
                    comb[p] = c;
                    have |= 1ULL << p;
                    return 1;
                }
                mask ^= row[p];
                c ^= comb[p];
            }
            return 0;
        }
    };

    struct Row
    {
        u64 mask;
        u8 rhs[16]; // right-hand side for each guess (singles use entry 0)
    };

    void obs5Rows(const Linear8a::Symbolic &sym, const Linear8a::Obs5 &obs, Row *rows)
    {
        const Linear8a::Word &a = sym.s0[obs.step];
        const Linear8a::Word &b = sym.s1[obs.step];
        u8 o = obs.value;
        for (int j = 0; j < 4; j++)
        {
            rows[j].mask = a.mask[j];
            rows[4 + j].mask = b.mask[j];
        }
        rows[8].mask = a.mask[4] ^ b.mask[4];
        for (u8 g = 0; g < 16; g++)
        {
            u8 low = (o - g) & 15;
            u8 carry = ((g + low) >> 4) & 1;
            for (int j = 0; j < 4; j++)
            {
                rows[j].rhs[g] = ((g >> j) & 1) ^ ((a.constant >> j) & 1);
                rows[4 + j].rhs[g] = ((low >> j) & 1) ^ ((b.constant >> j) & 1);
            }
            rows[8].rhs[g] = ((o >> 4) & 1) ^ carry ^ ((a.constant >> 4) & 1) ^ ((b.constant >> 4) & 1);
        }
    }
}

namespace Linear8a
{
    u64 Word::apply(u64 x) const
    {
        return applyLinear(x) ^ constant;
    }

    u64 Word::applyLinear(u64 x) const
    {
        u64 result = 0;
        for (int i = 0; i < 64; i++)
        {
            result |= static_cast<u64>(std::popcount(mask[i] & x) & 1) << i;
        }
        return result;
    }

    Symbolic::Symbolic(u32 steps)
    {
        s0.resize(steps + 1);
        s1.resize(steps + 1);
        for (int i = 0; i < 64; i++)
        {
            s0[0].mask[i] = 1ULL << i;
            s1[0].mask[i] = 0;
        }
        s0[0].constant = 0;
        s1[0].constant = Xoro8a::C;

        for (u32 t = 0; t < steps; t++)
        {
            const Word &a = s0[t];
            const Word &b = s1[t];
            Word x;
            for (int i = 0; i < 64; i++)
            {
                x.mask[i] = a.mask[i] ^ b.mask[i];
            }
            x.constant = a.constant ^ b.constant;

            Word &na = s0[t + 1];
            Word &nb = s1[t + 1];
            for (int i = 0; i < 64; i++)
            {
                u64 m = a.mask[(i - 24 + 64) % 64] ^ x.mask[i];
                if (i >= 16)
                {
                    m ^= x.mask[i - 16];
                }
                na.mask[i] = m;
                nb.mask[i] = x.mask[(i - 37 + 64) % 64];
            }
            na.constant = Xoro8a::rotl(a.constant, 24) ^ x.constant ^ (x.constant << 16);
            nb.constant = Xoro8a::rotl(x.constant, 37);
        }
    }

    Plan makePlan(const Symbolic &sym, const std::vector<Obs5> &obs5, const std::vector<Obs1> &obs1, const std::vector<u32> &targets)
    {
        Plan plan;
        plan.words = 1 + 2 * static_cast<u32>(targets.size());

        std::vector<Row> rows;
        Basis basis;

        // 1-bit observations: always used, and an inconsistent one rules the plan out
        std::vector<u8> singleRhs;
        {
            // separate basis with explicit right-hand sides for the consistency test
            std::array<u64, 64> r {};
            std::array<u8, 64> rhs {};
            u64 have = 0;
            for (const Obs1 &obs : obs1)
            {
                const Word &a = sym.s0[obs.step];
                const Word &b = sym.s1[obs.step];
                u64 mask = a.mask[0] ^ b.mask[0];
                u8 value = (obs.value & 1) ^ (a.constant & 1) ^ (b.constant & 1);

                u64 m = mask;
                u8 v = value;
                bool independent = false;
                while (m)
                {
                    int p = 63 - std::countl_zero(m);
                    if (!(have >> p & 1))
                    {
                        r[p] = m;
                        rhs[p] = v;
                        have |= 1ULL << p;
                        independent = true;
                        break;
                    }
                    m ^= r[p];
                    v ^= rhs[p];
                }
                if (!independent)
                {
                    if (v)
                    {
                        return plan; // contradiction: no seed fits
                    }
                    continue;
                }

                Row row;
                row.mask = mask;
                row.rhs[0] = value;
                basis.add(mask, static_cast<int>(rows.size()));
                rows.emplace_back(row);
                singleRhs.emplace_back(value);
            }
        }
        size_t singles = rows.size();

        // 5-bit observations: use one when it adds more than the 4 bits it costs
        std::vector<std::array<int, 9>> groupIds;
        for (size_t i = 0; i < obs5.size() && basis.rank() < 64; i++)
        {
            if (rows.size() + 9 > maxRows)
            {
                break;
            }
            Row group[9];
            obs5Rows(sym, obs5[i], group);

            Basis trial = basis;
            int gain = 0;
            for (int k = 0; k < 9; k++)
            {
                gain += trial.add(group[k].mask, static_cast<int>(rows.size()) + k);
            }
            if (gain <= 4)
            {
                continue;
            }

            basis = trial;
            std::array<int, 9> ids;
            for (int k = 0; k < 9; k++)
            {
                ids[k] = static_cast<int>(rows.size());
                rows.emplace_back(group[k]);
            }
            groupIds.emplace_back(ids);
            plan.used.emplace_back(static_cast<int>(i));
        }

        int freeCount = 64 - basis.rank();
        if (freeCount > maxFree)
        {
            return plan;
        }

        // Back-substitution with symbols: x bit p as a combination of row right-hand sides and free bits
        std::array<Comb, 64> xc;
        int freeIndex = 0;
        for (int p = 0; p < 64; p++)
        {
            if (!(basis.have >> p & 1))
            {
                xc[p].reset();
                xc[p].set(maxRows + freeIndex++);
                continue;
            }
            Comb c = basis.comb[p];
            u64 m = basis.row[p] & ~(1ULL << p);
            while (m)
            {
                int q = std::countr_zero(m);
                c ^= xc[q];
                m &= m - 1;
            }
            xc[p] = c;
        }

        auto contribution = [&](int symbol) {
            u64 x = 0;
            for (int p = 0; p < 64; p++)
            {
                if (xc[p].test(symbol))
                {
                    x |= 1ULL << p;
                }
            }
            return x;
        };

        std::vector<u64> rowX(rows.size());
        for (size_t k = 0; k < rows.size(); k++)
        {
            rowX[k] = contribution(static_cast<int>(k));
        }

        auto pushWords = [&](std::vector<u64> &out, u64 x, bool constant) {
            out.emplace_back(x);
            for (u32 step : targets)
            {
                out.emplace_back(constant ? sym.s0[step].apply(x) : sym.s0[step].applyLinear(x));
                out.emplace_back(constant ? sym.s1[step].apply(x) : sym.s1[step].applyLinear(x));
            }
        };

        u64 baseX = 0;
        for (size_t k = 0; k < singles; k++)
        {
            if (singleRhs[k])
            {
                baseX ^= rowX[k];
            }
        }
        pushWords(plan.base, baseX, true);

        for (const auto &ids : groupIds)
        {
            plan.radix.emplace_back(16);
            plan.offset.emplace_back(static_cast<u32>(plan.table.size() / plan.words));
            for (u8 g = 0; g < 16; g++)
            {
                u64 x = 0;
                for (int id : ids)
                {
                    if (rows[id].rhs[g])
                    {
                        x ^= rowX[id];
                    }
                }
                pushWords(plan.table, x, false);
            }
            plan.bits += 4;
        }
        for (int f = 0; f < freeCount; f++)
        {
            plan.radix.emplace_back(2);
            plan.offset.emplace_back(static_cast<u32>(plan.table.size() / plan.words));
            pushWords(plan.table, 0, false);
            pushWords(plan.table, contribution(maxRows + f), false);
            plan.bits += 1;
        }

        plan.feasible = true;
        return plan;
    }
}
