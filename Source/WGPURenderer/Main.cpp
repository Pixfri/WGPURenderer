// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <WGPURenderer/WGPUDeviceUtils.hpp>

#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h>

#include <iostream>

int main() {
    WGPUInstanceDescriptor desc;
    desc.nextInChain = nullptr;

    const WGPUInstance instance = wgpuCreateInstance(&desc);

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
    
    const WGPUDevice device = WGPURenderer::RequestDeviceSync(adapter, &deviceDesc);

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

    const WGPUQueue queue = wgpuDeviceGetQueue(device);

    auto onQueueWorkDone = [](const WGPUQueueWorkDoneStatus status, void* /* pUserData */) {
        std::cout << "Queued work finished with status: " << status << '\n';
    };
    wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, nullptr /* pUserData */);

    WGPUCommandEncoderDescriptor encoderDesc;
    encoderDesc.nextInChain = nullptr;
#ifdef WR_DEBUG
    encoderDesc.label = "Main command encoder";
#else
    encoderDesc.label = nullptr;
#endif
    const WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

    wgpuCommandEncoderInsertDebugMarker(encoder, "Do one thing");
    wgpuCommandEncoderInsertDebugMarker(encoder, "Do another thing");

    WGPUCommandBufferDescriptor cmdBufferDesc{};
    cmdBufferDesc.nextInChain = nullptr;
    encoderDesc.nextInChain = nullptr;
#ifdef WR_DEBUG
    encoderDesc.label = "Main command buffer";
#else
    encoderDesc.label = nullptr;
#endif
    WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);
    wgpuCommandEncoderRelease(encoder);

    // Finally submit the command queue
    std::cout << "Submitting commands...\n";
    wgpuQueueSubmit(queue, 1, &cmdBuffer);
    wgpuCommandBufferRelease(cmdBuffer);
    std::cout << "Commands submitted.\n";

    for (int i = 0; i < 5; ++i) {
        wgpuDevicePoll(device, false, nullptr);
    }
    
    // Release the adapter after we got the device, we don't need it anymore.
    wgpuAdapterRelease(adapter);

    wgpuQueueRelease(queue);
    
    wgpuDeviceRelease(device);

    return EXIT_SUCCESS;
}
