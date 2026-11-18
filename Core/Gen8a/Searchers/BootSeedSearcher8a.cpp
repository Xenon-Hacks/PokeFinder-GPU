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

#include "BootSeedSearcher8a.hpp"
#include <Core/Gen8a/Generator8a.hpp>
#include <Core/Gen8a/Linear8a.hpp>
#include <Core/Gen8a/Templates8a.hpp>
#include <Core/Util/OpenCL.hpp>
#include <algorithm>
#include <memory>

using namespace OpenCL;

// Device code: one work item per candidate of one (start advance, gap) plan. Observations arrive as -D defines:
//   NK unown count; IV0..IVn (6 per Unown), AB0.. (255 = unknown), NA0.., WL0.. / WH0.. weight range
// Candidate words: 0 seed, 1/2 state at the first EC, 3/4 state at check 1, 5/6 state at check 2
static const char *kernelSource = R"CL(
inline ulong rotl64(ulong x, uint k)
{
    return (x << k) | (x >> (64 - k));
}

inline ulong nx(ulong *a, ulong *b)
{
    ulong r = *a + *b;
    ulong t = *a ^ *b;
    *a = rotl64(*a, 24) ^ t ^ (t << 16);
    *b = rotl64(t, 37);
    return r;
}

__constant uchar ivs[NK * 6] = { IVLIST };
__constant uchar abl[NK] = { ABLIST };
__constant uchar nat[NK] = { NALIST };
__constant uchar wlo[NK] = { WLLIST };
__constant uchar whi[NK] = { WHLIST };

inline bool verify(ulong a, ulong b)
{
    for (uint k = 0; k < NK; k++)
    {
        nx(&a, &b);
        nx(&a, &b);
        nx(&a, &b);
        for (uint i = 0; i < 6; i++)
        {
            if ((uint)(nx(&a, &b) & 31) != ivs[k * 6 + i])
                return false;
        }
        uint ab = (uint)(nx(&a, &b) & 1);
        if (abl[k] != 255 && ab != abl[k])
            return false;
        uint v;
        do
        {
            v = (uint)(nx(&a, &b) & 31);
        } while (v >= 25);
        if (v != nat[k])
            return false;
        do
        {
            v = (uint)(nx(&a, &b) & 255);
        } while (v >= 129);
        v += (uint)(nx(&a, &b) & 127);
        if (v < wlo[k] || v > whi[k])
            return false;
    }
    return true;
}

__kernel void bootscan(ulong first, ulong total, __global const ulong *table, __global const uint *digits, uint ndigits,
                       ulong b0, ulong b1, ulong b2, ulong b3, ulong b4, ulong b5, ulong b6, uint check1, uint check2,
                       __global uint *count, __global ulong *out, uint cap)
{
    ulong gid = get_global_id(0);
    if (gid >= total)
        return;
    ulong index = first + gid;

    uint entry[40];
    for (int d = (int)ndigits - 1; d >= 0; d--)
    {
        uint radix = digits[d * 2];
        entry[d] = digits[d * 2 + 1] + (uint)(index % radix);
        index /= radix;
    }

    ulong c0 = b3, c1 = b4;
    for (uint d = 0; d < ndigits; d++)
    {
        c0 ^= table[entry[d] * 7 + 3];
        c1 ^= table[entry[d] * 7 + 4];
    }
    if ((uint)((c0 + c1) & 31) != check1)
        return;

    if (check2 < 32)
    {
        c0 = b5;
        c1 = b6;
        for (uint d = 0; d < ndigits; d++)
        {
            c0 ^= table[entry[d] * 7 + 5];
            c1 ^= table[entry[d] * 7 + 6];
        }
        if ((uint)((c0 + c1) & 31) != check2)
            return;
    }

    ulong x = b0, s0 = b1, s1 = b2;
    for (uint d = 0; d < ndigits; d++)
    {
        x ^= table[entry[d] * 7];
        s0 ^= table[entry[d] * 7 + 1];
        s1 ^= table[entry[d] * 7 + 2];
    }
    if (!verify(s0, s1))
        return;

    uint idx = atomic_inc(count);
    if (idx < cap)
        out[idx] = x;
}
)CL";

namespace
{
    struct Root
    {
        u32 advance;
        Linear8a::Plan plan;
        u32 check1;
        u32 check2; // 32 = none
    };

    constexpr u32 W = 7;

