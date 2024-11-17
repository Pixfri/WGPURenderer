// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <WGPURenderer/ShaderLoader.hpp>

#include <fstream>
#include <iostream>

namespace WGPURenderer {
    std::string ReadShaderFileContent(const std::filesystem::path& shaderPath) {
        std::string shaderCode;
        std::ifstream shaderFile;

        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            shaderFile.open(std::filesystem::path("Resources/Shaders") / shaderPath);
            std::stringstream shaderStream;

            shaderStream << shaderFile.rdbuf();

            shaderFile.close();

            shaderCode = shaderStream.str();
        } catch (const std::ifstream::failure& e) {
            std::cerr << "Couldn't read shader content at path: " << shaderPath << '\n' << "Error: " << e.what() << '\n';
        }

        return shaderCode;
    }
}
