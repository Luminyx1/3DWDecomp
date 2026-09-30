#pragma once

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;

class SimpleLayoutAppearWaitEnd : public LayoutActor {
public:
    SimpleLayoutAppearWaitEnd(const char* pName, const char* pLayoutName,
                              const LayoutInitInfo& rInfo, const char* pArchiveName,
                              bool isLocalized);

    void appear() override;
    void end();
    void startWait();

    virtual void exeAppear();
    virtual void exeWait();
    virtual void exeEnd();

    bool isWait() const;
    bool isEnd() const;

private:
    bool mIsSkipAction = false;
};

LayoutActor* createSimpleLayout(const char* pName, const char* pLayoutName,
                                const LayoutInitInfo& rInfo, const char* pArchiveName);
}  // namespace al
