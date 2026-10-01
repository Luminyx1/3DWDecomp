#pragma once

namespace al {
class GraphicsCopyInfo;
class GraphicsComputeInfo;
class GraphicsUpdateInfo;
class GraphicsRenderInfo;
class GraphicsCalcGpuInfo;
class RenderVariables;

class IUsePartsGraphics {
public:
    virtual void finalize() = 0;
    virtual void endInit();
    virtual void doCommandBufferCopy(const GraphicsCopyInfo* pInfo) const {}
    virtual void doComputeShader(const GraphicsComputeInfo* pInfo) const {}
    virtual void drawSystem(const GraphicsRenderInfo* pInfo) const;
    virtual void update(const GraphicsUpdateInfo& rInfo) = 0;
    virtual void calcGpu(const GraphicsCalcGpuInfo& rInfo) = 0;
    virtual void drawGBufferAfterSky(const GraphicsRenderInfo& rInfo) const {}
    virtual void drawForward(const GraphicsRenderInfo& rInfo, const RenderVariables& rVars) const {}
    virtual void drawDeferred(const GraphicsRenderInfo& rInfo) const {}
    virtual void drawLdr(const GraphicsRenderInfo& rInfo) const {}
    virtual void drawIndirect(const GraphicsRenderInfo& rInfo, const RenderVariables& rVars) const {}
    virtual void drawCubemap(const GraphicsRenderInfo& rInfo) const {}
    virtual const char* getName() const = 0;
};
}  // namespace al
