// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <WGPURenderer/Application.hpp>
#include <WGPURenderer/ResourceManager.hpp>

#include <webgpu/webgpu.hpp>

#include <glfw3webgpu.h>

#include <array>
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

        m_Surface = glfwGetWGPUSurface(instance, m_Window);

        if (!m_Surface) {
            std::cerr << "Couldn't get WebGPU surface!\n";
            return false;
        }

        wgpu::RequestAdapterOptions adapterOptions{};
        adapterOptions.nextInChain = nullptr;
        adapterOptions.compatibleSurface = m_Surface;
        wgpu::Adapter adapter = instance.requestAdapter(adapterOptions);

        if (!adapter) {
            std::cerr << "Couldn't retrieve WebGPU adapter!\n";
            return false;
        }

        // Release the instance since we don't need it after we've got the adapter.
        instance.release();

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

        m_SurfaceFormat = m_Surface.getPreferredFormat(adapter);
        surfaceConfiguration.format = m_SurfaceFormat;

        // We don't need any particular view format
        surfaceConfiguration.viewFormatCount = 0;
        surfaceConfiguration.viewFormats = nullptr;

        surfaceConfiguration.usage = wgpu::TextureUsage::RenderAttachment;

        surfaceConfiguration.device = m_Device;

        surfaceConfiguration.presentMode = wgpu::PresentMode::Fifo;

        surfaceConfiguration.alphaMode = wgpu::CompositeAlphaMode::Auto;

        m_Surface.configure(surfaceConfiguration);

        adapter.release();

        if (!InitializePipeline()) {
            std::cerr << "Failed to initialize pipeline!\n";
            return false;
        }

        if (!InitializeBuffers()) {
            std::cerr << "Failed to initialize buffers!\n";
            return false;
        }

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
        renderPassColorAttachment.clearValue = wgpu::Color{0.01, 0.01, 0.01, 1.0};

        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.depthStencilAttachment = nullptr;
        renderPassDesc.timestampWrites = nullptr;

        // Create the render pass encoder
        wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

        renderPass.setPipeline(m_Pipeline);

        renderPass.setVertexBuffer(0, m_PointBuffer, 0, m_PointBuffer.getSize());
        renderPass.setIndexBuffer(m_IndexBuffer, wgpu::IndexFormat::Uint16, 0, m_IndexBuffer.getSize());

        renderPass.drawIndexed(m_IndexCount, 1, 0, 0, 0);

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
        m_IndexBuffer.release();
        m_PointBuffer.release();
        m_Pipeline.release();
        m_Surface.unconfigure();
        m_Queue.release();
        m_Device.release();
        m_Surface.release();
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    bool Application::InitializePipeline() {
        wgpu::ShaderModule shaderModule = ResourceManager::LoadShaderModule("main.wgsl", m_Device);

        if (!shaderModule) {
            std::cerr << "Couldn't load shader!";
            return false;
        }

        wgpu::RenderPipelineDescriptor pipelineDesc{};

        wgpu::VertexBufferLayout vertexBufferLayout;

        std::array<wgpu::VertexAttribute, 2> vertexAttributes{};
        // Position attribute
        vertexAttributes[0].shaderLocation = 0;
        vertexAttributes[0].format = wgpu::VertexFormat::Float32x2;
        vertexAttributes[0].offset = 0;

        // Color attribute
        vertexAttributes[1].shaderLocation = 1;
        vertexAttributes[1].format = wgpu::VertexFormat::Float32x3;
        vertexAttributes[1].offset = 2 * sizeof(float);

        vertexBufferLayout.attributeCount = vertexAttributes.size();
        vertexBufferLayout.attributes = vertexAttributes.data();

        vertexBufferLayout.arrayStride = 5ull * sizeof(float);
        vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

        pipelineDesc.vertex.bufferCount = 1;
        pipelineDesc.vertex.buffers = &vertexBufferLayout;

        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.vertex.entryPoint = "vs_main";
        pipelineDesc.vertex.constantCount = 0;
        pipelineDesc.vertex.constants = nullptr;

        // Each sequence of 3 vertices is a triangle.
        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

        // We'll see later how to specify the order in which vertices should be
        // connected. When not specified, vertices are considered sequentially.
        pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;

        // The face orientation is defined by assuming that when looking
        // from the front of the face, its corner vertices are enumerated
        // in the counter-clockwise (CCW) order.
        pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;

        // But the face orientation does not matter much because we do not
        // cull (i.e. "hide") the faces pointing away from us (which is often
        // used for optimization).
        pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

        // We tell that the programmable fragment shader stage is described
        // by the function called 'fs_main' in the shader module.
        wgpu::FragmentState fragmentState;
        fragmentState.module = shaderModule;
        fragmentState.entryPoint = "fs_main";
        fragmentState.constantCount = 0;
        fragmentState.constants = nullptr;

        wgpu::BlendState blendState;
        blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
        blendState.color.dstFactor = wgpu::BlendFactor::OneMinusDstAlpha;
        blendState.color.operation = wgpu::BlendOperation::Add;

        blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
        blendState.alpha.dstFactor = wgpu::BlendFactor::One;
        blendState.alpha.operation = wgpu::BlendOperation::Add;

        wgpu::ColorTargetState colorTarget{};
        colorTarget.format = m_SurfaceFormat;
        colorTarget.blend = &blendState;
        colorTarget.writeMask = wgpu::ColorWriteMask::All; // We could write to only some of the color channels.

        // We have only one target because our render pass has only one output color
        // attachment.
        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;

        pipelineDesc.fragment = &fragmentState;

        pipelineDesc.depthStencil = nullptr;

        // Samples per pixel
        pipelineDesc.multisample.count = 1;
        // Default value for the mask, meaning "all bits on"
        pipelineDesc.multisample.mask = ~0u;
        // Default value as well (irrelevant for count = 1 anyway)
        pipelineDesc.multisample.alphaToCoverageEnabled = false;

        pipelineDesc.layout = nullptr;

        m_Pipeline = m_Device.createRenderPipeline(pipelineDesc);

        if (!m_Pipeline) {
            std::cerr << "Failed to create render pipeline!\n";
            return false;
        }

        return true;
    }

    bool Application::InitializeBuffers() {
        std::vector<float> pointData;
        std::vector<uint16_t> indexData;

        // Check for errors
        if (!ResourceManager::LoadGeometry("webgpu.txt", pointData, indexData)) {
            std::cerr << "Couldn't load geometry!\n";
            return false;
        }

        m_IndexCount = static_cast<uint32_t>(indexData.size());

        // Create vertex buffer
        wgpu::BufferDescriptor bufferDesc{};
        bufferDesc.size = pointData.size() * sizeof(float);
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
        bufferDesc.mappedAtCreation = false;
        m_PointBuffer = m_Device.createBuffer(bufferDesc);

        m_Queue.writeBuffer(m_PointBuffer, 0, pointData.data(), bufferDesc.size);

        // Create index buffer
        bufferDesc.size = indexData.size() * sizeof(uint16_t);
        bufferDesc.size = (bufferDesc.size + 3) & ~3; // round up to the next multiple of 4
        bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
        m_IndexBuffer = m_Device.createBuffer(bufferDesc);

        m_Queue.writeBuffer(m_IndexBuffer, 0, indexData.data(), bufferDesc.size);

        return true;
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
        const wgpu::TextureView targetView = texture.createView(viewDescriptor);

        return targetView;
    }
}
