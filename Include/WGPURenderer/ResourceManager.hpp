// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#ifndef WR_RESOURCEMANAGER_HPP
#define WR_RESOURCEMANAGER_HPP

#include <webgpu/webgpu.hpp>

#include <filesystem>
#include <vector>

namespace WGPURenderer {
    class ResourceManager {
    public:
        ResourceManager() = delete;
        ~ResourceManager() = delete;

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager(ResourceManager&&) = delete;

        ResourceManager& operator=(const ResourceManager&) = delete;
        ResourceManager& operator=(ResourceManager&&) = delete;

        static bool LoadGeometry(const std::filesystem::path& path,
                                 std::vector<float>& pointData,
                                 std::vector<uint16_t>& indexData);

        static wgpu::ShaderModule LoadShaderModule(const std::filesystem::path& path,
                                                   wgpu::Device device);

    private:
    };
}

#endif // WR_RESOURCEMANAGER_HPP
