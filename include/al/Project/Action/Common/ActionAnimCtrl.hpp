#pragma once

#include <basis/seadTypes.h>

namespace al {
struct ActionAnimCtrlInfo;
class LiveActor;

class ActionAnimCtrl {
public:
    static ActionAnimCtrl* tryCreate(LiveActor* pActor, const char* pArchiveName,
                                     const char* pSuffix);

    ActionAnimCtrl(LiveActor* pActor);

    void init(const char* pArchiveName, const char* pSuffix);
    bool start(const char* pActionName);
    ActionAnimCtrlInfo* findAnimInfo(const char* pActionName) const;
    f32 getFrame() const;
    f32 getActionFrameMax(const char* pActionName) const;
    f32 getFrameRate() const;
    bool trySetFrame(f32 frame);
    bool isExistAction(const char* pActionName) const;
    bool isActionOneTime(const char* pActionName) const;
    const char* getPlayingActionName() const;
    void sortCtrlInfo();

private:
    LiveActor* mParentActor;
    const char* mArchiveName = nullptr;
    s32 mInfoCount = 0;
    ActionAnimCtrlInfo** mInfos = nullptr;
    ActionAnimCtrlInfo* mPlayingInfo = nullptr;
    s32 mAnimType = -1;
};

static_assert(sizeof(ActionAnimCtrl) == 0x30);

}  // namespace al
