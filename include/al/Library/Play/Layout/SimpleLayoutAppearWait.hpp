#pragma once

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;

class SimpleLayoutAppearWait : public LayoutActor {
public:
    SimpleLayoutAppearWait(const char* pName, const char* pLayoutName, const LayoutInitInfo& rInfo,
                           const char* pArchiveName);

    void appear() override;
    virtual void appear2();

    void exeAppear();
    void exeWait();
};
}  // namespace al