    /**
     * @brief Plan for one start advance and gap. Words: seed, state at the first EC, check 1, check 2
     */
    bool makeRoot(const Linear8a::Symbolic &sym, const std::vector<UnownObservation8a> &unown, u32 advance, u32 gap, Root &root)
    {
        std::vector<Linear8a::Obs5> obs5;
        std::vector<Linear8a::Obs1> obs1;
        u32 start = advance;
        for (size_t k = 0; k < unown.size() && k < 2; k++)
        {
            for (u32 i = 0; i < 6; i++)
            {
                obs5.push_back({ start + 3 + i, unown[k].ivs[i] });
            }
            if (unown[k].ability >= 0)
            {
                obs1.push_back({ start + 9, static_cast<u8>(unown[k].ability) });
            }
            start += 10 + gap;
        }

        // Pick the checks after the plan: the first observations it did not use
        Linear8a::Plan probe = Linear8a::makePlan(sym, obs5, obs1, {});
        if (!probe.feasible)
        {
            return false;
        }
        std::vector<u32> unused;
        for (size_t i = 0; i < obs5.size(); i++)
        {
            if (std::find(probe.used.begin(), probe.used.end(), static_cast<int>(i)) == probe.used.end())
            {
                unused.emplace_back(static_cast<u32>(i));
            }
        }
        if (unused.empty())
        {
            return false;
        }
        u32 c1 = unused[0];
        u32 c2 = unused.size() > 1 ? unused[1] : unused[0];

        root.advance = advance;
        root.plan = Linear8a::makePlan(sym, obs5, obs1, { advance, obs5[c1].step, obs5[c2].step });
        root.check1 = obs5[c1].value;
        root.check2 = unused.size() > 1 ? obs5[c2].value : 32;
        return root.plan.feasible;
    }

    Spec8a unownSpec()
    {
        return Templates8a::unown(0);
    }
}

BootSeedSearcher8a::BootSeedSearcher8a(const std::vector<UnownObservation8a> &unown, u32 minAdvance, u32 maxAdvance, u32 maxGap) :
    unown(unown), minAdvance(minAdvance), maxAdvance(maxAdvance), maxGap(std::max<u32>(maxGap, 3)), usedGPU(false)
{
}

u32 BootSeedSearcher8a::verify(u64 seed, u32 advance, std::vector<u8> *weights) const
{
    Xoro8a rng(seed);
    rng.advance(advance);
    Spec8a spec = unownSpec();
    for (const auto &obs : unown)
    {
        State8a state = Generator8a::generate(rng, spec, false);
        if (state.ivs != obs.ivs || state.nature != obs.nature || (obs.ability >= 0 && state.ability != obs.ability)
            || state.weight < obs.weightMin || state.weight > obs.weightMax)
        {
            return 0;
        }
        if (weights)
        {
            weights->emplace_back(state.weight);
        }
    }
    return rng.draws;
}

std::string BootSeedSearcher8a::getError() const
{
    return error;
}

bool BootSeedSearcher8a::getUsedGPU() const
{
    return usedGPU;
}

void BootSeedSearcher8a::startSearch(int threads, bool useGPU)
{
    setMaxProgress(std::max<u64>(static_cast<u64>(maxAdvance - minAdvance + 1) * (maxGap - 2), 1));
    activeThreads = 1;

    threadContainer.emplace_back([=, this] {
        if (unown.size() < 2)
        {
            error = "At least two Unown are needed";
        }
        else if (!useGPU || !searchGPU())
        {
            usedGPU = false;
            progress = 0;
            {
                std::lock_guard<std::mutex> guard(mutex);
                results.clear();
            }
            searchCPU(threads);
        }
        activeThreads--;
    });
}

void BootSeedSearcher8a::addResult(u64 seed, u32 advance)
{
    std::vector<u8> weights;
    u32 end = verify(seed, advance, &weights);
    if (!end)
    {
        return;
    }

    // The same seed is reached once per gap that fits its Unown: keep one
    std::lock_guard<std::mutex> guard(mutex);
    if (std::find(seen.begin(), seen.end(), seed) != seen.end())
    {
        return;
    }
    seen.emplace_back(seed);
    results.push_back({ seed, advance, end, weights });
}

