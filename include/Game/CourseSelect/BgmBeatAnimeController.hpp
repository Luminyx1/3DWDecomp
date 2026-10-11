#pragma once

#include "Library/Nerve/NerveExecutor.hpp"

namespace al {
class LiveActor;
}

class BgmBeatAnimeController : public al::NerveExecutor {
public:
    explicit BgmBeatAnimeController(al::LiveActor* pActor);
    void init(const char* pActionName, bool isIgnoreSkl, bool isIgnoreMts, bool isIgnoreMcl);
    void initAnimeInfo();
    void reset();
    void update();
    bool isPlayingJunctionAnime() const;
    void exeWait();
    void exeWaitSyncBeatBySyncAnime();
    void setActionFrameRateWithSklAnimeCheck(float rate);
    void setAnimeFrame();
    void exeSyncBeat();
    void exeWaitSyncBeatByWaitAnime();
    void exeWaitSyncBeatLightStart();
    void exeWaitSyncBeatLight();
    void exeSyncBeatLight();

private:
    al::LiveActor* mpActor;
    const char* mpActionName;
    unsigned char _20[8];
    int mSklBeatCount;
    int mSklBeatIndex;
    int mMtsBeatCount;
    int mMtsBeatIndex;
    int mMclBeatCount;
    int mMclBeatIndex;
    bool mIsIgnoreSkl;
    bool mIsIgnoreMts;
    bool mIsIgnoreMcl;
    bool mIsJunctionAnime;
    float _44;
    float _48;
    float _4c;
    float _50;
    float mBeatRate;
    int mJunctionStepCount;
};

static_assert(sizeof(BgmBeatAnimeController) == 0x60);
