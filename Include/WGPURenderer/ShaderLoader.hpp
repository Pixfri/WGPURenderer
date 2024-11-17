// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#ifndef WR_SHADERLOADER_HPP
#define WR_SHADERLOADER_HPP

#include <filesystem>
#include <string>

namespace WGPURenderer {
    std::string ReadShaderFileContent(const std::filesystem::path& shaderPath);
}

#endif // WR_SHADERLOADER_HPP