void BootSeedSearcher8a::searchCPU(int threads)
{
    Linear8a::Symbolic sym(maxAdvance + 2 * (10 + maxGap) + 16);
    u32 gaps = maxGap - 2;
    u64 roots = static_cast<u64>(maxAdvance - minAdvance + 1) * gaps;
    std::atomic<u64> next(0);

    auto worker = [&] {
        u32 entry[64];
        while (!isCancelled())
        {
            u64 r = next.fetch_add(1);
            if (r >= roots)
            {
                break;
            }
            u32 advance = minAdvance + static_cast<u32>(r / gaps);
            u32 gap = 3 + static_cast<u32>(r % gaps);

            Root root;
            if (makeRoot(sym, unown, advance, gap, root))
            {
                const Linear8a::Plan &p = root.plan;
                const u64 *table = p.table.data();
                u32 digits = static_cast<u32>(p.radix.size());

                // Odometer over the digits with running XORs of the check 1 words
                std::vector<u64> acc0(digits + 1), acc1(digits + 1);
                acc0[0] = p.base[3];
                acc1[0] = p.base[4];
                std::vector<u32> value(digits, 0);
                for (u32 d = 0; d < digits; d++)
                {
                    entry[d] = p.offset[d];
                    acc0[d + 1] = acc0[d] ^ table[entry[d] * W + 3];
                    acc1[d + 1] = acc1[d] ^ table[entry[d] * W + 4];
                }

                u32 last = digits - 1;
                u32 lastRadix = p.radix[last];
                u32 lastOffset = p.offset[last];
                while (!isCancelled())
                {
                    u64 h0 = acc0[last];
                    u64 h1 = acc1[last];
                    for (u32 g = 0; g < lastRadix; g++)
                    {
                        const u64 *e = table + (lastOffset + g) * W;
                        u64 s0 = h0 ^ e[3];
                        u64 s1 = h1 ^ e[4];
                        if (static_cast<u32>((s0 + s1) & 31) != root.check1)
                        {
                            continue;
                        }
                        entry[last] = lastOffset + g;
                        if (root.check2 < 32)
                        {
                            u64 t0 = p.base[5], t1 = p.base[6];
                            for (u32 d = 0; d < digits; d++)
                            {
                                t0 ^= table[entry[d] * W + 5];
                                t1 ^= table[entry[d] * W + 6];
                            }
                            if (static_cast<u32>((t0 + t1) & 31) != root.check2)
                            {
                                continue;
                            }
                        }
                        u64 x = p.base[0];
                        for (u32 d = 0; d < digits; d++)
                        {
                            x ^= table[entry[d] * W];
                        }
                        addResult(x, advance);
                    }

                    // Advance the odometer (all digits but the last)
                    int d = static_cast<int>(last) - 1;
                    while (d >= 0)
                    {
                        if (++value[d] < p.radix[d])
                        {
                            break;
                        }
                        value[d] = 0;
                        d--;
                    }
                    if (d < 0)
                    {
                        break;
                    }
                    for (u32 k = static_cast<u32>(d); k < last; k++)
                    {
                        entry[k] = p.offset[k] + value[k];
                        acc0[k + 1] = acc0[k] ^ table[entry[k] * W + 3];
                        acc1[k + 1] = acc1[k] ^ table[entry[k] * W + 4];
                    }
                }
            }
            progress++;
        }
    };

    std::vector<std::thread> workers;
    for (int i = 1; i < std::max(threads, 1); i++)
    {
        workers.emplace_back(worker);
    }
    worker();
    for (auto &thread : workers)
    {
        thread.join();
    }
}

