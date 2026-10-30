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

#include "SeedSearcher6.hpp"
#include <Core/Gen6/Pokemon6.hpp>
#include <Core/RNG/MT.hpp>
#include <Core/Util/OpenCL.hpp>
#include <algorithm>
#include <vector>

using namespace OpenCL;

// Outputs a single frame may read: perfect IV picks (re-rolls included), 6 IVs, ability and nature
constexpr u32 look = 40;

// Device code. Search settings arrive as -D defines:
//   FIRST        MT output index of the first IV draw at frame FMIN (FMIN + skip)
//   FCOUNT       number of trigger frames
//   IVCNT        perfect IVs picked with rand(6)
//   IVLO0..5/IVHI0..5  per stat IV bounds, PokeFinder stat order
//   NATURE/NATOFF      nature to match (255 any) and outputs between the last IV and the nature
static const char *kernelSource = R"CL(
#define LOOK 40
#define RING 64

inline uint temper(uint y)
{
    y ^= y >> 11;
    y ^= (y << 7) & 0x9d2c5680u;
    y ^= (y << 15) & 0xefc60000u;
    y ^= y >> 18;
    return y;
}

inline void twist(uint *mt)
{
    for (int i = 0; i < 227; i++)
    {
        uint y = (mt[i] & 0x80000000u) | (mt[i + 1] & 0x7fffffffu);
        mt[i] = mt[i + 397] ^ (y >> 1) ^ ((y & 1) * 0x9908b0dfu);
    }
    for (int i = 227; i < 623; i++)
    {
        uint y = (mt[i] & 0x80000000u) | (mt[i + 1] & 0x7fffffffu);
        mt[i] = mt[i - 227] ^ (y >> 1) ^ ((y & 1) * 0x9908b0dfu);
    }
    uint y = (mt[623] & 0x80000000u) | (mt[0] & 0x7fffffffu);
    mt[623] = mt[396] ^ (y >> 1) ^ ((y & 1) * 0x9908b0dfu);
}

inline uint rd(const uint *ring, uint base, uint k)
{
    return ring[(base + k) & (RING - 1)];
}

inline bool check(const uint *ring, uint base)
{
    const uchar lo[6] = { IVLO0, IVLO1, IVLO2, IVLO3, IVLO4, IVLO5 };
    const uchar hi[6] = { IVHI0, IVHI1, IVHI2, IVHI3, IVHI4, IVHI5 };

    uint k = 0;
    uchar ivs[6] = { 255, 255, 255, 255, 255, 255 };
    for (uint picks = 0; picks < IVCNT;)
    {
        if (k >= LOOK - 8)
        {
            return false;
        }
        uint index = (uint)(((ulong)rd(ring, base, k++) * 6) >> 32);
        if (ivs[index] == 255)
        {
            ivs[index] = 31;
            if (31 < lo[index] || 31 > hi[index])
            {
                return false;
            }
            picks++;
        }
    }

    for (int i = 0; i < 6; i++)
    {
        if (ivs[i] == 255)
        {
            uchar iv = rd(ring, base, k++) >> 27;
            if (iv < lo[i] || iv > hi[i])
            {
                return false;
            }
        }
    }

#if NATURE != 255
    k += NATOFF;
    if ((uint)(((ulong)rd(ring, base, k) * 25) >> 32) != NATURE)
    {
        return false;
    }
#endif
    return true;
}

__kernel void seedscan(uint start, uint total, __global uint *count, __global uint *out, uint capacity)
{
    uint gid = get_global_id(0);
    if (gid >= total)
    {
        return;
    }

    uint seed = start + gid;
    uint mt[624];
    mt[0] = seed;
    for (uint i = 1; i < 624; i++)
    {
        mt[i] = 0x6c078965u * (mt[i - 1] ^ (mt[i - 1] >> 30)) + i;
    }

    uint blocks = FIRST / 624;
    uint pos = FIRST % 624;
    for (uint b = 0; b <= blocks; b++)
    {
        twist(mt);
    }

    uint ring[RING];
    for (uint j = 0; j < LOOK; j++)
    {
        if (pos == 624)
        {
            twist(mt);
            pos = 0;
        }
        ring[j] = temper(mt[pos++]);
    }

    for (uint f = 0; f < FCOUNT; f++)
    {
        if (check(ring, f))
        {
            uint slot = atomic_inc(count);
            if (slot < capacity)
            {
                out[slot * 2] = seed;
                out[slot * 2 + 1] = f;
            }
        }

        if (pos == 624)
        {
            twist(mt);
            pos = 0;
        }
        ring[(f + LOOK) & (RING - 1)] = temper(mt[pos++]);
    }
}
)CL";

