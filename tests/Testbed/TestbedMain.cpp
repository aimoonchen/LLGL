/*
 * TestbedMain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "TestbedContext.h"
#include <string>


using namespace LLGL;

static void RunTestbedForRenderer(const char* moduleName)
{
    Log::Printf("Run Testbed: %s\n", moduleName);
    Log::Printf("=============================\n");
    TestbedContext context{ moduleName };
    context.RunAllTests();
    Log::Printf("=============================\n\n");
}

static const char* GetRendererModule(const std::string& name)
{
    if (name == "gl" || name == "opengl")
        return "OpenGL";
    if (name == "vk" || name == "vulkan")
        return "Vulkan";
    if (name == "mt" || name == "mtl" || name == "metal")
        return "Metal";
    if (name == "d3d11" || name == "D3D11" || name == "dx11" || name == "DX11")
        return "Direct3D11";
    if (name == "d3d12" || name == "D3D12" || name == "dx12" || name == "DX12")
        return "Direct3D12";
    if (name == "null" || name == "nil" || name == "0")
        return "Null";
    return name.c_str();
}

int main(int argc, char* argv[])
{
    Log::RegisterCallbackStd();
    if (argc >= 2)
    {
        // Run tests for specific renderer
        RunTestbedForRenderer(GetRendererModule(argv[1]));
    }
    else
    {
        // Run tests for all available renderers
        for (std::string moduleName : RenderSystem::FindModules())
            RunTestbedForRenderer(moduleName.c_str());
    }
    #ifdef _WIN32
    system("pause");
    #endif
    return 0;
}



