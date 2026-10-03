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

#include "GPUIVScanner5.hpp"
#include <Core/Enum/DSType.hpp>
#include <Core/Gen5/Profile5.hpp>
#include <Core/RNG/SHA1.hpp>
#include <Core/Util/DateTime.hpp>
#include <Core/Util/OpenCL.hpp>
#include <algorithm>

using namespace OpenCL;

// Device code. Search settings arrive as -D defines:
//   IVOFF        MT outputs skipped before the first IV at IV advance 0
//   IVMIN/IVMAX  IV advance range (inclusive)
//   ROAMER       roamer IV order
//   IVLO0..5/IVHI0..5  per stat IV bounds, PokeFinder stat order
//   USE_CACHE    look seeds up in a sorted list of IV cache MT seeds instead of running MT
static const char *kernelSource = R"CL(
#define MULT 0x5d588b656c078965UL
#define ADDC 0x269ec3UL

inline uint bswap32(uint x)
{
    return (x >> 24) | ((x >> 8) & 0xff00u) | ((x << 8) & 0xff0000u) | (x << 24);
}

// SHA-1 rounds 9-79 from the host's state after rounds 0-8, then the one BWRNG step PokeFinder applies
inline ulong bootSeed(__constant uint *w, __constant uint *alpha, uint key, uint timeWord)
{
    uint m[16];
    for (int i = 0; i < 16; i++)
        m[i] = w[i];
    m[9] = timeWord;
    m[12] = key;

    uint a = alpha[0], b = alpha[1], c = alpha[2], d = alpha[3], e = alpha[4];
    for (int i = 9; i < 80; i++)
    {
        uint wi;
        if (i < 16)
        {
            wi = m[i];
        }
        else
        {
            wi = rotate(m[(i - 3) & 15] ^ m[(i - 8) & 15] ^ m[(i - 14) & 15] ^ m[(i - 16) & 15], 1u);
            m[i & 15] = wi;
        }
        uint f, k;
        if (i < 20) { f = (b & c) | (~b & d); k = 0x5a827999u; }
        else if (i < 40) { f = b ^ c ^ d; k = 0x6ed9eba1u; }
        else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8f1bbcdcu; }
        else { f = b ^ c ^ d; k = 0xca62c1d6u; }
        uint t = rotate(a, 5u) + f + e + k + wi;
        e = d; d = c; c = rotate(b, 30u); b = a; a = t;
    }

    ulong raw = ((ulong)bswap32(b + 0xefcdab89u) << 32) | (ulong)bswap32(a + 0x67452301u);
    return raw * MULT + ADDC;
}

inline void emit(__global uint *count, __global uint *out, uint cap, uint index, uint aux, ulong seed, uint ivs)
{
    uint idx = atomic_inc(count);
    if (idx < cap)
    {
        out[idx * 5 + 0] = index;
        out[idx * 5 + 1] = aux;
        out[idx * 5 + 2] = (uint)seed;
        out[idx * 5 + 3] = (uint)(seed >> 32);
        out[idx * 5 + 4] = ivs;
    }
}

#ifndef USE_CACHE
#define NK (IVOFF + IVMAX + 6) // MT outputs needed
#define K0 (IVOFF + IVMIN) // first MT output that can be an IV

