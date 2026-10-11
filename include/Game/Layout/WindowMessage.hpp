#pragma once
#include "Library/Layout/LayoutActor.hpp"
namespace al { class LayoutInitInfo; }

// Layout and interface required by course-select messages.
class WindowMessage : public al::LayoutActor {
public:
    WindowMessage(const al::LayoutInitInfo& rInfo, const char* pLayout, const char* pName, const char* pArchive);
    void appearWithSystemMessage(const char* pLayout, const char* pMessage, int inputPort);
private:
    void* mpState;
    unsigned char _130[0x10];
};
static_assert(sizeof(WindowMessage) == 0x140);
