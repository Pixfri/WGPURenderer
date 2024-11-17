// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#ifndef WR_APPLICATION_HPP
#define WR_APPLICATION_HPP

#include <GLFW/glfw3.h>

#include <webgpu/webgpu.hpp>

namespace WGPURenderer {
    class Application {
    public:
        Application() = default;
        ~Application() = default;

        Application(const Application&) = delete;
        Application(Application&&) = delete;

        Application& operator=(const Application&) = delete;
        Application& operator=(Application&&) = delete;

        bool Run();
    
    private:
        GLFWwindow* m_Window = nullptr;
        wgpu::Surface m_Surface = nullptr;
        wgpu::TextureFormat m_SurfaceFormat = wgpu::TextureFormat::Undefined;
        wgpu::Device m_Device = nullptr;
        wgpu::Queue m_Queue = nullptr;
        std::unique_ptr<wgpu::ErrorCallback> m_UncapturedErrorCallbackHandle = nullptr;
        wgpu::RenderPipeline m_Pipeline = nullptr;
        
        bool Initialize();

        void MainLoop();

        void Terminate();

        bool InitializePipeline();

        wgpu::TextureView GetNextSurfaceTextureView();
    };
}

#endif // WR_APPLICATION_HPP