SeedSearcher6::SeedSearcher6(const SeedSearch6Settings &settings) : settings(settings), clockStart(0), usedGPU(false)
{
}

bool SeedSearcher6::matches(const SeedSearch6Settings &settings, const u32 *outputs, u32 size)
{
    u32 k = 0;
    std::array<u8, 6> ivs = { 255, 255, 255, 255, 255, 255 };
    for (u8 picks = 0; picks < settings.ivCount;)
    {
        if (k >= size - 8)
        {
            return false;
        }
        u8 index = Gen6::rand(outputs[k++], 6);
        if (ivs[index] == 255)
        {
            ivs[index] = 31;
            if (31 < settings.ivMin[index] || 31 > settings.ivMax[index])
            {
                return false;
            }
            picks++;
        }
    }

    for (u8 i = 0; i < 6; i++)
    {
        if (ivs[i] == 255)
        {
            u8 iv = outputs[k++] >> 27;
            if (iv < settings.ivMin[i] || iv > settings.ivMax[i])
            {
                return false;
            }
        }
    }

    if (settings.nature != 255)
    {
        k += settings.natureOffset;
        if (Gen6::rand(outputs[k], 25) != settings.nature)
        {
            return false;
        }
    }
    return true;
}

std::vector<SeedResult6> SeedSearcher6::narrow(const SeedSearch6Settings &settings, const std::vector<SeedResult6> &candidates,
                                               u32 minGap, u32 maxGap)
{
    u32 gaps = maxGap - minGap + 1;
    std::vector<u32> outputs(gaps + look);

    std::vector<SeedResult6> results;
    for (const auto &candidate : candidates)
    {
        MT mt(candidate.seed, candidate.frame + minGap + settings.skip);
        for (u32 &output : outputs)
        {
            output = mt.next();
        }

        for (u32 gap = 0; gap < gaps; gap++)
        {
            if (matches(settings, &outputs[gap], look))
            {
                SeedResult6 result = candidate;
                result.frame = candidate.frame + minGap + gap;
                results.emplace_back(result);
            }
        }
    }
    return results;
}

std::string SeedSearcher6::getError() const
{
    return error;
}

bool SeedSearcher6::getUsedGPU() const
{
    return usedGPU;
}

void SeedSearcher6::startSearch(int threads, bool useGPU, u32 start, u64 count, u64 clockStart)
{
    this->clockStart = clockStart;
    setMaxProgress(std::max<u64>(count, 1));
    activeThreads = 1;

    threadContainer.emplace_back([=, this] {
        if (!useGPU || !searchGPU(start, count))
        {
            usedGPU = false;
            progress = 0;
            {
                // A GPU failure part way through leaves partial results, the CPU pass finds them again
                std::lock_guard<std::mutex> guard(mutex);
                results.clear();
            }
            searchCPU(start, count, threads);
        }
        activeThreads--;
    });
}

void SeedSearcher6::addResult(u32 seed, u32 frame, u32 start)
{
    SeedResult6 result;
    result.seed = seed;
    result.frame = settings.minFrame + frame;
    result.ms = clockStart == 0 ? 0 : clockStart + static_cast<u32>(seed - start);

    std::lock_guard<std::mutex> guard(mutex);
    results.emplace_back(result);
}

