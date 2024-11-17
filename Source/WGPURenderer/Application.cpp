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

        // Configure the surface
        WGPUSurfaceConfiguration surfaceConfiguration;
        surfaceConfiguration.nextInChain = nullptr;
        surfaceConfiguration.width = 640;
        surfaceConfiguration.height = 480;

        const WGPUTextureFormat surfaceFormat = wgpuSurfaceGetPreferredFormat(m_Surface, adapter);
        surfaceConfiguration.format = surfaceFormat;

        // We don't need any particular view format
        surfaceConfiguration.viewFormatCount = 0;
        surfaceConfiguration.viewFormats = nullptr;

        surfaceConfiguration.usage = WGPUTextureUsage_RenderAttachment;

        surfaceConfiguration.device = m_Device;

        surfaceConfiguration.presentMode = WGPUPresentMode_Fifo;

        surfaceConfiguration.alphaMode = WGPUCompositeAlphaMode_Auto;

        wgpuSurfaceConfigure(m_Surface, &surfaceConfiguration);

        wgpuAdapterRelease(adapter);

        return true;
    }

    void Application::MainLoop() const {
        // Get the next target texture view.
        const WGPUTextureView targetView = GetNextSurfaceTextureView();
        if (!targetView) {
            return;
        }

        WGPUCommandEncoderDescriptor encoderDesc;
        encoderDesc.nextInChain = nullptr;
#ifdef WR_DEBUG
        encoderDesc.label = "Main command encoder";
#else
        encoderDesc.label = nullptr;
#endif

        // Create an encoder to register our commands.
        const WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_Device, &encoderDesc);

        // Describe and create a render pass encoder from the command encoder.
        WGPURenderPassDescriptor renderPassDesc{};
        renderPassDesc.nextInChain = nullptr;
#ifdef WR_DEBUG
        renderPassDesc.label = "Main render pass";
#else
        renderPassDesc.label = nullptr;
#endif

        WGPURenderPassColorAttachment renderPassColorAttachment;
        renderPassColorAttachment.nextInChain = nullptr;
        renderPassColorAttachment.view = targetView;
        renderPassColorAttachment.resolveTarget = nullptr;
        renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
        renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
        renderPassColorAttachment.clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0};

        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.depthStencilAttachment = nullptr;
        renderPassDesc.timestampWrites = nullptr;
        
        // Create the render pass encoder
        const WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

        // Release the render pass encoder when we're done using it.
        wgpuRenderPassEncoderEnd(renderPass);
        wgpuRenderPassEncoderRelease(renderPass);

        // Describe and build a command buffer from the encoder's registered commands.
        WGPUCommandBufferDescriptor cmdBufferDesc;
        cmdBufferDesc.nextInChain = nullptr;
#ifdef WR_DEBUG
        cmdBufferDesc.label = "Main command buffer";
#else
        cmdBufferDesc.label = nullptr;
#endif
        
        const WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);
        wgpuCommandEncoderRelease(encoder);

        // Submit the command buffer to the GPU and release it.
        wgpuQueueSubmit(m_Queue, 1, &cmdBuffer);
        wgpuCommandBufferRelease(cmdBuffer);

        // Release the surface texture view. 
        wgpuTextureViewRelease(targetView);

        // Present the surface.
        wgpuSurfacePresent(m_Surface);
        
        wgpuDevicePoll(m_Device, false, nullptr);
    }

    void Application::Terminate() const {
        wgpuSurfaceUnconfigure(m_Surface);
        wgpuQueueRelease(m_Queue);
        wgpuDeviceRelease(m_Device);
        wgpuSurfaceRelease(m_Surface);
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    WGPUTextureView Application::GetNextSurfaceTextureView() const {
        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture(m_Surface, &surfaceTexture);

        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
            return nullptr;
        }

        WGPUTextureViewDescriptor viewDescriptor;
        viewDescriptor.nextInChain = nullptr;
#ifdef WR_DEBUG
        viewDescriptor.label = "Surface texture view";
#else
        viewDescriptor.label = nullptr;
#endif
        viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
        viewDescriptor.dimension = WGPUTextureViewDimension_2D;
        viewDescriptor.baseMipLevel = 0;
        viewDescriptor.mipLevelCount = 1;
        viewDescriptor.baseArrayLayer = 0;
        viewDescriptor.arrayLayerCount = 1;
        viewDescriptor.aspect = WGPUTextureAspect_All;
        const WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

        return targetView;
    }
}
