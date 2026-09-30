#include <nn/ui2d/ui2d_Pane.h>
#include <nn/util/util_StringUtil.h>
namespace nn::ui2d {
// name supplies at most 24 visible characters plus a terminator.
void Pane::SetName(const char* name) { nn::util::Strlcpy(mPanelName, name, sizeof(mPanelName)); }
// data supplies the pane's eight-character user data string.
void Pane::SetUserData(const char* data) { nn::util::Strlcpy(mUserData, data, sizeof(mUserData)); }
// child is inserted after the last child and its global transform becomes dirty.
void Pane::AppendChild(Pane* child) {
    m_Children.LinkPrev(&child->m_Link);
    child->mParent = this;
    child->mFlags |= 0x10;
}

// child is inserted before the first child and its global transform becomes dirty.
void Pane::PrependChild(Pane* child) {
    m_Children.GetNext()->LinkPrev(&child->m_Link);
    child->mParent = this;
    child->mFlags |= 0x10;
}

// child is detached from this pane's child list without destroying it.
void Pane::RemoveChild(Pane* child) {
    if (&m_Children != &child->m_Link) child->m_Link.Unlink();
    child->mParent = nullptr;
}
}
