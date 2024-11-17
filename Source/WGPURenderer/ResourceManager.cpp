// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <WGPURenderer/ResourceManager.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace WGPURenderer {
    bool ResourceManager::LoadGeometry(const std::filesystem::path& path,
                                       std::vector<float>& pointData,
                                       std::vector<uint16_t>& indexData) {
        std::ifstream file(std::filesystem::path("Resources/Models") / path);
        if (!file.is_open()) {
            return false;
        }

        pointData.clear();
        indexData.clear();

        enum class Section : uint8_t {
            None,
            Points,
            Indices,
        };
        auto currentSection = Section::None;

        float value;
        uint16_t index;
        std::string line;
        while (!file.eof()) {
            getline(file, line);

            // overcome the `CRLF` problem
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line == "[points]") {
                currentSection = Section::Points;
            } else if (line == "[indices]") {
                currentSection = Section::Indices;
            } else if (line[0] == '#' || line.empty()) {
                // Do nothing, this is a comment
            } else if (currentSection == Section::Points) {
                std::istringstream iss(line);
                // Get x, y, r, g, b
                for (int i = 0; i < 5; ++i) {
                    iss >> value;
                    pointData.push_back(value);
                }
            } else if (currentSection == Section::Indices) {
                std::istringstream iss(line);
                // Get corners #0 #1 and #2
                for (int i = 0; i < 3; ++i) {
                    iss >> index;
                    indexData.push_back(index);
                }
            }
        }
        return true;
    }

    wgpu::ShaderModule ResourceManager::LoadShaderModule(const std::filesystem::path& path,
                                                         wgpu::Device device) {
        std::ifstream file(std::filesystem::path("Resources/Shaders") / path);
        if (!file.is_open()) {
            return nullptr;
        }

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        std::string shaderSource(size, ' ');
        file.seekg(0);
        file.read(shaderSource.data(), static_cast<std::streamsize>(size));

        wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
        shaderCodeDesc.chain.next = nullptr;
        shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;

        // ReSharper thinks it's not used because it's not directly referenced, but it's still used.
        
        // ReSharper disable once CppAssignedValueIsNeverUsed
        shaderCodeDesc.code = shaderSource.c_str();

        wgpu::ShaderModuleDescriptor shaderDesc{};
        shaderDesc.hintCount = 0;
        shaderDesc.hints = nullptr;
        shaderDesc.nextInChain = &shaderCodeDesc.chain;

        return device.createShaderModule(shaderDesc);
    }
}
