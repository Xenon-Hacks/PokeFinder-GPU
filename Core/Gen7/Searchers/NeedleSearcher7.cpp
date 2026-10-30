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

#include "NeedleSearcher7.hpp"
#include <Core/RNG/SFMT.hpp>
#include <Core/Util/OpenCL.hpp>
#include <algorithm>
#include <thread>

using namespace OpenCL;

// Device code. The search settings arrive as -D defines:
//   TITLE   advance of the first needle (417 / 477); the needle k is the u64 at advance TITLE + k = words 2*(TITLE+k), +1
//   NCOUNT  number of needles, N0..N15 their values
// SFMT-19937 as the games use it: init_gen_rand with the period certification, then whole twists; one advance = two words.
static const char *kernelSource = R"CL(
#define N32 624
#define N128 156
#define POS1 122
#define SL1 18
#define SR1 11

inline void recursion(__private uint *r, __private const uint *a, __private const uint *b, __private const uint *c,
                      __private const uint *d)
{
    // x = a << 8 bits (128-bit), y = c >> 8 bits (128-bit), little endian word order
    ulong ah = ((ulong)a[3] << 32) | a[2];
    ulong al = ((ulong)a[1] << 32) | a[0];
    ulong xh = (ah << 8) | (al >> 56);
    ulong xl = al << 8;
    ulong ch = ((ulong)c[3] << 32) | c[2];
    ulong cl = ((ulong)c[1] << 32) | c[0];
    ulong yh = ch >> 8;
    ulong yl = (cl >> 8) | (ch << 56);

    r[0] = a[0] ^ (uint)xl ^ ((b[0] >> SR1) & 0xdfffffefu) ^ (uint)yl ^ (d[0] << SL1);
    r[1] = a[1] ^ (uint)(xl >> 32) ^ ((b[1] >> SR1) & 0xddfecb7fu) ^ (uint)(yl >> 32) ^ (d[1] << SL1);
    r[2] = a[2] ^ (uint)xh ^ ((b[2] >> SR1) & 0xbffaffffu) ^ (uint)(yh) ^ (d[2] << SL1);
    r[3] = a[3] ^ (uint)(xh >> 32) ^ ((b[3] >> SR1) & 0xbffffff6u) ^ (uint)(yh >> 32) ^ (d[3] << SL1);
}

inline void twist(__private uint *s)
{
    uint r1[4] = { s[(N128 - 2) * 4], s[(N128 - 2) * 4 + 1], s[(N128 - 2) * 4 + 2], s[(N128 - 2) * 4 + 3] };
    uint r2[4] = { s[(N128 - 1) * 4], s[(N128 - 1) * 4 + 1], s[(N128 - 1) * 4 + 2], s[(N128 - 1) * 4 + 3] };
    for (int i = 0; i < N128; i++)
    {
        uint r[4];
        int b = (i + POS1) % N128;
        recursion(r, s + i * 4, s + b * 4, r1, r2);
        s[i * 4] = r[0];
        s[i * 4 + 1] = r[1];
        s[i * 4 + 2] = r[2];
        s[i * 4 + 3] = r[3];
        r1[0] = r2[0]; r1[1] = r2[1]; r1[2] = r2[2]; r1[3] = r2[3];
        r2[0] = r[0]; r2[1] = r[1]; r2[2] = r[2]; r2[3] = r[3];
    }
}

__constant uchar needles[16] = { N0, N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12, N13, N14, N15 };

__kernel void needlescan(uint start, uint total, __global uint *count, __global uint *out, uint cap)
{
    uint gid = get_global_id(0);
    if (gid >= total)
        return;
    uint seed = start + gid;

    uint s[N32];
    s[0] = seed;
    uint x = seed;
    for (uint i = 1; i < N32; i++)
    {
        x = 1812433253u * (x ^ (x >> 30)) + i;
        s[i] = x;
    }
    // Period certification: parity 00000001 00000000 00000000 13C9E684
    uint inner = (s[0] & 1u) ^ (s[3] & 0x13c9e684u);
    inner ^= inner >> 16;
    inner ^= inner >> 8;
    inner ^= inner >> 4;
    inner ^= inner >> 2;
    inner ^= inner >> 1;
    s[0] ^= (~inner) & 1u;

    // Words 0..623 come from the first twist, 624..1247 from the second; the needles sit past advance 312
    twist(s);
    twist(s);

    for (uint k = 0; k < NCOUNT; k++)
    {
        uint w = 2u * (TITLE + k) - N32;
        ulong v = ((ulong)s[w + 1] << 32) | s[w];
        if ((uint)(v % 17ul) != needles[k])
            return;
    }

    uint idx = atomic_inc(count);
    if (idx < cap)
        out[idx] = seed;
}
)CL";

