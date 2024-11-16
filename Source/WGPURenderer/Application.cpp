// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <WGPURenderer/Application.hpp>

#include <WGPURenderer/WGPUDeviceUtils.hpp>

#include <glfw3webgpu.h>

#include <webgpu/wgpu.h>

#include <iostream>

namespace WGPURenderer {
    bool Application::Run() {
        if (!Initialize()) {
            return false;
        }

        while (!glfwWindowShouldClose(m_Window)) {
            glfwPollEvents();
            MainLoop();
        }

        Terminate();

        return true;
    }

    bool Application::Initialize() {
        if (!glfwInit()) {
            std::cerr << "Couldn't initialize GLFW!\n";
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_Window = glfwCreateWindow(640, 480, "WebGPU Renderer", nullptr, nullptr);

        if (!m_Window) {
            std::cerr << "Couldn't create GLFW window!\n";
            glfwTerminate();
            return false;
        }

        WGPUInstanceDescriptor desc;
        desc.nextInChain = nullptr;

        const WGPUInstance instance = wgpuCreateInstance(&desc);

        if (!instance) {
            std::cerr << "Couldn't initialize WebGPU!\n";
            return false;
        }

        std::cout << "WebGPU instance: 0x" << instance << '\n';

        m_Surface = glfwGetWGPUSurface(instance, m_Window);

        if (!m_Surface) {
            std::cerr << "Couldn't get WebGPU surface!\n";
            return false;
        }

        std::cout << "Requesting WebGPU adapter...\n";

        WGPURequestAdapterOptions adapterOptions{};
        adapterOptions.nextInChain = nullptr;
        adapterOptions.compatibleSurface = m_Surface;
        const WGPUAdapter adapter = RequestAdapterSync(instance, &adapterOptions);

        if (!adapter) {
            std::cerr << "Couldn't retrieve WebGPU adapter!\n";
            return false;
        }

        std::cout << "Got WebGPU adapter: 0x" << adapter << '\n';

        InspectAdapter(adapter);

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

        deviceDesc.deviceLostCallback = [](const WGPUDeviceLostReason reason, const char* message,
                                           void* /*pUserData*/) {
            std::cout << "Device lost: reason: " << reason;
            if (message) {
                std::cout << " (" << message << ')';
            }

            std::cout << '\n';
        };

        m_Device = RequestDeviceSync(adapter, &deviceDesc);

        if (!m_Device) {
            std::cerr << "Couldn't retrieve WebGPU device!\n";
            return false;
        }

        // Release the adapter after we got the device, we don't need it anymore.
        wgpuAdapterRelease(adapter);

        std::cout << "Got WebGPU device: 0x" << m_Device << '\n';

        auto onDeviceError = [](const WGPUErrorType type, const char* message, void* /*pUserData*/) {
            std::cerr << "Uncaptured device error: type: " << type;
            if (message) {
                std::cerr << " (" << message << ')';
            }

            std::cout << '\n';
        };
        wgpuDeviceSetUncapturedErrorCallback(m_Device, onDeviceError, nullptr);

        InspectDevice(m_Device);

        m_Queue = wgpuDeviceGetQueue(m_Device);

        return true;
    }

    void Application::MainLoop() const {
        wgpuDevicePoll(m_Device, false, nullptr);
    }

    void Application::Terminate() const {
        wgpuQueueRelease(m_Queue);
        wgpuDeviceRelease(m_Device);
        wgpuSurfaceRelease(m_Surface);
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }
}
