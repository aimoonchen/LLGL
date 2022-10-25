/*
 * DbgRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderContext.h"
#include "DbgCore.h"


namespace LLGL
{


DbgRenderContext::DbgRenderContext(RenderContext& instance) :
    instance { instance }
{
    ShareSurfaceAndConfig(instance);
}

void DbgRenderContext::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}

void DbgRenderContext::Present()
{
    instance.Present();
}

std::uint32_t DbgRenderContext::GetSamples() const
{
    return instance.GetSamples();
}

Format DbgRenderContext::GetColorFormat() const
{
    return instance.GetColorFormat();
}

Format DbgRenderContext::GetDepthStencilFormat() const
{
    return instance.GetDepthStencilFormat();
}

const RenderPass* DbgRenderContext::GetRenderPass() const
{
    return instance.GetRenderPass();
}


/*
 * ======= Private: =======
 */

bool DbgRenderContext::OnSetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    auto result = instance.SetVideoMode(videoModeDesc);
    ShareSurfaceAndConfig(instance);
    return result;
}

bool DbgRenderContext::OnSetVsyncInterval(std::uint32_t vsyncInterval)
{
    auto result = instance.SetVsyncInterval(vsyncInterval);
    ShareSurfaceAndConfig(instance);
    return result;
}


} // /namespace LLGL



// ================================================================================
