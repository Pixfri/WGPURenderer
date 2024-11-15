// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#include <iostream>

namespace WGPURenderer {
    inline void Main::TestPrint(const std::string& msg) {
        std::cout << msg << std::endl;
    }
}
