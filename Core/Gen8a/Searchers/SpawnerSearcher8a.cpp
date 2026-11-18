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

#include "SpawnerSearcher8a.hpp"
#include <Core/Gen8a/Generator8a.hpp>
#include <Core/Gen8a/Linear8a.hpp>
#include <Core/Util/OpenCL.hpp>
#include <algorithm>
#include <bit>

using namespace OpenCL;

// Device code for step 1: one work item per candidate entity seed of one plan (PID draw count). The plan words are
// 0 entity seed, 1/2 state after the ability draw. The filter arrives as -D defines: RATIO, GEN (255 any), NAT (255 any),
// HLO/HHI, WLO/WHI. Survivors go to the CPU for the full check.
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

inline uint rnd(ulong *a, ulong *b, uint n, uint mask)
{
    uint v;
    do
    {
        v = (uint)(nx(a, b) & mask);
    } while (v >= n);
    return v;
}

__kernel void entityscan(ulong first, ulong total, __global const ulong *table, __global const uint *digits, uint ndigits,
                         ulong b0, ulong b1, ulong b2, __global uint *count, __global ulong *out, uint cap)
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

    ulong a = b1, b = b2;
    for (uint d = 0; d < ndigits; d++)
    {
        a ^= table[entry[d] * 3 + 1];
        b ^= table[entry[d] * 3 + 2];
    }

#if RATIO != 0 && RATIO < 254
    uint g = rnd(&a, &b, 253, 255) + 1 < RATIO ? 1 : 0;
#if GEN != 255
    if (g != GEN)
        return;
#endif
#endif
    uint n = rnd(&a, &b, 25, 31);
#if NAT != 255
    if (n != NAT)
        return;
#endif
    uint h = rnd(&a, &b, 129, 255);
    h += (uint)(nx(&a, &b) & 127);
    if (h < HLO || h > HHI)
        return;
    uint w = rnd(&a, &b, 129, 255);
    w += (uint)(nx(&a, &b) & 127);
    if (w < WLO || w > WHI)
        return;

    ulong x = b0;
    for (uint d = 0; d < ndigits; d++)
        x ^= table[entry[d] * 3];

    uint idx = atomic_inc(count);
    if (idx < cap)
        out[idx] = x;
}
)CL";

namespace
{
    constexpr u32 W = 3;

    std::vector<int> pidDraws(const WildObservation8a &obs)
    {
        std::vector<int> result;
        int rolls = std::max<int>(obs.rolls, 1);
        if (obs.shiny == 0)
        {
            result.emplace_back(rolls);
        }
        else
        {
            for (int i = 1; i <= rolls; i++)
            {
                result.emplace_back(i);
            }
        }
        return result;
    }

    Linear8a::Plan entityPlan(const Linear8a::Symbolic &sym, const WildObservation8a &obs, u32 t0)
    {
        std::vector<Linear8a::Obs5> obs5;
        std::vector<Linear8a::Obs1> obs1;
        for (u32 i = 0; i < 6; i++)
        {
            obs5.push_back({ t0 + i, obs.ivs[i] });
        }
        if (obs.ability >= 0)
        {
            obs1.push_back({ t0 + 6, static_cast<u8>(obs.ability) });
        }
        return Linear8a::makePlan(sym, obs5, obs1, { t0 + 7 });
    }

    /**
     * @brief Quick filter from the state after the ability draw: gender, nature, height, weight
     */
    bool quick(u64 s0, u64 s1, const WildObservation8a &obs)
    {
        Xoro8a rng(s0, s1);
        u8 ratio = obs.genderRatio;
        if (ratio != 0 && ratio < 254)
        {
            u32 g = rng.rand(253, true) + 1 < ratio ? 1 : 0;
            if (obs.gender >= 0 && g != static_cast<u32>(obs.gender))
            {
                return false;
            }
        }
        u32 nature = rng.rand(25, true);
        if (obs.nature >= 0 && nature != static_cast<u32>(obs.nature))
        {
            return false;
        }
        u32 h = rng.rand(129, true);
        h += rng.rand(128, true);
        if (h < obs.heightMin || h > obs.heightMax)
        {
            return false;
        }
        u32 w = rng.rand(129, true);
        w += rng.rand(128, true);
        return w >= obs.weightMin && w <= obs.weightMax;
    }

    // Step 2 data: Xoroshiro(s, C) after one step is (A s ^ ka, B s ^ kb); the entity seed is their sum
    struct SlotSystem
    {
        u64 a[64];
        u64 d[64];
        u64 ka;
        u64 kd;