// One work item per (keypress, second)
__kernel void ivscan(__constant uint *w, __constant uint *alpha, __global const uint *keys, __global const uint *times, uint keyStart,
                     uint total, __global uint *count, __global uint *out, uint cap)
{
    uint gid = get_global_id(0);
    if (gid >= total)
        return;

    uint keyIndex = keyStart + gid / 86400u;
    uint second = gid % 86400u;
    ulong seed = bootSeed(w, alpha, keys[keyIndex], times[second]);

    // MT19937 seeded with the upper 32 bits. Output k only needs state[k], state[k + 1] and state[k + 397] of the
    // initial fill while k + 397 < 624, so walk the fill once and keep the words needed instead of the full table.
    uint x = (uint)(seed >> 32);
    uint lo[NK + 1];
    lo[0] = x;
    for (uint i = 1; i <= NK; i++)
    {
        x = 0x6c078965u * (x ^ (x >> 30)) + i;
        lo[i] = x;
    }
    for (uint i = NK + 1; i < 397 + K0; i++)
        x = 0x6c078965u * (x ^ (x >> 30)) + i;

    uchar iv[NK];
    for (uint k = K0; k < NK; k++)
    {
        x = 0x6c078965u * (x ^ (x >> 30)) + (397 + k);
        uint y = (lo[k] & 0x80000000u) | (lo[k + 1] & 0x7fffffffu);
        uint v = x ^ (y >> 1) ^ ((y & 1u) ? 0x9908b0dfu : 0u);
        v ^= v >> 11;
        v ^= (v << 7) & 0x9d2c5680u;
        v ^= (v << 15) & 0xefc60000u;
        v ^= v >> 18;
        iv[k] = (uchar)(v >> 27);
    }

    for (uint j = IVMIN; j <= IVMAX; j++)
    {
        uint base = IVOFF + j;
        uint hp = iv[base], atk = iv[base + 1], def = iv[base + 2];
#if ROAMER
        uint spd = iv[base + 3], spe = iv[base + 4], spa = iv[base + 5];
#else
        uint spa = iv[base + 3], spd = iv[base + 4], spe = iv[base + 5];
#endif
        if (hp >= IVLO0 && hp <= IVHI0 && atk >= IVLO1 && atk <= IVHI1 && def >= IVLO2 && def <= IVHI2 && spa >= IVLO3 && spa <= IVHI3
            && spd >= IVLO4 && spd <= IVHI4 && spe >= IVLO5 && spe <= IVHI5)
        {
            emit(count, out, cap, keyIndex * 86400u + second, j, seed,
                 hp | (atk << 5) | (def << 10) | (spa << 15) | (spd << 20) | (spe << 25));
        }
    }
}
#else
// One work item per (keypress, second): binary search the seed's MT seed in the sorted IV cache list
__kernel void ivscan(__constant uint *w, __constant uint *alpha, __global const uint *keys, __global const uint *times, uint keyStart,
                     uint total, __global uint *count, __global uint *out, uint cap, __global const uint *mtSeeds, uint mtCount)
{
    uint gid = get_global_id(0);
    if (gid >= total)
        return;

    uint keyIndex = keyStart + gid / 86400u;
    uint second = gid % 86400u;
    ulong seed = bootSeed(w, alpha, keys[keyIndex], times[second]);
    uint s = (uint)(seed >> 32);

    uint lo = 0, hi = mtCount;
    while (lo < hi)
    {
        uint mid = (lo + hi) >> 1;
        if (mtSeeds[mid] < s)
            lo = mid + 1;
        else
            hi = mid;
    }
    for (; lo < mtCount && mtSeeds[lo] == s; lo++)
    {
        emit(count, out, cap, keyIndex * 86400u + second, lo, seed, 0);
    }
}
#endif
)CL";

constexpr u32 recordSize = 5;
constexpr u32 initialCapacity = 1 << 16;

struct GPUIVScanner5::Device
{
    Device(const Profile5 &profile) : sha(profile), dsType(profile.getDSType()), vcount(profile.getVCount())
    {
    }

    ~Device()
    {
        const API *cl = OpenCL::api();
        for (cl_mem mem : { w, alpha, keys, times, count, out, mtSeeds })
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
    }

    SHA1 sha;
    DSType dsType;
    u8 vcount;
    u32 capacity = initialCapacity;
    u32 keysPerLaunch = 1;
    u32 mtCount = 0;
    cl_context context = nullptr;
    cl_command_queue queue = nullptr;
    cl_program program = nullptr;
    cl_kernel kernel = nullptr;
    cl_mem w = nullptr;
    cl_mem alpha = nullptr;
    cl_mem keys = nullptr;
    cl_mem times = nullptr;
    cl_mem count = nullptr;
    cl_mem out = nullptr;
    cl_mem mtSeeds = nullptr;
};

GPUIVScanner5::GPUIVScanner5(const Profile5 &profile, const std::vector<u32> &keypresses, const IVPrefilter5 &prefilter,
                             u32 initialAdvances, u32 maxAdvances) :
    cache(false)
{
    std::string options = "-D IVOFF=" + std::to_string(prefilter.offset) + " -D IVMIN=" + std::to_string(initialAdvances)
        + " -D IVMAX=" + std::to_string(initialAdvances + maxAdvances) + " -D ROAMER=" + std::to_string(prefilter.roamer ? 1 : 0);
    for (int i = 0; i < 6; i++)
    {
        options += " -D IVLO" + std::to_string(i) + "=" + std::to_string(prefilter.ivMin[i]);
        options += " -D IVHI" + std::to_string(i) + "=" + std::to_string(prefilter.ivMax[i]);
    }

    if (!supportsMT(prefilter, initialAdvances, maxAdvances))
    {
        error = "IV advance range too large for the GPU MT search";
        return;
    }

    init(profile, keypresses, options);
    if (device)
    {
        // Keep launches short so the display driver doesn't time out on long IV ranges
        device->keysPerLaunch = std::max<u32>(1, (1 << 25) / 86400);
    }
}

