#include <eui/euiScalableFontTextBoxEx.h>
namespace eui {
// pMgr owns the scalable font glyph cache used by this text box.
ScalableFontTextBoxEx::ScalableFontTextBoxEx(ScalableFontMgr* pMgr)
    : mFontMgr(pMgr), mGlyphState(0), _16c(0), _170(nullptr) {}
// rInfo and rCommands supply draw state and commands; glyphs must be ready before drawing.
void ScalableFontTextBoxEx::DrawSelf(nn::ui2d::DrawInfo& rInfo, nn::gfx::CommandBuffer& rCommands) {
    if (mGlyphState == 2) nn::ui2d::TextBox::DrawSelf(rInfo, rCommands);
}
}
