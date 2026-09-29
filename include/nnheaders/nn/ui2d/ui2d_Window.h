#pragma once

#include <nn/ui2d/ui2d_Pane.h>

namespace nn::ui2d {
struct ResWindow;
struct BuildResultInformation;

class Window : public Pane {
public:
    Window(int, int);
    Window(BuildResultInformation*, nn::gfx::Device*, const ResWindow*, const ResWindow*,
           const BuildArgSet&);
    Window(const Window& rOther, nn::gfx::Device* pDevice) : Pane(rOther) {
        CopyImpl(rOther, pDevice, nullptr, nullptr);
    }
    ~Window() override;
    NN_RUNTIME_TYPEINFO(Pane);
    void Finalize(nn::gfx::Device*) override;
    nn::util::Unorm8x4 GetVertexColor(int) const override;
    void SetVertexColor(int, const nn::util::Unorm8x4&) override;
    u8 GetVertexColorElement(int) const override;
    void SetVertexColorElement(int, u8) override;
    u32 GetMaterialCount() const override;
    Material* GetMaterial(int) const override;
    Material* FindMaterialByName(const char*, bool) override;
    const Material* FindMaterialByName(const char*, bool) const override;
    void Calculate(DrawInfo&, CalculateContext&, bool) override;
    void DrawSelf(DrawInfo&, nn::gfx::CommandBuffer&) override;
    void SetupPaneEffectSourceImageRenderState(nn::gfx::CommandBuffer&) const override;
    void CopyImpl(const Window&, nn::gfx::Device*, const Layout*, detail::BuildPaneTreeContext*);

    unsigned char _D2[0x5e];
};
}  // namespace nn::ui2d