GPUIVScanner5::GPUIVScanner5(const Profile5 &profile, const std::vector<u32> &keypresses,
                             const std::vector<std::pair<u64, std::array<u8, 6>>> &entries) :
    entries(entries), cache(true)
{
    std::ranges::sort(this->entries, [](const auto &left, const auto &right) {
        return static_cast<u32>(left.first) != static_cast<u32>(right.first) ? static_cast<u32>(left.first) < static_cast<u32>(right.first)
                                                                             : left.first < right.first;
    });

    init(profile, keypresses, "-D USE_CACHE");
    if (!device)
    {
        return;
    }
    device->keysPerLaunch = std::max<u32>(1, (1 << 26) / 86400);

    std::vector<u32> seeds;
    seeds.reserve(this->entries.size() + 1);
    for (const auto &entry : this->entries)
    {
        seeds.emplace_back(static_cast<u32>(entry.first));
    }
    device->mtCount = static_cast<u32>(seeds.size());
    seeds.emplace_back(0); // Buffers can't be empty

    const API *cl = OpenCL::api();
    cl_int err;
    device->mtSeeds = cl->clCreateBuffer(device->context, CL_MEM_READ_ONLY, seeds.size() * sizeof(u32), nullptr, &err);
    if (err != CL_SUCCESS
        || cl->clEnqueueWriteBuffer(device->queue, device->mtSeeds, CL_TRUE, 0, seeds.size() * sizeof(u32), seeds.data(), 0, nullptr,
                                    nullptr)
            != CL_SUCCESS)
    {
        error = "Failed to upload the IV cache to the GPU";
        device.reset();
    }
}

GPUIVScanner5::~GPUIVScanner5() = default;

bool GPUIVScanner5::supportsMT(const IVPrefilter5 &prefilter, u32 initialAdvances, u32 maxAdvances)
{
    // The kernel reads MT outputs from the initial state fill, which only works for outputs below 227
    return prefilter.supported && (prefilter.offset + initialAdvances + maxAdvances + 6) <= 227;
}

std::string GPUIVScanner5::getError() const
{
    return error;
}

u32 GPUIVScanner5::getKeypressesPerLaunch() const
{
    return device ? device->keysPerLaunch : 1;
}

bool GPUIVScanner5::isReady() const
{
    return device != nullptr;
}

void GPUIVScanner5::init(const Profile5 &profile, const std::vector<u32> &keypresses, const std::string &options)
{
    const API *cl = OpenCL::api();
    if (!cl)
    {
        error = "No OpenCL GPU found";
        return;
    }

    auto dev = std::make_unique<Device>(profile);
    cl_device_id id = OpenCL::device();
    cl_int err;

    auto fail = [this](const std::string &message, cl_int code) { error = message + " (OpenCL error " + std::to_string(code) + ")"; };

    dev->context = cl->clCreateContext(nullptr, 1, &id, nullptr, nullptr, &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL context", err);
    }

    dev->queue = cl->clCreateCommandQueue(dev->context, id, 0, &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL queue", err);
    }

    dev->program = cl->clCreateProgramWithSource(dev->context, 1, &kernelSource, nullptr, &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL program", err);
    }

    err = cl->clBuildProgram(dev->program, 1, &id, options.c_str(), nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        size_t size = 0;
        cl->clGetProgramBuildInfo(dev->program, id, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size);
        std::string log(size, '\0');
        cl->clGetProgramBuildInfo(dev->program, id, CL_PROGRAM_BUILD_LOG, size, log.data(), nullptr);
        return fail("Failed to build OpenCL program: " + log, err);
    }

    dev->kernel = cl->clCreateKernel(dev->program, "ivscan", &err);
    if (err != CL_SUCCESS)
    {
        return fail("Failed to create OpenCL kernel", err);
    }

    std::vector<u32> times(86400);
    for (u32 time = 0; time < 86400; time++)
    {
        dev->sha.setTime(time, dev->dsType);
        times[time] = dev->sha.getMessage()[9];
    }

    auto buffer = [&](cl_mem &mem, cl_mem_flags flags, size_t size, const void *data) {
        mem = cl->clCreateBuffer(dev->context, flags, size, nullptr, &err);
        if (err == CL_SUCCESS && data)
        {
            err = cl->clEnqueueWriteBuffer(dev->queue, mem, CL_TRUE, 0, size, data, 0, nullptr, nullptr);
        }
        return err == CL_SUCCESS;
    };

    std::vector<u32> keys = keypresses;
    keys.emplace_back(0); // Buffers can't be empty
    if (!buffer(dev->w, CL_MEM_READ_ONLY, 16 * sizeof(u32), nullptr) || !buffer(dev->alpha, CL_MEM_READ_ONLY, 5 * sizeof(u32), nullptr)
        || !buffer(dev->keys, CL_MEM_READ_ONLY, keys.size() * sizeof(u32), keys.data())
        || !buffer(dev->times, CL_MEM_READ_ONLY, times.size() * sizeof(u32), times.data())
        || !buffer(dev->count, CL_MEM_READ_WRITE, sizeof(u32), nullptr)
        || !buffer(dev->out, CL_MEM_READ_WRITE, static_cast<size_t>(dev->capacity) * recordSize * sizeof(u32), nullptr))
    {
        return fail("Failed to allocate GPU memory", err);
    }

    device = std::move(dev);
}