NeedleSearcher7::NeedleSearcher7(u32 titleAdvance, const std::vector<u8> &needles) : needles(needles), titleAdvance(titleAdvance), usedGPU(false)
{
}

std::vector<u8> NeedleSearcher7::needlesFor(u32 seed, u32 titleAdvance, u32 count)
{
    SFMT sfmt(seed, titleAdvance);
    std::vector<u8> result(count);
    for (u8 &needle : result)
    {
        needle = static_cast<u8>(sfmt.next() % 17);
    }
    return result;
}

std::string NeedleSearcher7::getError() const
{
    return error;
}

bool NeedleSearcher7::getUsedGPU() const
{
    return usedGPU;
}

void NeedleSearcher7::startSearch(int threads, bool useGPU, u32 start, u64 count)
{
    setMaxProgress(std::max<u64>(count, 1));
    activeThreads = 1;

    threadContainer.emplace_back([=, this] {
        if (!useGPU || !searchGPU(start, count))
        {
            usedGPU = false;
            progress = 0;
            {
                std::lock_guard<std::mutex> guard(mutex);
                results.clear();
            }
            searchCPU(start, count, threads);
        }
        activeThreads--;
    });
}

void NeedleSearcher7::addResult(u32 seed)
{
    std::lock_guard<std::mutex> guard(mutex);
    results.emplace_back(seed);
}

void NeedleSearcher7::searchCPU(u32 start, u64 count, int threads)
{
    constexpr u64 chunk = 0x400;
    std::atomic<u64> next(0);

    auto worker = [&] {
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
                SFMT sfmt(seed, titleAdvance);
                bool match = true;
                for (u8 needle : needles)
                {
                    if (static_cast<u8>(sfmt.next() % 17) != needle)
                    {
                        match = false;
                        break;
                    }
                }
                if (match)
                {
                    addResult(seed);
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

bool NeedleSearcher7::searchGPU(u32 start, u64 count)
{
    const API *cl = OpenCL::api();
    if (!cl)
    {
        error = "No OpenCL GPU found";
        return false;
    }
    if (needles.empty() || needles.size() > 16 || 2 * (titleAdvance + needles.size()) > 1248)
    {
        error = "Between 1 and 16 needles are supported";
        return false;
    }

    std::string options = "-D TITLE=" + std::to_string(titleAdvance) + " -D NCOUNT=" + std::to_string(needles.size());
    for (size_t i = 0; i < 16; i++)
    {
        options += " -D N" + std::to_string(i) + "=" + std::to_string(i < needles.size() ? needles[i] : 0);
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

    kernel = cl->clCreateKernel(program, "needlescan", &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL kernel", err);
    }

    u32 capacity = 1 << 16;
    countBuffer = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(u32), nullptr, &err);
    if (err == CL_SUCCESS)
    {
        out = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, static_cast<size_t>(capacity) * sizeof(u32), nullptr, &err);
    }
    if (err != CL_SUCCESS)
    {
        return fail("Failed to allocate GPU memory", err);
    }

    // Each seed costs an SFMT init and two twists; keep a launch to about a quarter second so the display driver stays happy
    constexpr u32 perLaunch = 1 << 22;

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

            cl->clReleaseMemObject(out);
            capacity = found;
            out = cl->clCreateBuffer(context, CL_MEM_READ_WRITE, static_cast<size_t>(capacity) * sizeof(u32), nullptr, &err);
        }

        if (err != CL_SUCCESS)
        {
            usedGPU = false;
            return fail("GPU search failed", err);
        }

        if (found > 0)
        {
            std::vector<u32> seeds(found);
            err = cl->clEnqueueReadBuffer(queue, out, CL_TRUE, 0, seeds.size() * sizeof(u32), seeds.data(), 0, nullptr, nullptr);
            if (err != CL_SUCCESS)
            {
                usedGPU = false;
                return fail("GPU search failed", err);
            }
            for (u32 seed : seeds)
            {
                addResult(seed);
            }
        }

        done += total;
        progress = done;
    }

    release();
    return true;
}
