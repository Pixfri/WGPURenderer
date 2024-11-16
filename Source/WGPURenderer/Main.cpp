// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <WGPURenderer/WGPUDeviceUtils.hpp>

#include <webgpu/webgpu.h>

#include <iostream>

int main() {
    WGPUInstanceDescriptor desc;
    desc.nextInChain = nullptr;

    WGPUInstance instance = wgpuCreateInstance(&desc);

    if (!instance) {
        std::cerr << "Couldn't initialize WebGPU!\n";
        return EXIT_FAILURE;
    }

    std::cout << "WebGPU instance: 0x" << instance << '\n';

    std::cout << "Requesting WebGPU adapter...\n";

    WGPURequestAdapterOptions adapterOptions{};
    adapterOptions.nextInChain = nullptr;
    const WGPUAdapter adapter = WGPURenderer::RequestAdapterSync(instance, &adapterOptions);

    std::cout << "Got WebGPU adapter: 0x" << adapter << '\n';

    WGPURenderer::InspectAdapter(adapter);

    // Release the instance since we don't need it after we've got the adapter.
    wgpuInstanceRelease(instance);

    std::cout << "Requesting WebGPU device...\n";

    WGPUDeviceDescriptor deviceDesc{};
    deviceDesc.nextInChain = nullptr;
    
#ifdef WR_DEBUG
    deviceDesc.label = "WebGPU Device";
#else
    deviceDesc.label = nullptr;
#endif
    
    deviceDesc.requiredFeatureCount = 0;
    deviceDesc.requiredFeatures = nullptr;
    deviceDesc.requiredLimits = nullptr;
    deviceDesc.defaultQueue.nextInChain = nullptr;
    
#ifdef WR_DEBUG
    deviceDesc.defaultQueue.label = "WebGPU Default Queue";
#else
    deviceDesc.defaultQueue.label = nullptr;
#endif

    deviceDesc.deviceLostCallback = [](const WGPUDeviceLostReason reason, const char* message, void* /*pUserData*/) {
        std::cout << "Device lost: reason: " << reason;
        if (message) {
            std::cout << " (" << message << ')';
        }

        std::cout << '\n';
    };
    
    WGPUDevice device = WGPURenderer::RequestDeviceSync(adapter, &deviceDesc);

    std::cout << "Got WebGPU device: 0x" << device << '\n';

    auto onDeviceError = [](const WGPUErrorType type, const char* message, void* /*pUserData*/) {
        std::cerr << "Uncaptured device error: type: " << type;
        if (message) {
            std::cerr << " (" << message << ')';
        }

        std::cout << '\n';
    };
    wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr);

    WGPURenderer::InspectDevice(device);

    // Release the adapter after we got the device, we don't need it anymore.
    wgpuAdapterRelease(adapter);
    
    wgpuDeviceRelease(device);

    return EXIT_SUCCESS;
}
