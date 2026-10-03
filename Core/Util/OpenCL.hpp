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

#ifndef OPENCL_HPP
#define OPENCL_HPP

#include <Core/Global.hpp>
#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @brief Minimal OpenCL 1.2 bindings that are loaded at runtime, so PokeFinder builds without an OpenCL SDK
 * and still runs on machines without an OpenCL driver.
 */
namespace OpenCL
{
    using cl_int = s32;
    using cl_uint = u32;
    using cl_ulong = u64;
    using cl_bitfield = u64;
    using cl_bool = u32;
    using cl_device_type = cl_bitfield;
    using cl_mem_flags = cl_bitfield;
    using cl_command_queue_properties = cl_bitfield;
    using cl_platform_info = cl_uint;
    using cl_device_info = cl_uint;
    using cl_program_build_info = cl_uint;
    using cl_context_properties = intptr_t;

    using cl_platform_id = struct _cl_platform_id *;
    using cl_device_id = struct _cl_device_id *;
    using cl_context = struct _cl_context *;
    using cl_command_queue = struct _cl_command_queue *;
    using cl_mem = struct _cl_mem *;
    using cl_program = struct _cl_program *;
    using cl_kernel = struct _cl_kernel *;
    using cl_event = struct _cl_event *;

    constexpr cl_int CL_SUCCESS = 0;
    constexpr cl_bool CL_TRUE = 1;
    constexpr cl_device_type CL_DEVICE_TYPE_CPU = 1 << 1;
    constexpr cl_device_type CL_DEVICE_TYPE_GPU = 1 << 2;
    constexpr cl_device_type CL_DEVICE_TYPE_ALL = 0xffffffff;
    constexpr cl_device_info CL_DEVICE_TYPE = 0x1000;
    constexpr cl_device_info CL_DEVICE_MAX_COMPUTE_UNITS = 0x1002;
    constexpr cl_device_info CL_DEVICE_NAME = 0x102b;
    constexpr cl_program_build_info CL_PROGRAM_BUILD_LOG = 0x1183;
    constexpr cl_mem_flags CL_MEM_READ_WRITE = 1 << 0;
    constexpr cl_mem_flags CL_MEM_READ_ONLY = 1 << 2;

    struct API
    {
        cl_int (*clGetPlatformIDs)(cl_uint, cl_platform_id *, cl_uint *);
        cl_int (*clGetDeviceIDs)(cl_platform_id, cl_device_type, cl_uint, cl_device_id *, cl_uint *);
        cl_int (*clGetDeviceInfo)(cl_device_id, cl_device_info, size_t, void *, size_t *);
        cl_context (*clCreateContext)(const cl_context_properties *, cl_uint, const cl_device_id *,
                                      void (*)(const char *, const void *, size_t, void *), void *, cl_int *);
        cl_command_queue (*clCreateCommandQueue)(cl_context, cl_device_id, cl_command_queue_properties, cl_int *);
        cl_program (*clCreateProgramWithSource)(cl_context, cl_uint, const char **, const size_t *, cl_int *);
        cl_int (*clBuildProgram)(cl_program, cl_uint, const cl_device_id *, const char *, void (*)(cl_program, void *), void *);
        cl_int (*clGetProgramBuildInfo)(cl_program, cl_device_id, cl_program_build_info, size_t, void *, size_t *);
        cl_kernel (*clCreateKernel)(cl_program, const char *, cl_int *);
        cl_mem (*clCreateBuffer)(cl_context, cl_mem_flags, size_t, void *, cl_int *);
        cl_int (*clSetKernelArg)(cl_kernel, cl_uint, size_t, const void *);
        cl_int (*clEnqueueNDRangeKernel)(cl_command_queue, cl_kernel, cl_uint, const size_t *, const size_t *, const size_t *, cl_uint,
                                         const cl_event *, cl_event *);
        cl_int (*clEnqueueReadBuffer)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, void *, cl_uint, const cl_event *, cl_event *);
        cl_int (*clEnqueueWriteBuffer)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, const void *, cl_uint, const cl_event *,
                                       cl_event *);
        cl_int (*clFinish)(cl_command_queue);
        cl_int (*clReleaseMemObject)(cl_mem);
        cl_int (*clReleaseKernel)(cl_kernel);
        cl_int (*clReleaseProgram)(cl_program);
        cl_int (*clReleaseCommandQueue)(cl_command_queue);
        cl_int (*clReleaseContext)(cl_context);
    };

    /**
     * @brief Loads the OpenCL runtime (once) and picks a device
     *
     * GPU devices are preferred. CPU devices are only used when the environment variable POKEFINDER_OPENCL_CPU is set,
     * since PokeFinder's own SIMD CPU searcher is normally faster than an OpenCL CPU driver.
     *
     * @return API function table, or nullptr if no usable device was found
     */
    const API *api();

    /**
     * @brief Returns the device selected by \ref api()
     *
     * @return Selected device
     */
    cl_device_id device();

    /**
     * @brief Returns the name of the device selected by \ref api(), or an empty string if none
     *
     * @return Device name
     */
    std::string deviceName();

    /**
     * @brief Checks whether a usable OpenCL device exists
     *
     * @return true A device is available
     * @return false No device is available
     */
    bool isAvailable();
}

#endif // OPENCL_HPP
