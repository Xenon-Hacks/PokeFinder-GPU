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

#include "StockBootSeedSearcher8a.hpp"
#include <Core/Util/OpenCL.hpp>
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <memory>

using namespace OpenCL;

namespace
{
    constexpr u64 XOROSHIRO_CONST = 0x82A2B175229D6A5BULL;
    constexpr u32 MAX_OUT = 1u << 22;
    constexpr int TABLE_WORDS = 6 + 6 * 160 * 6 + 64 * 5;
    constexpr int MAX_STEPS = 800;

    // -100 * log2 of the chance of n redraws between two Unown (weight rand(129) redraw 127/256, nature redraw 7/32)
    constexpr u32 extraCost[16] = { 134, 183, 266, 360, 458, 557, 658, 759, 860, 961, 1062, 1163, 1264, 1365, 1466, 1567 };

    inline u64 rotl(u64 x, int k)
    {
        return (x << k) | (x >> (64 - k));
    }

    inline u64 next(u64 &a, u64 &b)
    {
        u64 r = a + b;
        u64 t = a ^ b;
        a = rotl(a, 24) ^ t ^ (t << 16);
        b = rotl(t, 37);
        return r;
    }

    inline u8 effortClass(u32 iv)
    {
        return iv == 31 ? 3 : iv >= 26 ? 2 : iv >= 20 ? 1 : 0;
    }

    // Every state bit after t steps as an affine function of the 64 seed bits
    struct Affine
    {
        u64 m[64];
        u64 c;
    };

    void symbolicStep(const Affine &a, const Affine &b, Affine &na, Affine &nb)
    {
        Affine t;
        for (int i = 0; i < 64; i++)
        {
            t.m[i] = a.m[i] ^ b.m[i];
        }
        t.c = a.c ^ b.c;
        for (int i = 0; i < 64; i++)
        {
            u64 m = a.m[(i + 40) % 64] ^ t.m[i];
            if (i >= 16)
            {
                m ^= t.m[i - 16];
            }
            na.m[i] = m;
            nb.m[i] = t.m[(i + 27) % 64];
        }
        na.c = rotl(a.c, 24) ^ t.c ^ (t.c << 16);
        nb.c = rotl(t.c, 37);
    }

    // Device code. T: tables (base, per-draw guess entries, kernel basis), 6/6/5 words each:
    // seed part, parity checks, and the linear images of two test Unown (0 and 1 nature redraws each).
    // P: 0 test count A, 1 kernel bits, 2-7 classes A, 8 extra-draw guess, 9 has extra draw, 10 test count B, 11-16 classes B
    const char *kernelSource = R"CL(
inline uint cls(ulong w)
{
    uint iv = ((uint)(w & 31) + (uint)((w >> 5) & 31)) & 31;
    return iv == 31 ? 3u : iv >= 26 ? 2u : iv >= 20 ? 1u : 0u;
}

inline int tst(ulong w, uint nt, __global const uint *c)
{
    for (uint q = 0; q < nt; q++)
    {
        if (cls(w) != c[q])
            return 0;
        w >>= 10;
    }
    return 1;
}

__kernel void scan(__global const ulong *T, __global const uint *P, __global volatile uint *cnt, __global ulong *out)
{
    uint gid = get_global_id(0);
    uint nt = P[0], kd = P[1], nt2 = P[10];
    ulong x = T[0], ck = T[1], w0 = T[2], w1 = T[3], w2 = T[4], w3 = T[5];
    for (uint d = 0; d < 5; d++)
    {
        uint e = (gid >> (5 * d)) & 31;
        __global const ulong *p = T + 6 + (d * 160 + e) * 6;
        x ^= p[0]; ck ^= p[1]; w0 ^= p[2]; w1 ^= p[3]; w2 ^= p[4]; w3 ^= p[5];
    }
    if (P[9])
    {
        __global const ulong *p = T + 6 + (5 * 160 + P[8]) * 6;
        x ^= p[0]; ck ^= p[1]; w0 ^= p[2]; w1 ^= p[3]; w2 ^= p[4]; w3 ^= p[5];
    }
    if (ck)
        return;
    ulong nk = 1UL << kd;
    for (ulong g = 0;;)
    {
        if ((tst(w0, nt, P + 2) || tst(w1, nt, P + 2)) && (tst(w2, nt2, P + 11) || tst(w3, nt2, P + 11)))
        {
            uint i = atomic_inc(cnt);
            if (i < (1u << 22))
                out[i] = x;
        }
        g++;
        if (g == nk)
            break;
        uint b = ctz(g);
        __global const ulong *k = T + 6 + 6 * 160 * 6 + b * 5;
        x ^= k[0]; w0 ^= k[1]; w1 ^= k[2]; w2 ^= k[3]; w3 ^= k[4];
    }
}
)CL";
}

