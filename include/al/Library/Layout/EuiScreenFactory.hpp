#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <eui/euiScreenFactory.h>

namespace sead {
class Heap;
}

namespace al {
class EuiScreenFactory : public eui::ScreenFactory {
public:
    void initialize(sead::Heap* pHeap, s32 screenNum);
    eui::Screen* createScreen(sead::Heap* pHeap, s32 screenId) override;
    const char* getScreenName(s32 screenId) const override;
    s32 getScreenNum() const override;
    s32 findScreenId(const char* pScreenName) const;
    s32 assignScreenName(const char* pScreenName);

private:
    sead::Buffer<const char*> mScreenNames;
    sead::Buffer<void*> mScreenUserData;
};

static_assert(sizeof(EuiScreenFactory) == 0x28);
}  // namespace al
