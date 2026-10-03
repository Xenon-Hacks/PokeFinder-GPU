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

#include "OpenCL.hpp"
#include <cstdlib>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace OpenCL
{
    namespace
    {
        struct Runtime
        {
            API api {};
            cl_device_id device = nullptr;
            std::string name;
            bool loaded = false;
        };

        void *openLibrary()
        {
#if defined(_WIN32)
            return reinterpret_cast<void *>(LoadLibraryA("OpenCL.dll"));
#elif defined(__APPLE__)
            return dlopen("/System/Library/Frameworks/OpenCL.framework/OpenCL", RTLD_NOW | RTLD_LOCAL);
#else
            void *lib = dlopen("libOpenCL.so.1", RTLD_NOW | RTLD_LOCAL);
            return lib ? lib : dlopen("libOpenCL.so", RTLD_NOW | RTLD_LOCAL);
#endif
        }

        void *getSymbol(void *lib, const char *name)
        {
#ifdef _WIN32
            return reinterpret_cast<void *>(GetProcAddress(reinterpret_cast<HMODULE>(lib), name));
#else
            return dlsym(lib, name);
#endif
        }

        template <class T>
        bool load(void *lib, T &fn, const char *name)
        {
            fn = reinterpret_cast<T>(getSymbol(lib, name));
            return fn != nullptr;
        }

        Runtime init()
        {
            Runtime runtime;

            void *lib = openLibrary();
            if (!lib)
            {
                return runtime;
            }

            API &a = runtime.api;
            bool ok = load(lib, a.clGetPlatformIDs, "clGetPlatformIDs") && load(lib, a.clGetDeviceIDs, "clGetDeviceIDs")
                && load(lib, a.clGetDeviceInfo, "clGetDeviceInfo") && load(lib, a.clCreateContext, "clCreateContext")
                && load(lib, a.clCreateCommandQueue, "clCreateCommandQueue")
                && load(lib, a.clCreateProgramWithSource, "clCreateProgramWithSource") && load(lib, a.clBuildProgram, "clBuildProgram")
                && load(lib, a.clGetProgramBuildInfo, "clGetProgramBuildInfo") && load(lib, a.clCreateKernel, "clCreateKernel")
                && load(lib, a.clCreateBuffer, "clCreateBuffer") && load(lib, a.clSetKernelArg, "clSetKernelArg")
                && load(lib, a.clEnqueueNDRangeKernel, "clEnqueueNDRangeKernel") && load(lib, a.clEnqueueReadBuffer, "clEnqueueReadBuffer")
                && load(lib, a.clEnqueueWriteBuffer, "clEnqueueWriteBuffer") && load(lib, a.clFinish, "clFinish")
                && load(lib, a.clReleaseMemObject, "clReleaseMemObject") && load(lib, a.clReleaseKernel, "clReleaseKernel")
                && load(lib, a.clReleaseProgram, "clReleaseProgram") && load(lib, a.clReleaseCommandQueue, "clReleaseCommandQueue")
                && load(lib, a.clReleaseContext, "clReleaseContext");
            if (!ok)
            {
                return runtime;
            }

            cl_uint platformCount = 0;
            if (a.clGetPlatformIDs(0, nullptr, &platformCount) != CL_SUCCESS || platformCount == 0)
            {
                return runtime;
            }
            std::vector<cl_platform_id> platforms(platformCount);
            a.clGetPlatformIDs(platformCount, platforms.data(), nullptr);

            bool allowCPU = std::getenv("POKEFINDER_OPENCL_CPU") != nullptr;
            cl_device_type wanted = allowCPU ? (CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU) : CL_DEVICE_TYPE_GPU;

            // Pick the device with the most compute units, preferring GPUs over CPUs
            cl_uint bestScore = 0;
            for (auto platform : platforms)
            {
                cl_uint deviceCount = 0;
                if (a.clGetDeviceIDs(platform, wanted, 0, nullptr, &deviceCount) != CL_SUCCESS || deviceCount == 0)
                {
                    continue;
                }
                std::vector<cl_device_id> devices(deviceCount);
                a.clGetDeviceIDs(platform, wanted, deviceCount, devices.data(), nullptr);

                for (auto device : devices)
                {
                    cl_device_type type = 0;
                    cl_uint units = 0;
                    a.clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, nullptr);
                    a.clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(units), &units, nullptr);
                    cl_uint score = units + ((type & CL_DEVICE_TYPE_GPU) ? 0x10000 : 1);
                    if (score > bestScore)
                    {
                        bestScore = score;
                        runtime.device = device;
                    }
                }
            }

            if (runtime.device)
            {
                char name[256] = {};
                a.clGetDeviceInfo(runtime.device, CL_DEVICE_NAME, sizeof(name) - 1, name, nullptr);
                runtime.name = name;
                runtime.loaded = true;
            }
            return runtime;
        }

        const Runtime &runtime()
        {
            static const Runtime instance = init();
            return instance;
        }
    }

    const API *api()
    {
        return runtime().loaded ? &runtime().api : nullptr;
    }

    cl_device_id device()
    {
        return runtime().device;
    }

    std::string deviceName()
    {
        return runtime().name;
    }

    bool isAvailable()
    {
        return runtime().loaded;
    }
}