StockBootSeedSearcher8a::StockBootSeedSearcher8a(const std::vector<StockUnown8a> &unown, u32 minAdvance, u32 maxAdvance, u32 maxExtra) :
    unown(unown), minAdvance(minAdvance), maxAdvance(maxAdvance), extraBlock(-1), extraStat(0), testBlock { 0, 1 }
{
    if (this->unown.size() != 5)
    {
        return;
    }
    maxExtra = std::min<u32>(maxExtra, 15);

    // Extra nearly exact draw: an effort level 3 (IV 31) if there is one, else an effort level 2 (IV 26-30)
    int best = 1;
    for (int k = 0; k < 5; k++)
    {
        for (int i = 0; i < 6; i++)
        {
            if (this->unown[k].effort[i] > best)
            {
                best = this->unown[k].effort[i];
                extraBlock = k;
                extraStat = i;
            }
        }
    }

    // Test Unown: the two whose effort levels a random seed is least likely to match
    static const double chance[4] = { 20 / 32.0, 6 / 32.0, 5 / 32.0, 1 / 32.0 };
    double pass[5];
    for (int k = 0; k < 5; k++)
    {
        pass[k] = 1;
        for (int i = 0; i < 6; i++)
        {
            if (!(k == extraBlock && i == extraStat))
            {
                pass[k] *= chance[this->unown[k].effort[i] & 3];
            }
        }
    }
    int order[5] = { 0, 1, 2, 3, 4 };
    std::sort(order, order + 5, [&](int a, int b) { return pass[a] < pass[b]; });
    testBlock[0] = order[0];
    testBlock[1] = order[1];

    u32 combos = 1;
    for (int k = 0; k < 4; k++)
    {
        combos *= maxExtra + 1;
    }
    for (u32 nature0 = minAdvance + 10; nature0 <= maxAdvance + 12; nature0++)
    {
        for (u32 c = 0; c < combos; c++)
        {
            for (u8 redraw = 0; redraw < (extraBlock >= 0 ? 2 : 1); redraw++)
            {
                Alignment a;
                a.nature0 = nature0;
                a.redraw = redraw;
                a.cost = redraw * 220 + (nature0 > maxAdvance + 10 ? (nature0 - maxAdvance - 10) * 220 : 0);
                u32 cc = c;
                for (int k = 0; k < 4; k++)
                {
                    a.extra[k] = cc % (maxExtra + 1);
                    cc /= maxExtra + 1;
                    a.cost += extraCost[a.extra[k]];
                }
                alignments.emplace_back(a);
            }
        }
    }
    std::stable_sort(alignments.begin(), alignments.end(), [](const Alignment &a, const Alignment &b) { return a.cost < b.cost; });
}

u16 StockBootSeedSearcher8a::displayedWeight(u8 scalar)
{
    auto percent = [](int s) { return (static_cast<float>(s) / 255.0f) * 0.40000004f + 0.8f; };
    float kg = 5.0f * percent(128) * percent(scalar);
    return static_cast<u16>(std::floor(kg * 10 + 0.5));
}

bool StockBootSeedSearcher8a::verify(u64 seed, u32 advance) const
{
    u64 a = seed;
    u64 b = XOROSHIRO_CONST;
    for (u32 i = 0; i < advance; i++)
    {
        next(a, b);
    }
    for (const auto &u : unown)
    {
        next(a, b);
        next(a, b);
        next(a, b);
        for (int i = 0; i < 6; i++)
        {
            if (effortClass(next(a, b) & 31) != u.effort[i])
            {
                return false;
            }
        }
        next(a, b);
        u32 v;
        do
        {
            v = next(a, b) & 31;
        } while (v >= 25);
        if (v != u.nature)
        {
            return false;
        }
        do
        {
            v = next(a, b) & 255;
        } while (v >= 129);
        v += next(a, b) & 127;
        if (displayedWeight(static_cast<u8>(v)) != u.weight)
        {
            return false;
        }
    }
    return true;
}

std::string StockBootSeedSearcher8a::getError() const
{
    return error;
}

