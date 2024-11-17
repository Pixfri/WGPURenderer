// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <WGPURenderer/Application.hpp>

#include <webgpu/webgpu.hpp>

#include <glfw3webgpu.h>

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

        wgpu::Instance instance = wgpuCreateInstance(nullptr);

        if (!instance) {
            std::cerr << "Couldn't initialize WebGPU!\n";
            return false;
        }

        std::cout << "WebGPU instance: " << instance << '\n';

        m_Surface = glfwGetWGPUSurface(instance, m_Window);

        if (!m_Surface) {
            std::cerr << "Couldn't get WebGPU surface!\n";
            return false;
        }

        std::cout << "Requesting WebGPU adapter...\n";

        wgpu::RequestAdapterOptions adapterOptions{};
        adapterOptions.nextInChain = nullptr;
        adapterOptions.compatibleSurface = m_Surface;
        wgpu::Adapter adapter = instance.requestAdapter(adapterOptions);

        if (!adapter) {
            std::cerr << "Couldn't retrieve WebGPU adapter!\n";
            return false;
        }

        std::cout << "Got WebGPU adapter: " << adapter << '\n';

        // Release the instance since we don't need it after we've got the adapter.
        instance.release();

        std::cout << "Requesting WebGPU device...\n";

        wgpu::DeviceDescriptor deviceDesc{};
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

        m_Device = adapter.requestDevice(deviceDesc);

        if (!m_Device) {
            std::cerr << "Couldn't retrieve WebGPU device!\n";
            return false;
        }

        std::cout << "Got WebGPU device: " << m_Device << '\n';

        m_UncapturedErrorCallbackHandle = m_Device.setUncapturedErrorCallback(
            [](const wgpu::ErrorType type, const char* message) {
                std::cerr << "Uncaptured device error: type: " << type;
                if (message) {
                    std::cerr << " (" << message << ')';
                }

                std::cout << '\n';
            });


        m_Queue = m_Device.getQueue();

        // Configure the surface
        wgpu::SurfaceConfiguration surfaceConfiguration;
        surfaceConfiguration.nextInChain = nullptr;
        surfaceConfiguration.width = 640;
        surfaceConfiguration.height = 480;

        const wgpu::TextureFormat surfaceFormat = m_Surface.getPreferredFormat(adapter);
        surfaceConfiguration.format = surfaceFormat;

        // We don't need any particular view format
        surfaceConfiguration.viewFormatCount = 0;
        surfaceConfiguration.viewFormats = nullptr;

        surfaceConfiguration.usage = wgpu::TextureUsage::RenderAttachment;

        surfaceConfiguration.device = m_Device;

        surfaceConfiguration.presentMode = wgpu::PresentMode::Fifo;

        surfaceConfiguration.alphaMode = wgpu::CompositeAlphaMode::Auto;

        m_Surface.configure(surfaceConfiguration);

        adapter.release();

        return true;
    }

    void Application::MainLoop() {
        // Get the next target texture view.
        wgpu::TextureView targetView = GetNextSurfaceTextureView();
        if (!targetView) {
            return;
        }

        wgpu::CommandEncoderDescriptor encoderDesc;
        encoderDesc.nextInChain = nullptr;
#ifdef WR_DEBUG
        encoderDesc.label = "Main command encoder";
#else
        encoderDesc.label = nullptr;
#endif

        // Create an encoder to register our commands.
        wgpu::CommandEncoder encoder = m_Device.createCommandEncoder(encoderDesc);

        // Describe and create a render pass encoder from the command encoder.
        wgpu::RenderPassDescriptor renderPassDesc{};
        renderPassDesc.nextInChain = nullptr;
#ifdef WR_DEBUG
        renderPassDesc.label = "Main render pass";
#else
        renderPassDesc.label = nullptr;
#endif

        wgpu::RenderPassColorAttachment renderPassColorAttachment;
        renderPassColorAttachment.nextInChain = nullptr;
        renderPassColorAttachment.view = targetView;
        renderPassColorAttachment.resolveTarget = nullptr;
        renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
        renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
        renderPassColorAttachment.clearValue = wgpu::Color{0.9, 0.1, 0.2, 1.0};

        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.depthStencilAttachment = nullptr;
        renderPassDesc.timestampWrites = nullptr;

        // Create the render pass encoder
        wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

        // Release the render pass encoder when we're done using it.
        renderPass.end();
        renderPass.release();

        // Describe and build a command buffer from the encoder's registered commands.
        wgpu::CommandBufferDescriptor cmdBufferDesc;
        cmdBufferDesc.nextInChain = nullptr;
#ifdef WR_DEBUG
        cmdBufferDesc.label = "Main command buffer";
#else
        cmdBufferDesc.label = nullptr;
#endif

        wgpu::CommandBuffer cmdBuffer = encoder.finish(cmdBufferDesc);
        encoder.release();

        // Submit the command buffer to the GPU and release it.
        m_Queue.submit(1, &cmdBuffer);
        cmdBuffer.release();

        // Release the surface texture view. 
        targetView.release();

        // Present the surface.
        m_Surface.present();

        m_Device.poll(false);
    }

    void Application::Terminate() {
        m_Surface.unconfigure();
        m_Queue.release();
        m_Device.release();
        m_Surface.release();
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    wgpu::TextureView Application::GetNextSurfaceTextureView() {
        wgpu::SurfaceTexture surfaceTexture;
        m_Surface.getCurrentTexture(&surfaceTexture);

        if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
            return nullptr;
        }
        wgpu::Texture texture = surfaceTexture.texture;

        wgpu::TextureViewDescriptor viewDescriptor;
        viewDescriptor.nextInChain = nullptr;
#ifdef WR_DEBUG
        viewDescriptor.label = "Surface texture view";
#else
        viewDescriptor.label = nullptr;
#endif
        viewDescriptor.format = texture.getFormat();
        viewDescriptor.dimension = wgpu::TextureViewDimension::_2D;
        viewDescriptor.baseMipLevel = 0;
        viewDescriptor.mipLevelCount = 1;
        viewDescriptor.baseArrayLayer = 0;
        viewDescriptor.arrayLayerCount = 1;
        viewDescriptor.aspect = wgpu::TextureAspect::All;
        wgpu::TextureView targetView = texture.createView(viewDescriptor);

        return targetView;
    }
}