        SlotSystem()
        {
            Linear8a::Symbolic sym(1);
            for (int i = 0; i < 64; i++)
            {
                a[i] = sym.s0[1].mask[i];
                d[i] = sym.s0[1].mask[i] ^ sym.s1[1].mask[i];
            }
            ka = sym.s0[1].constant;
            kd = sym.s0[1].constant ^ sym.s1[1].constant;
        }
    };

    u64 entityOf(u64 slot)
    {
        Xoro8a rng(slot);
        rng.next();
        return rng.next();
    }

    /**
     * @brief Carry-guessing search for x + y = e where x, y are the two state words after one step. Bit j always gives
     * x_j ^ y_j = e_j ^ carry; only when that is 0 is the carry out unknown (it equals x_j), so only those bits branch.
     * Equations are kept in a pivot basis with an undo log
     */
    struct SlotInverter
    {
        const SlotSystem &sys;
        u64 e;
        u64 row[64];
        u64 rhs = 0;
        u64 have = 0;
        std::vector<u64> found;

        SlotInverter(const SlotSystem &sys, u64 e) : sys(sys), e(e)
        {
        }

        // pivot, -1 dependent and consistent, -2 contradiction
        int add(u64 m, u32 r)
        {
            while (m)
            {
                int p = 63 - std::countl_zero(m);
                if (!(have >> p & 1))
                {
                    row[p] = m;
                    rhs = (rhs & ~(1ULL << p)) | (static_cast<u64>(r) << p);
                    have |= 1ULL << p;
                    return p;
                }
                m ^= row[p];
                r ^= (rhs >> p) & 1;
            }
            return r ? -2 : -1;
        }

        void undo(int p)
        {
            if (p >= 0)
            {
                have &= ~(1ULL << p);
            }
        }

        u64 solve(u64 freeBits) const
        {
            u64 x = 0;
            int fi = 0;
            for (int p = 0; p < 64; p++)
            {
                if (!(have >> p & 1))
                {
                    if ((freeBits >> fi++) & 1)
                    {
                        x |= 1ULL << p;
                    }
                    continue;
                }
                u64 m = row[p] & ~(1ULL << p);
                u32 r = static_cast<u32>((rhs >> p) & 1) ^ (std::popcount(m & x) & 1);
                x |= static_cast<u64>(r) << p;
            }
            return x;
        }

        void leaf()
        {
            int free = 64 - std::popcount(have);
            if (free > 16)
            {
                return;
            }
            for (u64 f = 0; f < (1ULL << free); f++)
            {
                u64 s = solve(f);
                if (entityOf(s) == e && std::find(found.begin(), found.end(), s) == found.end())
                {
                    found.emplace_back(s);
                }
            }
        }

        void dfs(int j, u32 carry)
        {
            if (have == ~0ULL || j == 64)
            {
                leaf();
                return;
            }
            u32 eb = (e >> j) & 1;
            int p1 = add(sys.d[j], eb ^ carry ^ static_cast<u32>((sys.kd >> j) & 1));
            if (p1 == -2)
            {
                return;
            }
            if (eb ^ carry)
            {
                dfs(j + 1, carry);
            }
            else
            {
                for (u32 g = 0; g < 2; g++)
                {
                    int p2 = add(sys.a[j], g ^ static_cast<u32>((sys.ka >> j) & 1));
                    if (p2 != -2)
                    {
                        dfs(j + 1, g);
                        undo(p2);
                    }
                }
            }
            undo(p1);
        }
    };
}

bool WildObservation8a::matches(u64 entity) const
{
    Spec8a spec;
    spec.pid = Spec8a::randomPID;
    spec.rolls = std::max<u8>(rolls, 1);
    spec.genderRatio = genderRatio;
    Xoro8a rng(entity);
    State8a state = Generator8a::generate(rng, spec, true);
    if ((shiny >= 0 && state.shiny != (shiny == 1)) || state.ivs != ivs || (ability >= 0 && state.ability != ability)
        || (gender >= 0 && genderRatio != 0 && genderRatio < 254 && state.gender != gender) || (nature >= 0 && state.nature != nature))
    {
        return false;
    }
    return state.height >= heightMin && state.height <= heightMax && state.weight >= weightMin && state.weight <= weightMax;
}

SpawnerSearcher8a::SpawnerSearcher8a(const WildObservation8a &first, const WildObservation8a &second) :
    first(first), second(second), usedGPU(false), candidates(0)
{
}

