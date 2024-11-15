// Copyright (C) 2024 Jean "Pixfri" Letessier 
// This file is part of WGPURenderer.
// For conditions of distribution and use, see copyright notice in LICENSE.

#pragma once

#ifndef WR_MAIN_HPP
#define WR_MAIN_HPP

#include <string>

namespace WGPURenderer {
    class Main {
    public:
        Main() = delete;
        ~Main() = delete;

        Main(const Main&) = delete;
        Main(Main&&) = delete;

        Main& operator=(const Main&) = delete;
        Main& operator=(Main&&) = delete;

        static inline void TestPrint(const std::string& msg);
    
    private:
    };
}

#include <WGPURenderer/Main.inl>

#endif // WR_MAIN_HPP
