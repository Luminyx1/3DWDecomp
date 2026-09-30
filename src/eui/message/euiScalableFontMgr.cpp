#include <eui/euiScalableFontMgr.h>
#include <eui/euiScalableFontTextBoxEx.h>
namespace eui {
static_assert(sizeof(sead::Thread) == 0x100, "Texture update thread base size");
bool ScalableFontMgr::UpdateTextureCacheThread::requestUpdate() {
    if (sendMessage(1, sead::MessageQueue::BlockType::NonBlocking)) {
        mUpdatePending = true;
        return true;
    }
    return false;
}
// message is unused: each request updates the texture cache and releases the pending flag.
void ScalableFontMgr::UpdateTextureCacheThread::calc_(sead::MessageQueue::Element message) {
    mTextureCache->UpdateTextureCache();
    mUpdatePending = false;
}
void ScalableFontMgr::update() {
    if (_5d) {
        if (!mUpdateThread->mUpdatePending) {
            mTextureCache->CompleteTextureCache();
            _5d = false;
            ++_58;
        }
        return;
    }
    if (_5e) {
        mTextureCache->ResetTextureCache();
        for (const auto& entry : mFonts) entry.font.RegisterAlternateCharGlyph();
        ++_58;
        _5e = false;
    }
    if (mReservedTextBox) {
        auto* textBox = mReservedTextBox;
        while (textBox) textBox = textBox->registerGlyphsAndGetNext(this);
        mReservedTextBox = nullptr;
    }
    if (_5c && mUpdateThread->requestUpdate()) {
        _5c = false;
        _5d = true;
    }
}
SEAD_SINGLETON_DISPOSER_IMPL(ScalableFontMgr);
ScalableFontMgr::ScalableFontMgr() : _28(nullptr), mTextureCache(nullptr), mUpdateThread(nullptr),
    mReservedTextBox(nullptr), _58(0), _5c(0), _5d(0), _5e(0) {}
ScalableFontMgr::FontParameter::FontParameter() : name(nullptr), size(0), face(0), _10(0) {}
// pName names the font; size and face select its glyphs; value supplies the fourth font parameter.
ScalableFontMgr::FontParameter::FontParameter(const char* pName, int size, u16 face, int value)
    : name(pName), size(size), face(face), _10(value) {}
// NON_MATCHING: branch relocation awaits registerGlyphs_ reconstruction.
// pText and length identify the text; pFont supplies the font whose glyph readiness is checked.
bool ScalableFontMgr::isGlyphsReady(const char16_t* pText, u32 length, const nn::font::ScalableFont* pFont) {
    return registerGlyphs_(pText, length, pFont, -1, true);
}
// NON_MATCHING: branch relocation awaits registerGlyphs_ reconstruction.
// pText and length identify the text; pFont selects the font; lockGroup groups cache locks.
bool ScalableFontMgr::registerGlyphs(const char16_t* pText, u32 length, const nn::font::ScalableFont* pFont, int lockGroup) {
    return registerGlyphs_(pText, length, pFont, lockGroup, false);
}
// pTextBox replaces the pending text box; the previous reservation is returned.
ScalableFontTextBoxEx* ScalableFontMgr::reserveRegisterGlyphs(ScalableFontTextBoxEx* pTextBox) {
    return __atomic_exchange_n(&mReservedTextBox, pTextBox, __ATOMIC_RELAXED);
}
// rName selects a font by its registered name; null means no font matched.
const nn::font::ScalableFont* ScalableFontMgr::getFont(const sead::SafeString& rName) const {
    const FontEntry* fonts = mFonts.getBufferPtr();
    const u32 count = mFonts.size();
    for (size_t i = 0; i != count; ++i) if (rName == fonts[i].name) return &fonts[u32(i)].font;
    return nullptr;
}
// rName selects a font by its registered name; null means no font matched.
nn::font::ScalableFont* ScalableFontMgr::getFont(const sead::SafeString& rName) {
    return const_cast<nn::font::ScalableFont*>(static_cast<const ScalableFontMgr*>(this)->getFont(rName));
}
// pFont identifies the registered font whose name is requested.
const char* ScalableFontMgr::findFontName(const nn::font::ScalableFont* pFont) const {
    for (const auto& entry : mFonts) if (&entry.font == pFont) return entry.name.cstr();
    return nullptr;
}
void ScalableFontMgr::dumpTextureCacheGlyphTreeMap() const { mTextureCache->GetGlyphTreeMap().Dump(); }
u32 ScalableFontMgr::getTextureCacheNoSpaceError() const { return mTextureCache->GetNoSpaceError(); }
void ScalableFontMgr::clearNoSpaceError() { mTextureCache->ClearNoSpaceError(); }
// lockGroup identifies the cache locks to release.
void ScalableFontMgr::clearLockAllGlyphs(int lockGroup) { mTextureCache->ClearLockAllGlyphs(lockGroup); }
// code, size, and face identify a glyph; requesting it preserves its existing plotted state.
bool ScalableFontMgr::isNeedPlot_(char16_t code, u32 size, u16 face) {
    auto* node = mTextureCache->FindGlyphNode(code, size, face);
    if (node) {
        node->SetFlag(nn::font::GlyphNode::FlagBit_Requested);
        if (!node->IsFlagOn(nn::font::GlyphNode::FlagBit_NotPlotted)) return false;
    }
    return true;
}

}
