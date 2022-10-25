/*
 * D3D11RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_RENDER_CONTEXT_H
#define LLGL_D3D11_RENDER_CONTEXT_H


#include <LLGL/RenderContext.h>
#include "../DXCommon/ComPtr.h"
#include <d3d11.h>
#include <dxgi.h>


namespace LLGL
{


class D3D11CommandBuffer;

class D3D11RenderContext final : public RenderContext
{

    public:

        D3D11RenderContext(
            IDXGIFactory*                   factory,
            const ComPtr<ID3D11Device>&     device,
            const RenderContextDescriptor&  desc,
            const std::shared_ptr<Surface>& surface
        );

        void SetName(const char* name) override;

        void Present() override;

        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

    public:

        // Binds the framebuffer view of this swap-chain and stores a references to this command buffer.
        void BindFramebufferView(D3D11CommandBuffer* commandBuffer);

    private:

        bool OnSetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        bool OnSetVsyncInterval(std::uint32_t vsyncInterval) override;

        bool SetPresentSyncInterval(UINT syncInterval);

        void CreateSwapChain(IDXGIFactory* factory, UINT samples);
        void CreateBackBuffer(const VideoModeDescriptor& videoModeDesc);
        void ResizeBackBuffer(const VideoModeDescriptor& videoModeDesc);

    private:

        ComPtr<ID3D11Device>            device_;

        ComPtr<IDXGISwapChain>          swapChain_;
        UINT                            swapChainInterval_      = 0;
        DXGI_SAMPLE_DESC                swapChainSampleDesc_    = { 1, 0 };

        ComPtr<ID3D11Texture2D>         colorBuffer_;
        ComPtr<ID3D11RenderTargetView>  renderTargetView_;
        ComPtr<ID3D11Texture2D>         depthBuffer_;
        ComPtr<ID3D11DepthStencilView>  depthStencilView_;

        DXGI_FORMAT                     colorFormat_            = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT                     depthStencilFormat_     = DXGI_FORMAT_UNKNOWN;

        D3D11CommandBuffer*             bindingCommandBuffer_   = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