std::vector<u64> SpawnerSearcher8a::entityCandidates(const WildObservation8a &obs, int threads, u64 cap, bool *overflow)
{
    Linear8a::Symbolic sym(2 + obs.rolls + 8);
    std::vector<u64> out;
    std::mutex lock;
    *overflow = false;

    for (int draws : pidDraws(obs))
    {
        u32 t0 = 2 + draws;
        Linear8a::Plan p = entityPlan(sym, obs, t0);
        if (!p.feasible || p.radix.size() < 3)
        {
            continue;
        }
        const u64 *table = p.table.data();
        u32 digits = static_cast<u32>(p.radix.size());
        u32 tasks = p.radix[0] * p.radix[1];
        std::atomic<u32> next(0);

        auto worker = [&] {
            std::vector<u64> local;
            std::vector<u32> value(digits, 0);
            std::vector<u64> acc0(digits + 1), acc1(digits + 1), accx(digits + 1);
            while (!*overflow)
            {
                u32 task = next.fetch_add(1);
                if (task >= tasks)
                {
                    break;
                }
                value.assign(digits, 0);
                value[0] = task / p.radix[1];
                value[1] = task % p.radix[1];
                accx[0] = p.base[0];
                acc0[0] = p.base[1];
                acc1[0] = p.base[2];
                for (u32 d = 0; d < digits; d++)
                {
                    const u64 *en = table + (p.offset[d] + value[d]) * W;
                    accx[d + 1] = accx[d] ^ en[0];
                    acc0[d + 1] = acc0[d] ^ en[1];
                    acc1[d + 1] = acc1[d] ^ en[2];
                }

                u32 last = digits - 1;
                while (true)
                {
                    for (u32 g = 0; g < p.radix[last]; g++)
                    {
                        const u64 *en = table + (p.offset[last] + g) * W;
                        if (quick(acc0[last] ^ en[1], acc1[last] ^ en[2], obs))
                        {
                            u64 x = accx[last] ^ en[0];
                            if (obs.matches(x))
                            {
                                local.emplace_back(x);
                            }
                        }
                    }
                    int d = static_cast<int>(last) - 1;
                    while (d >= 2)
                    {
                        if (++value[d] < p.radix[d])
                        {
                            break;
                        }
                        value[d] = 0;
                        d--;
                    }
                    if (d < 2)
                    {
                        break;
                    }
                    for (u32 k = static_cast<u32>(d); k < last; k++)
                    {
                        const u64 *en = table + (p.offset[k] + value[k]) * W;
                        accx[k + 1] = accx[k] ^ en[0];
                        acc0[k + 1] = acc0[k] ^ en[1];
                        acc1[k + 1] = acc1[k] ^ en[2];
                    }
                }

                std::lock_guard<std::mutex> guard(lock);
                out.insert(out.end(), local.begin(), local.end());
                local.clear();
                if (out.size() > cap)
                {
                    *overflow = true;
                }
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

    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

std::vector<u64> SpawnerSearcher8a::slotSeeds(u64 entity)
{
    static const SlotSystem system;
    SlotInverter inverter(system, entity);
    inverter.dfs(0, 0);
    return inverter.found;
}

std::string SpawnerSearcher8a::getError() const
{
    return error;
}

bool SpawnerSearcher8a::getUsedGPU() const
{
    return usedGPU;
}

u64 SpawnerSearcher8a::getCandidates() const
{
    return candidates;
}

void SpawnerSearcher8a::checkSecond(u64 slot1, u64 entity1)
{
    u64 g0 = slot1 - Xoro8a::C;
    Xoro8a rng(g0);
    u64 out[10];
    for (u64 &o : out)
    {
        o = rng.next();
    }

    auto add = [&](u64 slot2, SpawnerRelation8a relation, u8 position) {
        u64 entity2 = entityOf(slot2);
        if (second.matches(entity2))
        {
            std::lock_guard<std::mutex> guard(mutex);
            results.push_back({ g0, slot1, entity1, slot2, entity2, relation, position });
        }
    };

    for (u8 k = 2; k <= 4; k++)
    {
        add(out[2 * k - 2], SpawnerRelation8a::SameFill, k);
    }
    for (u8 n = 1; n <= 4; n++)
    {
        add(out[2 * n] + Xoro8a::C, SpawnerRelation8a::NextFill, n);
    }
}

void SpawnerSearcher8a::startSearch(int threads, bool useGPU, u64 maxCandidates)
{
    setMaxProgress(1);
    activeThreads = 1;

    threadContainer.emplace_back([=, this] {
        std::vector<u64> entities;
        bool overflow = false;
        if (!useGPU || !candidatesGPU(entities, maxCandidates, &overflow))
        {
            usedGPU = false;
            entities = entityCandidates(first, threads, maxCandidates, &overflow);
        }
        candidates = entities.size();

        if (overflow)
        {
            error = "More than " + std::to_string(maxCandidates)
                + " entity seeds fit the first Pokémon. Narrow the height and weight ranges (or check the IVs) and search again";
        }
        else
        {
            setMaxProgress(std::max<u64>(entities.size(), 1));
            std::atomic<size_t> next(0);
            auto worker = [&] {
                while (!isCancelled())
                {
                    size_t i = next.fetch_add(1);
                    if (i >= entities.size())
                    {
                        break;
                    }
                    for (u64 slot : slotSeeds(entities[i]))
                    {
                        checkSecond(slot, entities[i]);
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
        activeThreads--;
    });
}

bool SpawnerSearcher8a::candidatesGPU(std::vector<u64> &result, u64 cap, bool *overflow)
{
    const API *cl = OpenCL::api();
    if (!cl)
    {
        error = "No OpenCL GPU found";
        return false;
    }
    const WildObservation8a &obs = first;
    std::string options = "-D RATIO=" + std::to_string(obs.genderRatio) + " -D GEN=" + std::to_string(obs.gender < 0 ? 255 : obs.gender)
        + " -D NAT=" + std::to_string(obs.nature < 0 ? 255 : obs.nature) + " -D HLO=" + std::to_string(obs.heightMin) + " -D HHI="
        + std::to_string(obs.heightMax) + " -D WLO=" + std::to_string(obs.weightMin) + " -D WHI=" + std::to_string(obs.weightMax);

    cl_device_id id = OpenCL::device();
    cl_int err;
    cl_context context = nullptr;
    cl_command_queue queue = nullptr;
    cl_program program = nullptr;
    cl_kernel kernel = nullptr;
    cl_mem tableBuffer = nullptr, digitBuffer = nullptr, countBuffer = nullptr, out = nullptr;

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
    kernel = cl->clCreateKernel(program, "entityscan", &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL kernel", err);
    }

    constexpr u32 maxEntries = 40 * 16;
    u32 capacity = static_cast<u32>(std::min<u64>(cap + 1, 1ULL << 24));
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
        out = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, static_cast<size_t>(capacity) * sizeof(u64), nullptr, &err);
    }
    if (err != CL_SUCCESS)
    {
        return fail("Failed to allocate GPU memory", err);
    }

    Linear8a::Symbolic sym(2 + obs.rolls + 8);
    constexpr u64 perLaunch = 1ULL << 26;
    usedGPU = true;
    *overflow = false;
    std::vector<u64> raw;

    for (int draws : pidDraws(obs))
    {
        Linear8a::Plan p = entityPlan(sym, obs, 2 + draws);
        if (!p.feasible)
        {
            continue;
        }
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
        err = cl->clEnqueueWriteBuffer(queue, tableBuffer, CL_TRUE, 0, p.table.size() * sizeof(u64), p.table.data(), 0, nullptr, nullptr);
        err |= cl->clEnqueueWriteBuffer(queue, digitBuffer, CL_TRUE, 0, digitData.size() * sizeof(u32), digitData.data(), 0, nullptr,
                                        nullptr);

        u64 total = p.count();
        for (u64 firstIndex = 0; firstIndex < total && err == CL_SUCCESS && !isCancelled() && !*overflow; firstIndex += perLaunch)
        {
            u64 n = std::min(perLaunch, total - firstIndex);
            u32 zero = 0;
            err |= cl->clEnqueueWriteBuffer(queue, countBuffer, CL_TRUE, 0, sizeof(u32), &zero, 0, nullptr, nullptr);
            cl_uint arg = 0;
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(u64), &firstIndex);
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(u64), &n);
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(cl_mem), &tableBuffer);
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(cl_mem), &digitBuffer);
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(u32), &digits);
            for (u32 w = 0; w < W; w++)
            {
                err |= cl->clSetKernelArg(kernel, arg++, sizeof(u64), &p.base[w]);
            }
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(cl_mem), &countBuffer);
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(cl_mem), &out);
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(u32), &capacity);
            size_t global = static_cast<size_t>(n);
            err |= cl->clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, nullptr, 0, nullptr, nullptr);
            u32 found = 0;
            err |= cl->clEnqueueReadBuffer(queue, countBuffer, CL_TRUE, 0, sizeof(u32), &found, 0, nullptr, nullptr);
            if (err == CL_SUCCESS && found > 0)
            {
                if (found >= capacity || raw.size() + found > cap * 4 + 1024)
                {
                    *overflow = true;
                    break;
                }
                size_t old = raw.size();
                raw.resize(old + found);
                err |= cl->clEnqueueReadBuffer(queue, out, CL_TRUE, 0, found * sizeof(u64), raw.data() + old, 0, nullptr, nullptr);
            }
        }
        if (err != CL_SUCCESS)
        {
            usedGPU = false;
            return fail("GPU search failed", err);
        }
    }
    release();

    // Full check on the CPU (PID draw count, shiny state)
    for (u64 x : raw)
    {
        if (first.matches(x))
        {
            result.emplace_back(x);
        }
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    if (result.size() > cap)
    {
        *overflow = true;
    }
    return true;
}
