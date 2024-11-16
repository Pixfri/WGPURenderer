// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <WGPURenderer/WGPUDeviceUtils.hpp>

#include <cassert>
#include <iostream>
#include <vector>

namespace WGPURenderer {
    /**
     * Utility function to get a WebGPU adapter, so that
     *     WGPUAdapter adapter = requestAdapterSync(options);
     * is roughly equivalent to
     *     const adapter = await navigator.gpu.requestAdapter(options);
     */
    WGPUAdapter RequestAdapterSync(const WGPUInstance instance, const WGPURequestAdapterOptions* options) {
        // A simple structure holding the local information shared with the
        // onAdapterRequestEnded callback.
        struct UserData {
            WGPUAdapter Adapter = nullptr;
            bool RequestEnded = false;
        };
        UserData userData;

        // Callback called by wgpuInstanceRequestAdapter when the request returns
        // This is a C++ lambda function, but could be any function defined in the
        // global scope. It must be non-capturing (the brackets [] are empty) so
        // that it behaves like a regular C function pointer, which is what
        // wgpuInstanceRequestAdapter expects (WebGPU being a C API). The workaround
        // is to convey what we want to capture through the pUserData pointer,
        // provided as the last argument of wgpuInstanceRequestAdapter and received
        // by the callback as its last argument.
        auto onAdapterRequestEnded = [](const WGPURequestAdapterStatus status,
                                        const WGPUAdapter adapter,
                                        const char* message,
                                        void* pUserData) {
            UserData& data = *static_cast<UserData*>(pUserData);
            if (status == WGPURequestAdapterStatus_Success) {
                data.Adapter = adapter;
            } else {
                std::cerr << "Couldn't get WebGPU adapter: " << message << '\n';
            }
            data.RequestEnded = true;
        };

        // Call the WebGPU request adapter procedure
        wgpuInstanceRequestAdapter(instance, options, onAdapterRequestEnded, &userData);

        assert(userData.RequestEnded);

        return userData.Adapter;
    }

