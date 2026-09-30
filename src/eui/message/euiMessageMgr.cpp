#include <eui/euiMessageMgr.h>
namespace eui {
SEAD_SINGLETON_DISPOSER_IMPL(MessageMgr);
MessageMgr::MessageMgr() : mTextBoxWidthSizeOverColorEnabled(true) { mArchives.initOffset(0x20); }
MessageMgr::~MessageMgr() = default;
// index selects the gradient; top and bottom are its endpoint colors.
void MessageMgr::setGradationColor(u32 index, sead::Color4u8 top, sead::Color4u8 bottom) {
    auto& color = mGradationColors[index];
    color.top = top;
    color.bottom = bottom;
}
void MessageMgr::dumpLastGotMessageSetInfo() {}
}
