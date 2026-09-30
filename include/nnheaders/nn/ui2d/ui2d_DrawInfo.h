#pragma once

#include <nn/font/font_Util.h>
#include <nn/ui2d/ui2d_Pane.h>
#include <nn/util/util_MathTypes.h>
#include <nn/gfx/gfx_StateInfo.h>
#include <nn/gfx/gfx_DescriptorSlot.h>

namespace nn {
namespace font { class GpuBuffer; }
namespace ui2d {
class GraphicsResource;
class ShaderInfo;
class DrawInfo {
public:
    DrawInfo();
    NN_RUNTIME_TYPEINFO_BASE();
    virtual ~DrawInfo();
    void ResetDrawState();
    void SetProjectionMtx(const nn::util::MatrixT4x4fType& rProjection);
    void ConfigureBeforeDrawing(Layout* pLayout);
    void ConfigureAfterDrawing();
    void ResetCurrentShader();
    void Map(int bufferIndex);
    void Unmap();
    void SetGpuAccessBufferIndex(int bufferIndex);
    void SetFramebufferTexture(const nn::gfx::Texture* pTexture, int width, int height);
    void SetupProgram(nn::gfx::CommandBuffer* pCommands);
    bool RecordCurrentShader(const ShaderInfo* pShader, u16 variation);
    void ResetRenderTarget(nn::gfx::CommandBuffer& rCommands) const;
    void LoadProjectionMtx(float (*pMatrix)[4]);
    void LoadMtxModelView(float (*pMatrix)[4]);
    void SetFramebufferTextureDescriptorSlot(const nn::gfx::DescriptorSlot* pSlot);
    void SetFramebufferSamplerDescriptorSlot(const nn::gfx::DescriptorSlot* pSlot);
    const nn::gfx::DescriptorSlot* GetFramebufferTextureDescriptorSlot() const;
    const nn::gfx::DescriptorSlot* GetFramebufferSamplerDescriptorSlot() const;

    nn::util::MatrixT4x4fType m_ProjMtx;
    nn::util::MatrixT4x3fType m_ViewMtx;
    nn::util::MatrixT4x3fType m_ModelViewMtx;
    nn::util::Float2 m_LocationAdjustScale;
    const GraphicsResource* m_pGraphicsResource;
    const Pane::CalculateContext::LayoutInformation* m_pLayoutInformation;
    nn::font::GpuBuffer* m_pConstantBuffer;
    nn::font::GpuBuffer* m_pFontConstantBuffer;
    const nn::gfx::Texture* m_pFramebufferTexture;
    int mFramebufferWidth;
    int mFramebufferHeight;
    const nn::gfx::DescriptorSlot* m_pFramebufferTextureSlot;
    const nn::gfx::DescriptorSlot* m_pFramebufferSamplerSlot;
    const nn::gfx::ColorTargetView* m_pColorTarget;
    nn::gfx::ViewportStateInfo mViewport;
    nn::gfx::ScissorStateInfo mScissor;
    const nn::gfx::DepthStencilView* m_pDepthTarget;
    const nn::gfx::DepthStencilState* m_pDepthStencilState;
    const nn::gfx::RasterizerState* m_pRasterizerState;
    unsigned char _168[0x20];
    unsigned char _188[3];
    bool mModelViewLoaded;
    bool mVertexBufferDirty;
    bool _18D;
    u8 mFlags;
    const ShaderInfo* m_pCurrentShader;
    u16 mCurrentShaderVariation;

};
static_assert(sizeof(DrawInfo) == 0x1a0, "DrawInfo size");
}  // namespace ui2d
}  // namespace nn