bool GPUIVScanner5::scan(const Date &date, u16 timer0, u32 keyStart, u32 keyCount, std::vector<Candidate> &candidates)
{
    if (!device)
    {
        return false;
    }

    const API *cl = OpenCL::api();
    Device &dev = *device;

    dev.sha.setDate(date);
    dev.sha.setTimer0(timer0, dev.vcount);
    auto alpha = dev.sha.precompute();
    auto message = dev.sha.getMessage();

    u32 total = keyCount * 86400;
    cl_int err = cl->clEnqueueWriteBuffer(dev.queue, dev.w, CL_TRUE, 0, sizeof(message), message.data(), 0, nullptr, nullptr);
    err |= cl->clEnqueueWriteBuffer(dev.queue, dev.alpha, CL_TRUE, 0, sizeof(alpha), alpha.data(), 0, nullptr, nullptr);

    u32 found = 0;
    for (int attempt = 0; attempt < 2 && err == CL_SUCCESS; attempt++)
    {
        u32 zero = 0;
        err |= cl->clEnqueueWriteBuffer(dev.queue, dev.count, CL_TRUE, 0, sizeof(u32), &zero, 0, nullptr, nullptr);

        cl_uint arg = 0;
        err |= cl->clSetKernelArg(dev.kernel, arg++, sizeof(cl_mem), &dev.w);
        err |= cl->clSetKernelArg(dev.kernel, arg++, sizeof(cl_mem), &dev.alpha);
        err |= cl->clSetKernelArg(dev.kernel, arg++, sizeof(cl_mem), &dev.keys);
        err |= cl->clSetKernelArg(dev.kernel, arg++, sizeof(cl_mem), &dev.times);
        err |= cl->clSetKernelArg(dev.kernel, arg++, sizeof(u32), &keyStart);
        err |= cl->clSetKernelArg(dev.kernel, arg++, sizeof(u32), &total);
        err |= cl->clSetKernelArg(dev.kernel, arg++, sizeof(cl_mem), &dev.count);
        err |= cl->clSetKernelArg(dev.kernel, arg++, sizeof(cl_mem), &dev.out);
        err |= cl->clSetKernelArg(dev.kernel, arg++, sizeof(u32), &dev.capacity);
        if (cache)
        {
            err |= cl->clSetKernelArg(dev.kernel, arg++, sizeof(cl_mem), &dev.mtSeeds);
            err |= cl->clSetKernelArg(dev.kernel, arg++, sizeof(u32), &dev.mtCount);
        }

        size_t global = total;
        err |= cl->clEnqueueNDRangeKernel(dev.queue, dev.kernel, 1, nullptr, &global, nullptr, 0, nullptr, nullptr);
        err |= cl->clEnqueueReadBuffer(dev.queue, dev.count, CL_TRUE, 0, sizeof(u32), &found, 0, nullptr, nullptr);
        if (err != CL_SUCCESS || found <= dev.capacity)
        {
            break;
        }

        // More seeds passed than fit, grow the output buffer and run the launch again
        cl->clReleaseMemObject(dev.out);
        dev.capacity = found;
        dev.out = cl->clCreateBuffer(dev.context, CL_MEM_READ_WRITE, static_cast<size_t>(dev.capacity) * recordSize * sizeof(u32), nullptr,
                                     &err);
    }

    if (err != CL_SUCCESS)
    {
        error = "GPU search failed (OpenCL error " + std::to_string(err) + ")";
        return false;
    }

    if (found == 0)
    {
        return true;
    }

    std::vector<u32> records(static_cast<size_t>(found) * recordSize);
    err = cl->clEnqueueReadBuffer(dev.queue, dev.out, CL_TRUE, 0, records.size() * sizeof(u32), records.data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        error = "GPU search failed (OpenCL error " + std::to_string(err) + ")";
        return false;
    }

    candidates.reserve(candidates.size() + found);
    for (u32 i = 0; i < found; i++)
    {
        const u32 *record = &records[static_cast<size_t>(i) * recordSize];
        Candidate candidate;
        candidate.index = record[0];
        candidate.seed = (static_cast<u64>(record[3]) << 32) | record[2];
        if (cache)
        {
            const auto &entry = entries[record[1]];
            candidate.ivAdvance = static_cast<u32>(entry.first >> 32);
            candidate.ivs = entry.second;
        }
        else
        {
            candidate.ivAdvance = record[1];
            for (int j = 0; j < 6; j++)
            {
                candidate.ivs[j] = (record[4] >> (5 * j)) & 31;
            }
        }
        candidates.emplace_back(candidate);
    }

    return true;
}
