// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#ifndef WR_APPLICATION_HPP
#define WR_APPLICATION_HPP

#include <GLFW/glfw3.h>

#include <webgpu/webgpu.h>

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
        WGPUSurface m_Surface = nullptr;
        WGPUDevice m_Device = nullptr;
        WGPUQueue m_Queue = nullptr;

        bool Initialize();

        void MainLoop() const;

        void Terminate() const;
    };
}

#endif // WR_APPLICATION_HPP
