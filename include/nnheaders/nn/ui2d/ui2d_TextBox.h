#pragma once
#include <nn/ui2d/ui2d_Pane.h>
#include <nn/ui2d/ui2d_TextSearcher.h>
#include <nn/font/font_TagProcessorBase.h>
namespace nn::ui2d {
struct BuildResultInformation;
class TextBox : public Pane {
public:
    struct InitializeStringParam;
    TextBox();
    ~TextBox() override;
    NN_RUNTIME_TYPEINFO(Pane);
    void Finalize(nn::gfx::Device*) override;
    nn::util::Unorm8x4 GetVertexColor(int) const override;
    void SetVertexColor(int, const nn::util::Unorm8x4&) override;
    u8 GetVertexColorElement(int) const override;
    void SetVertexColorElement(int, u8) override;
    u32 GetMaterialCount() const override;
    Material* GetMaterial(int) const override;
    void GetSizeWithCaptureEffect(Size*) const override;
    void GetVertexPosWithCaptureEffect(nn::util::Float2*) const override;
    float GetItalicSize() const override;
    void Calculate(DrawInfo&, CalculateContext&, bool) override;
    void DrawSelf(DrawInfo&, nn::gfx::CommandBuffer&) override;
    void SetupPaneEffectSourceImageRenderState(nn::gfx::CommandBuffer&) const override;
    void LoadMtx(DrawInfo&) override;
    virtual void InitializeString(BuildResultInformation*, nn::gfx::Device*, const BuildArgSet&, const InitializeStringParam&);
    virtual void AllocateStringBuffer(nn::gfx::Device*, u16);
    virtual void AllocateStringBuffer(nn::gfx::Device*, u16, u16);
    virtual void FreeStringBuffer(nn::gfx::Device*);
    virtual u16 SetString(const u16*, u16);
    virtual u16 SetStringUtf8(const char*, u16);
    virtual u16 SetString(const u16*, u16, u16);
    virtual u16 SetStringUtf8(const char*, u16, u16);
    virtual nn::font::Rectangle GetTextDrawRect() const;
    virtual void SetupTextWriter(nn::font::TextWriterBase<u16>*) const;
    virtual void SetupTextWriterUtf8(nn::font::TextWriterBase<char>*) const;
    virtual void InitializeStringWithTextSearcherInfo(nn::gfx::Device*, const BuildArgSet&, const TextSearcher::TextInfo&);
    virtual void InitializeStringWithTextSearcherInfoUtf8(nn::gfx::Device*, const BuildArgSet&, const TextSearcher::TextInfoUtf8&);
    bool ValidateNextPrintableChar(const char* current, const char* next);
    bool ValidateNextPrintableChar(const u16* current, const u16* next);
    u32 GetCharFromPointer(const char* text);
    u32 GetCharFromPointer(const u16* text);
protected:
    // Text storage and rendering members await reconstruction; offsets are verified against constructors.
    u8 _d2[6];
    void* mTextBuffer;
    u8 _e0[0x32];
    u16 mTextLength;
    u16 mTextFlags;
    u8 _116[0x2a];
    Material* mMaterial;
    u8 _148[0x10];
};
static_assert(sizeof(TextBox) == 0x158, "TextBox size");
}