bool BootSeedSearcher8a::searchGPU()
{
    const API *cl = OpenCL::api();
    if (!cl)
    {
        error = "No OpenCL GPU found";
        return false;
    }

    auto list = [&](auto get) {
        std::string s;
        for (size_t k = 0; k < unown.size(); k++)
        {
            s += (k ? "," : "") + get(unown[k]);
        }
        return s;
    };
    std::string ivList;
    for (size_t k = 0; k < unown.size(); k++)
    {
        for (int i = 0; i < 6; i++)
        {
            ivList += (k || i ? "," : "") + std::to_string(unown[k].ivs[i]);
        }
    }
    std::string options = "-D NK=" + std::to_string(unown.size()) + " -D IVLIST=" + ivList
        + " -D ABLIST=" + list([](const UnownObservation8a &o) { return std::to_string(o.ability < 0 ? 255 : o.ability); })
        + " -D NALIST=" + list([](const UnownObservation8a &o) { return std::to_string(o.nature); })
        + " -D WLLIST=" + list([](const UnownObservation8a &o) { return std::to_string(o.weightMin); })
        + " -D WHLIST=" + list([](const UnownObservation8a &o) { return std::to_string(o.weightMax); });

    cl_device_id id = OpenCL::device();
    cl_int err;
    cl_context context = nullptr;
    cl_command_queue queue = nullptr;
    cl_program program = nullptr;
    cl_kernel kernel = nullptr;
    cl_mem tableBuffer = nullptr;
    cl_mem digitBuffer = nullptr;
    cl_mem countBuffer = nullptr;
    cl_mem out = nullptr;

    auto release = [&] {
        for (cl_mem mem : { out, countBuffer, digitBuffer, tableBuffer })
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
    err = cl->clBuildProgram(program, 1, &id, options.c_str(), nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        size_t size = 0;
        cl->clGetProgramBuildInfo(program, id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size);
        std::string log(size, '\0');
        cl->clGetProgramBuildInfo(program, id, CL_PROGRAM_BUILD_LOG, size, log.data(), nullptr);
        return fail("Failed to build OpenCL program: " + log, err);
    }
    kernel = cl->clCreateKernel(program, "bootscan", &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL kernel", err);
    }

    constexpr u32 maxEntries = 40 * 16;
    constexpr u32 capacity = 1 << 12;
    tableBuffer = cl->clCreateBuffer(context, CL_MEM_READ_ONLY, maxEntries * W * sizeof(u64), nullptr, &err);
    if (err == CL_SUCCESS)
    {
        digitBuffer = cl->clCreateBuffer(context, CL_MEM_READ_ONLY, 80 * sizeof(u32), nullptr, &err);
    }
    if (err == CL_SUCCESS)
    {
        countBuffer = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(u32), nullptr, &err);
    }
    if (err == CL_SUCCESS)
    {
        out = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, capacity * sizeof(u64), nullptr, &err);
    }
    if (err != CL_SUCCESS)
    {
        return fail("Failed to allocate GPU memory", err);
    }

    Linear8a::Symbolic sym(maxAdvance + 2 * (10 + maxGap) + 16);
    u32 gaps = maxGap - 2;
    usedGPU = true;

    // Keep one launch to a fraction of a second so the display stays responsive
    constexpr u64 perLaunch = 1ULL << 26;

    for (u32 advance = minAdvance; advance <= maxAdvance && !isCancelled(); advance++)
    {
        for (u32 gap = 3; gap < 3 + gaps && !isCancelled(); gap++)
        {
            Root root;
            if (makeRoot(sym, unown, advance, gap, root))
            {
                const Linear8a::Plan &p = root.plan;
                u32 digits = static_cast<u32>(p.radix.size());
                if (digits > 40 || p.table.size() / W > maxEntries)
                {
                    usedGPU = false;
                    return fail("Search plan too large for the GPU kernel", 0);
                }
                std::vector<u32> digitData;
                for (u32 d = 0; d < digits; d++)
                {
                    digitData.emplace_back(p.radix[d]);
                    digitData.emplace_back(p.offset[d]);
                }
                err = cl->clEnqueueWriteBuffer(queue, tableBuffer, CL_TRUE, 0, p.table.size() * sizeof(u64), p.table.data(), 0, nullptr,
                                               nullptr);
                err |= cl->clEnqueueWriteBuffer(queue, digitBuffer, CL_TRUE, 0, digitData.size() * sizeof(u32), digitData.data(), 0,
                                                nullptr, nullptr);

                u64 total = p.count();
                for (u64 first = 0; first < total && err == CL_SUCCESS && !isCancelled(); first += perLaunch)
                {
                    u64 n = std::min(perLaunch, total - first);
                    u32 zero = 0;
                    err |= cl->clEnqueueWriteBuffer(queue, countBuffer, CL_TRUE, 0, sizeof(u32), &zero, 0, nullptr, nullptr);
                    cl_uint arg = 0;
                    err |= cl->clSetKernelArg(kernel, arg++, sizeof(u64), &first);
                    err |= cl->clSetKernelArg(kernel, arg++, sizeof(u64), &n);
                    err |= cl->clSetKernelArg(kernel, arg++, sizeof(cl_mem), &tableBuffer);
                    err |= cl->clSetKernelArg(kernel, arg++, sizeof(cl_mem), &digitBuffer);
                    err |= cl->clSetKernelArg(kernel, arg++, sizeof(u32), &digits);
                    for (u32 w = 0; w < W; w++)
                    {
                        err |= cl->clSetKernelArg(kernel, arg++, sizeof(u64), &p.base[w]);
                    }
                    err |= cl->clSetKernelArg(kernel, arg++, sizeof(u32), &root.check1);
                    err |= cl->clSetKernelArg(kernel, arg++, sizeof(u32), &root.check2);
                    err |= cl->clSetKernelArg(kernel, arg++, sizeof(cl_mem), &countBuffer);
                    err |= cl->clSetKernelArg(kernel, arg++, sizeof(cl_mem), &out);
                    err |= cl->clSetKernelArg(kernel, arg++, sizeof(u32), &capacity);

                    size_t global = static_cast<size_t>(n);
                    err |= cl->clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, nullptr, 0, nullptr, nullptr);
                    u32 found = 0;
                    err |= cl->clEnqueueReadBuffer(queue, countBuffer, CL_TRUE, 0, sizeof(u32), &found, 0, nullptr, nullptr);
                    if (err == CL_SUCCESS && found > 0)
                    {
                        std::vector<u64> seeds(std::min(found, capacity));
                        err |= cl->clEnqueueReadBuffer(queue, out, CL_TRUE, 0, seeds.size() * sizeof(u64), seeds.data(), 0, nullptr,
                                                       nullptr);
                        for (u64 seed : seeds)
                        {
                            addResult(seed, advance);
                        }
                    }
                }
                if (err != CL_SUCCESS)
                {
                    usedGPU = false;
                    return fail("GPU search failed", err);
                }
            }
            progress++;
        }
    }

    release();
    return true;
}