u64 StockBootSeedSearcher8a::getAlignments() const
{
    return alignments.size();
}

void StockBootSeedSearcher8a::startSearch()
{
    setMaxProgress(std::max<u64>(alignments.size(), 1));
    activeThreads = 1;

    threadContainer.emplace_back([this] {
        if (unown.size() != 5)
        {
            error = "Exactly five Unown are needed";
        }
        else if (maxAdvance + 12 + 5 * 40 >= MAX_STEPS)
        {
            error = "The start advance is too high";
        }
        else
        {
            searchGPU();
        }
        activeThreads--;
    });
}

bool StockBootSeedSearcher8a::searchGPU()
{
    const API *cl = OpenCL::api();
    if (!cl)
    {
        error = "This search needs an OpenCL GPU";
        return false;
    }

    cl_device_id id = OpenCL::device();
    cl_int err;
    cl_context context = nullptr;
    cl_command_queue queue = nullptr;
    cl_program program = nullptr;
    cl_kernel kernel = nullptr;
    cl_mem tableBuffer = nullptr;
    cl_mem paramBuffer = nullptr;
    cl_mem countBuffer = nullptr;
    cl_mem outBuffer = nullptr;

    auto release = [&] {
        for (cl_mem mem : { outBuffer, countBuffer, paramBuffer, tableBuffer })
        {
            if (mem)
            {
                cl->clReleaseMemObject(mem);
            }
        }
        if (kernel)
        {
            cl->clReleaseKernel(kernel);
        }
        if (program)
        {
            cl->clReleaseProgram(program);
        }
        if (queue)
        {
            cl->clReleaseCommandQueue(queue);
        }
        if (context)
        {
            cl->clReleaseContext(context);
        }
    };
    auto fail = [&](const std::string &message, cl_int code) {
        error = message + " (OpenCL error " + std::to_string(code) + ")";
        release();
        return false;
    };

    context = cl->clCreateContext(nullptr, 1, &id, nullptr, nullptr, &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL context", err);
    }
    queue = cl->clCreateCommandQueue(context, id, 0, &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL queue", err);
    }
    program = cl->clCreateProgramWithSource(context, 1, &kernelSource, nullptr, &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL program", err);
    }
    err = cl->clBuildProgram(program, 1, &id, "", nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to build OpenCL program", err);
    }
    kernel = cl->clCreateKernel(program, "scan", &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL kernel", err);
    }
    tableBuffer = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(u64) * TABLE_WORDS, nullptr, &err);
    paramBuffer = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(u32) * 20, nullptr, &err);
    countBuffer = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(u32), nullptr, &err);
    outBuffer = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(u64) * MAX_OUT, nullptr, &err);
    if (err != CL_SUCCESS || !tableBuffer || !paramBuffer || !countBuffer)
    {
        return fail("Failed to create OpenCL buffers", err);
    }
    cl->clSetKernelArg(kernel, 0, sizeof(cl_mem), &tableBuffer);
    cl->clSetKernelArg(kernel, 1, sizeof(cl_mem), &paramBuffer);
    cl->clSetKernelArg(kernel, 2, sizeof(cl_mem), &countBuffer);
    cl->clSetKernelArg(kernel, 3, sizeof(cl_mem), &outBuffer);

    // Symbolic states
    auto A = std::make_unique<Affine[]>(MAX_STEPS);
    auto B = std::make_unique<Affine[]>(MAX_STEPS);
    for (int i = 0; i < 64; i++)
    {
        A[0].m[i] = 1ULL << i;
        B[0].m[i] = 0;
    }
    A[0].c = 0;
    B[0].c = XOROSHIRO_CONST;
    for (int t = 0; t + 1 < MAX_STEPS; t++)
    {
        symbolicStep(A[t], B[t], A[t + 1], B[t + 1]);
    }

    auto table = std::make_unique<u64[]>(TABLE_WORDS);
    auto found = std::make_unique<u64[]>(MAX_OUT);

    for (const Alignment &al : alignments)
    {
        if (isCancelled())
        {
            break;
        }

        u32 nat[5];
        nat[0] = al.nature0;
        for (int k = 1; k < 5; k++)
        {
            nat[k] = nat[k - 1] + 13 + al.extra[k - 1];
        }

        // Exact draws: the five natures, plus the extra effort level draw
        u32 drawIndex[6];
        int valueCount[6];
        u8 values[6][5];
        int m = 0;
        for (int k = 0; k < 5; k++)
        {
            drawIndex[m] = nat[k];
            valueCount[m] = 1;
            values[m][0] = unown[k].nature;
            m++;
        }
        if (extraBlock >= 0)
        {
            drawIndex[m] = nat[extraBlock] - 7 + extraStat - al.redraw;
            if (unown[extraBlock].effort[extraStat] == 3)
            {
                valueCount[m] = 1;
                values[m][0] = 31;
            }
            else
            {
                valueCount[m] = 5;
                for (u8 j = 0; j < 5; j++)
                {
                    values[m][j] = 26 + j;
                }
            }
            m++;
        }

        // Equations: row d*10+j is bit j of s0 (j < 5) or bit j-5 of s1 at draw d. Reduced with combination tracking
        int rowCount = 10 * m;
        u64 mask[60];
        u64 comb[60];
        u8 cst[60];
        for (int d = 0; d < m; d++)
        {
            for (int j = 0; j < 10; j++)
            {
                const Affine &v = j < 5 ? A[drawIndex[d]] : B[drawIndex[d]];
                int bit = j < 5 ? j : j - 5;
                mask[d * 10 + j] = v.m[bit];
                cst[d * 10 + j] = (v.c >> bit) & 1;
                comb[d * 10 + j] = 1ULL << (d * 10 + j);
            }
        }
        int pivot[64];
        int rank = 0;
        for (int col = 63; col >= 0; col--)
        {
            int p = -1;
            for (int r = rank; r < rowCount; r++)
            {
                if ((mask[r] >> col) & 1)
                {
                    p = r;
                    break;
                }
            }
            if (p < 0)
            {
                continue;
            }
            std::swap(mask[p], mask[rank]);
            std::swap(comb[p], comb[rank]);
            std::swap(cst[p], cst[rank]);
            for (int r = 0; r < rowCount; r++)
            {
                if (r != rank && ((mask[r] >> col) & 1))
                {
                    mask[r] ^= mask[rank];
                    comb[r] ^= comb[rank];
                    cst[r] ^= cst[rank];
                }
            }
            pivot[rank++] = col;
        }
        int checks = rowCount - rank;
        int kernelBits = 64 - rank;
        int freeColumn[64];
        int freeCount = 0;
        {
            u64 pivots = 0;
            for (int i = 0; i < rank; i++)
            {
                pivots |= 1ULL << pivot[i];
            }
            for (int c = 0; c < 64; c++)
            {
                if (!((pivots >> c) & 1))
                {
                    freeColumn[freeCount++] = c;
                }
            }
        }

        // Linear images for the two test Unown, each with 0 and 1 nature redraws: 6 (or 5) IV draws x 10 bits
        u64 testMask[4][60];
        u64 testConst[4] = { 0, 0, 0, 0 };
        u32 testClass[2][6];
        u32 testCount[2];
        for (int t = 0; t < 2; t++)
        {
            int block = testBlock[t];
            int stat[6];
            u32 count = 0;
            for (int i = 0; i < 6; i++)
            {
                if (!(block == extraBlock && i == extraStat))
                {
                    stat[count++] = i;
                }
            }
            std::stable_sort(stat, stat + count, [&](int a, int b) { return unown[block].effort[a] > unown[block].effort[b]; });
            testCount[t] = count;
            for (u32 q = 0; q < count; q++)
            {
                testClass[t][q] = unown[block].effort[stat[q]];
                for (int s = 0; s < 2; s++)
                {
                    u32 index = nat[block] - 7 + stat[q] - s;
                    for (int j = 0; j < 10; j++)
                    {
                        const Affine &v = j < 5 ? A[index] : B[index];
                        int bit = j < 5 ? j : j - 5;
                        testMask[t * 2 + s][q * 10 + j] = v.m[bit];
                        if ((v.c >> bit) & 1)
                        {
                            testConst[t * 2 + s] |= 1ULL << (q * 10 + j);
                        }
                    }
                }
            }
        }
        auto image = [&](u64 x, int set) {
            u64 w = 0;
            int bits = testCount[set / 2] * 10;
            for (int b = 0; b < bits; b++)
            {
                w |= static_cast<u64>(std::popcount(testMask[set][b] & x) & 1) << b;
            }
            return w;
        };

        std::memset(table.get(), 0, sizeof(u64) * TABLE_WORDS);
        u64 baseSeed = 0;
        u64 baseCheck = 0;
        for (int i = 0; i < rank; i++)
        {
            if (cst[i])
            {
                baseSeed |= 1ULL << pivot[i];
            }
        }
        for (int z = 0; z < checks; z++)
        {
            if (cst[rank + z])
            {
                baseCheck |= 1ULL << z;
            }
        }
        table[0] = baseSeed;
        table[1] = baseCheck;
        for (int s = 0; s < 4; s++)
        {
            table[2 + s] = image(baseSeed, s) ^ testConst[s];
        }
        for (int d = 0; d < m; d++)
        {
            for (int e = 0; e < 32 * valueCount[d]; e++)
            {
                u32 u = e & 31;
                u32 w = (values[d][e >> 5] - u) & 31;
                u64 g = static_cast<u64>(u | (w << 5)) << (10 * d);
                u64 x = 0;
                u64 c = 0;
                for (int i = 0; i < rank; i++)
                {
                    if (std::popcount(comb[i] & g) & 1)
                    {
                        x |= 1ULL << pivot[i];
                    }
                }
                for (int z = 0; z < checks; z++)
                {
                    if (std::popcount(comb[rank + z] & g) & 1)
                    {
                        c |= 1ULL << z;
                    }
                }
                u64 *p = table.get() + 6 + (d * 160 + e) * 6;
                p[0] = x;
                p[1] = c;
                for (int s = 0; s < 4; s++)
                {
                    p[2 + s] = image(x, s);
                }
            }
        }
        for (int f = 0; f < freeCount; f++)
        {
            u64 x = 1ULL << freeColumn[f];
            for (int i = 0; i < rank; i++)
            {
                if ((mask[i] >> freeColumn[f]) & 1)
                {
                    x |= 1ULL << pivot[i];
                }
            }
            u64 *p = table.get() + 6 + 6 * 160 * 6 + f * 5;
            p[0] = x;
            for (int s = 0; s < 4; s++)
            {
                p[1 + s] = image(x, s);
            }
        }
        err = cl->clEnqueueWriteBuffer(queue, tableBuffer, CL_TRUE, 0, sizeof(u64) * TABLE_WORDS, table.get(), 0, nullptr, nullptr);
        if (err != CL_SUCCESS)
        {
            return fail("Failed to write OpenCL buffer", err);
        }

        u32 params[20] = {};
        params[0] = testCount[0];
        params[1] = kernelBits;
        params[10] = testCount[1];
        for (u32 q = 0; q < testCount[0]; q++)
        {
            params[2 + q] = testClass[0][q];
        }
        for (u32 q = 0; q < testCount[1]; q++)
        {
            params[11 + q] = testClass[1][q];
        }
        params[9] = m == 6;
        int batches = m == 6 ? 32 * valueCount[5] : 1;
        size_t global = static_cast<size_t>(1) << 25;
        for (int batch = 0; batch < batches && !isCancelled(); batch++)
        {
            u32 zero = 0;
            params[8] = batch;
            cl->clEnqueueWriteBuffer(queue, countBuffer, CL_TRUE, 0, sizeof(u32), &zero, 0, nullptr, nullptr);
            cl->clEnqueueWriteBuffer(queue, paramBuffer, CL_TRUE, 0, sizeof(params), params, 0, nullptr, nullptr);
            err = cl->clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, nullptr, 0, nullptr, nullptr);
            if (err != CL_SUCCESS)
            {
                return fail("Failed to run OpenCL kernel", err);
            }
            u32 count = 0;
            err = cl->clEnqueueReadBuffer(queue, countBuffer, CL_TRUE, 0, sizeof(u32), &count, 0, nullptr, nullptr);
            if (err != CL_SUCCESS)
            {
                return fail("Failed to read OpenCL buffer", err);
            }
            count = std::min(count, MAX_OUT);
            if (count)
            {
                cl->clEnqueueReadBuffer(queue, outBuffer, CL_TRUE, 0, sizeof(u64) * count, found.get(), 0, nullptr, nullptr);
                for (u32 i = 0; i < count; i++)
                {
                    for (u32 r = 0; r < 3 && nat[0] >= 10 + r; r++)
                    {
                        if (verify(found[i], nat[0] - 10 - r))
                        {
                            std::lock_guard<std::mutex> guard(mutex);
                            results.push_back({ found[i], nat[0] - 10 - r });
                            cancelled = true; // one seed is the answer
                        }
                    }
                }
            }
        }
        progress++;
    }

    release();
    return true;
}