void SeedSearcher6::searchCPU(u32 start, u64 count, int threads)
{
    constexpr u64 chunk = 0x1000;
    u32 frames = settings.maxFrame - settings.minFrame + 1;
    std::atomic<u64> next(0);

    auto worker = [&] {
        std::vector<u32> outputs(frames + look);
        while (!isCancelled())
        {
            u64 begin = next.fetch_add(chunk);
            if (begin >= count)
            {
                break;
            }
            u64 end = std::min(begin + chunk, count);

            for (u64 i = begin; i < end; i++)
            {
                u32 seed = start + static_cast<u32>(i);
                MT mt(seed, settings.minFrame + settings.skip);
                for (u32 &output : outputs)
                {
                    output = mt.next();
                }

                for (u32 frame = 0; frame < frames; frame++)
                {
                    if (matches(settings, &outputs[frame], look))
                    {
                        addResult(seed, frame, start);
                    }
                }
            }
            progress += end - begin;
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

bool SeedSearcher6::searchGPU(u32 start, u64 count)
{
    const API *cl = OpenCL::api();
    if (!cl)
    {
        error = "No OpenCL GPU found";
        return false;
    }

    u32 frames = settings.maxFrame - settings.minFrame + 1;
    std::string options = "-D FIRST=" + std::to_string(settings.minFrame + settings.skip) + " -D FCOUNT=" + std::to_string(frames)
        + " -D IVCNT=" + std::to_string(settings.ivCount) + " -D NATURE=" + std::to_string(settings.nature)
        + " -D NATOFF=" + std::to_string(settings.natureOffset);
    for (int i = 0; i < 6; i++)
    {
        options += " -D IVLO" + std::to_string(i) + "=" + std::to_string(settings.ivMin[i]);
        options += " -D IVHI" + std::to_string(i) + "=" + std::to_string(settings.ivMax[i]);
    }

    cl_device_id id = OpenCL::device();
    cl_int err;
    cl_context context = nullptr;
    cl_command_queue queue = nullptr;
    cl_program program = nullptr;
    cl_kernel kernel = nullptr;
    cl_mem countBuffer = nullptr;
    cl_mem out = nullptr;

    auto release = [&] {
        if (out)
        {
            cl->clReleaseMemObject(out);
        }
        if (countBuffer)
        {
            cl->clReleaseMemObject(countBuffer);
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

    kernel = cl->clCreateKernel(program, "seedscan", &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL kernel", err);
    }

    u32 capacity = 1 << 16;
    countBuffer = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(u32), nullptr, &err);
    if (err == CL_SUCCESS)
    {
        out = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, static_cast<size_t>(capacity) * 2 * sizeof(u32), nullptr, &err);
    }
    if (err != CL_SUCCESS)
    {
        return fail("Failed to allocate GPU memory", err);
    }

    // Keep each launch short so the display driver doesn't reset: work scales with the outputs every seed generates
    u64 work = 624 * 2 + static_cast<u64>(settings.minFrame) + settings.skip + frames + look;
    u32 perLaunch = static_cast<u32>(std::clamp<u64>((1ULL << 33) / work, 1 << 14, 1 << 22));

    usedGPU = true;
    for (u64 done = 0; done < count && !isCancelled();)
    {
        u32 total = static_cast<u32>(std::min<u64>(perLaunch, count - done));
        u32 first = start + static_cast<u32>(done);

        u32 found = 0;
        for (int attempt = 0; attempt < 2; attempt++)
        {
            u32 zero = 0;
            err = cl->clEnqueueWriteBuffer(queue, countBuffer, CL_TRUE, 0, sizeof(u32), &zero, 0, nullptr, nullptr);

            cl_uint arg = 0;
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(u32), &first);
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(u32), &total);
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(cl_mem), &countBuffer);
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(cl_mem), &out);
            err |= cl->clSetKernelArg(kernel, arg++, sizeof(u32), &capacity);

            size_t global = total;
            err |= cl->clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global, nullptr, 0, nullptr, nullptr);
            err |= cl->clEnqueueReadBuffer(queue, countBuffer, CL_TRUE, 0, sizeof(u32), &found, 0, nullptr, nullptr);
            if (err != CL_SUCCESS || found <= capacity)
            {
                break;
            }

            // More hits than fit, grow the output buffer and run the launch again
            cl->clReleaseMemObject(out);
            capacity = found;
            out = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, static_cast<size_t>(capacity) * 2 * sizeof(u32), nullptr, &err);
        }

        if (err != CL_SUCCESS)
        {
            usedGPU = false;
            return fail("GPU search failed", err);
        }

        if (found > 0)
        {
            std::vector<u32> records(static_cast<size_t>(found) * 2);
            err = cl->clEnqueueReadBuffer(queue, out, CL_TRUE, 0, records.size() * sizeof(u32), records.data(), 0, nullptr, nullptr);
            if (err != CL_SUCCESS)
            {
                usedGPU = false;
                return fail("GPU search failed", err);
            }
            for (u32 i = 0; i < found; i++)
            {
                addResult(records[i * 2], records[i * 2 + 1], start);
            }
        }

        done += total;
        progress = done;
    }

    release();
    return true;
}
