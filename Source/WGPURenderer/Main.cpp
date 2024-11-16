// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#include <WGPURenderer/Application.hpp>

#include <cstdlib>

int main() {
    WGPURenderer::Application app;

    if (!app.Run()) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
