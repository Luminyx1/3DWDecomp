#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ISceneObj.hpp"

class alModelCafe;
class DrcTouchAssistInfo;

namespace al {
class ActorInitInfo;
class IUseAudioKeeper;
class LayoutResource;
class Resource;
}  // namespace al

namespace rc {
class Stamp;

/**
 * @brief Scene object owning the collectible stamps and their resources.
 * @note Minimal declaration: only what rc::Stamp uses is declared so far.
 */
class StampDirector : public al::ISceneObj {
public:
    StampDirector(const al::ActorInitInfo& rInfo, const char* pArchiveName,
                  const DrcTouchAssistInfo* pTouchInfo, s32, s32);

    void activateStamp(Stamp* pStamp, s32);
    void releaseStamp(Stamp* pStamp);
    void setCollectStamp(s32 stampId);
    Stamp* getStampFromModel(alModelCafe* pModel) const;

    const al::Resource* getStampResource() const { return mStampResource; }

    al::LayoutResource* getLayoutResource() const { return mLayoutResource; }

    bool isUseNoCodeWallFilter() const { return _49; }

    const al::IUseAudioKeeper* getAudioKeeperUser() const { return mAudioKeeperUser; }

private:
    al::Resource* mStampResource;              // 0x08
    al::LayoutResource* mLayoutResource;       // 0x10
    void* _18;                                 // 0x18
    u8 _20[0x28];                              // 0x20
    bool _48;                                  // 0x48
    bool _49;                                  // 0x49
    al::IUseAudioKeeper* mAudioKeeperUser;     // 0x50
};
}  // namespace rc
