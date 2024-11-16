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

    std::cout << "Got WebGPU adapter: " << adapter << '\n';

    WGPURenderer::InspectAdapter(adapter);

    // Release the instance since we don't need it after we've got the adapter.
    wgpuInstanceRelease(instance);

    wgpuAdapterRelease(adapter);

    return EXIT_SUCCESS;
}