    void InspectAdapter(const WGPUAdapter adapter) {
        WGPUSupportedLimits supportedLimits{};
        supportedLimits.nextInChain = nullptr;

        if (wgpuAdapterGetLimits(adapter, &supportedLimits)) {
            // @formatter:off
            // clang-format off
            std::cout << "Adapter limits: \n";
            std::cout << "\t- maxTextureDimension1D:                     " << supportedLimits.limits.maxTextureDimension1D << '\n';
            std::cout << "\t- maxTextureDimension2D:                     " << supportedLimits.limits.maxTextureDimension2D << '\n';
            std::cout << "\t- maxTextureDimension3D:                     " << supportedLimits.limits.maxTextureDimension3D << '\n';
            std::cout << "\t- maxTextureArrayLayers:                     " << supportedLimits.limits.maxTextureArrayLayers << '\n';
            std::cout << "\t- maxBindGroups:                             " << supportedLimits.limits.maxBindGroups << '\n';
            std::cout << "\t- maxBindGroupsPlusVertexBuffers:            " << supportedLimits.limits.maxBindGroupsPlusVertexBuffers << '\n';
            std::cout << "\t- maxBindingsPerBindGroup:                   " << supportedLimits.limits.maxBindingsPerBindGroup << '\n';
            std::cout << "\t- maxDynamicUniformBuffersPerPipelineLayout: " << supportedLimits.limits.maxDynamicUniformBuffersPerPipelineLayout << '\n';
            std::cout << "\t- maxDynamicStorageBuffersPerPipelineLayout: " << supportedLimits.limits.maxDynamicStorageBuffersPerPipelineLayout << '\n';
            std::cout << "\t- maxSampledTexturesPerShaderStage:          " << supportedLimits.limits.maxSampledTexturesPerShaderStage << '\n';
            std::cout << "\t- maxSamplersPerShaderStage:                 " << supportedLimits.limits.maxSamplersPerShaderStage << '\n';
            std::cout << "\t- maxStorageBuffersPerShaderStage:           " << supportedLimits.limits.maxStorageBuffersPerShaderStage << '\n';
            std::cout << "\t- maxStorageTexturesPerShaderStage:          " << supportedLimits.limits.maxStorageTexturesPerShaderStage << '\n';
            std::cout << "\t- maxUniformBuffersPerShaderStage:           " << supportedLimits.limits.maxUniformBuffersPerShaderStage << '\n';
            std::cout << "\t- maxUniformBufferBindingSize:               " << supportedLimits.limits.maxUniformBufferBindingSize << '\n';
            std::cout << "\t- maxStorageBufferBindingSize:               " << supportedLimits.limits.maxStorageBufferBindingSize << '\n';
            std::cout << "\t- minUniformBufferOffsetAlignment:           " << supportedLimits.limits.minUniformBufferOffsetAlignment << '\n';
            std::cout << "\t- minStorageBufferOffsetAlignment:           " << supportedLimits.limits.minStorageBufferOffsetAlignment << '\n';
            std::cout << "\t- maxVertexBuffers:                          " << supportedLimits.limits.maxVertexBuffers << '\n';
            std::cout << "\t- maxBufferSize:                             " << supportedLimits.limits.maxBufferSize << '\n';
            std::cout << "\t- maxVertexAttributes:                       " << supportedLimits.limits.maxVertexAttributes << '\n';
            std::cout << "\t- maxVertexBufferArrayStride:                " << supportedLimits.limits.maxVertexBufferArrayStride << '\n';
            std::cout << "\t- maxInterStageShaderComponents:             " << supportedLimits.limits.maxInterStageShaderComponents << '\n';
            std::cout << "\t- maxInterStageShaderVariables:              " << supportedLimits.limits.maxInterStageShaderVariables << '\n';
            std::cout << "\t- maxColorAttachments:                       " << supportedLimits.limits.maxColorAttachments << '\n';
            std::cout << "\t- maxColorAttachmentBytesPerSample:          " << supportedLimits.limits.maxColorAttachmentBytesPerSample << '\n';
            std::cout << "\t- maxComputeWorkgroupStorageSize:            " << supportedLimits.limits.maxComputeWorkgroupStorageSize << '\n';
            std::cout << "\t- maxComputeInvocationsPerWorkgroup:         " << supportedLimits.limits.maxComputeInvocationsPerWorkgroup << '\n';
            std::cout << "\t- maxComputeWorkgroupSizeX:                  " << supportedLimits.limits.maxComputeWorkgroupSizeX << '\n';
            std::cout << "\t- maxComputeWorkgroupSizeY:                  " << supportedLimits.limits.maxComputeWorkgroupSizeY << '\n';
            std::cout << "\t- maxComputeWorkgroupSizeZ:                  " << supportedLimits.limits.maxComputeWorkgroupSizeZ << '\n';
            std::cout << "\t- maxComputeWorkgroupsPerDimension:          " << supportedLimits.limits.maxComputeWorkgroupsPerDimension << '\n';
            // @formatter:on
            // clang-format on
        }

        // Adapter features.

        // Call the function a first time with a null return address, just to get
        // the entry count.
        const size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);

        // Allocate memory.
        std::vector<WGPUFeatureName> features(featureCount);

        // Call the function a second time, with a non-null return address.
        wgpuAdapterEnumerateFeatures(adapter, features.data());

        std::cout << "Adapter features:\n";
        std::cout << std::hex; // Write integers as hexadecimal to ease comparison with webgpu.h literals
        for (auto f : features) {
            std::cout << "\t- 0x" << f << '\n';
        }
        std::cout << std::dec;

        // Adapter properties.

        WGPUAdapterProperties properties{};
        properties.nextInChain = nullptr;

        wgpuAdapterGetProperties(adapter, &properties);
        std::cout << "Adapter properties:\n";
        std::cout << "\t- VendorID: " << properties.vendorID << '\n';
        if (properties.vendorName) {
            std::cout << "\t- VendorName: " << properties.vendorName << '\n';
        }
        if (properties.architecture) {
            std::cout << "\t- Architecture: " << properties.architecture << '\n';
        }
        std::cout << "\t- DeviceID: " << properties.deviceID << '\n';
        if (properties.name) {
            std::cout << "\t- Name: " << properties.name << '\n';
        }
        if (properties.driverDescription) {
            std::cout << "\t- DriverDescription: " << properties.driverDescription << '\n';
        }
        std::cout << std::hex;
        std::cout << "\t- AdapterType: 0x" << properties.adapterType << '\n';
        std::cout << "\t- BackendType: 0x" << properties.backendType << '\n';
        std::cout << std::dec;
    }
}
