// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#ifndef WR_WGPUDEVICEUTILS_HPP
#define WR_WGPUDEVICEUTILS_HPP

#include <webgpu/webgpu.h>

namespace WGPURenderer {
    WGPUAdapter RequestAdapterSync(WGPUInstance instance, const WGPURequestAdapterOptions* options);
    void InspectAdapter(WGPUAdapter adapter);
    
    WGPUDevice RequestDeviceSync(WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor);
    void InspectDevice(WGPUDevice device);
}

#endif // WR_WGPUDEVICEUTILS_HPP
