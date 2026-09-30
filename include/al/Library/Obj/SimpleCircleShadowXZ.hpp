#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ActorInitInfo;

class SimpleCircleShadowXZ : public LiveActor {
public:
    SimpleCircleShadowXZ(const char* pName);

    void makeActorAppeared() override;
    void control() override;

    void initSimpleCircleShadow(LiveActor* pHost, const ActorInitInfo& rInfo,
                                const char* pArchiveName, const char* pSuffix);
    void updatePose();
    void syncHostVisible();
    void setOffsetWithInterpole(const sead::Vector3f& rOffset);
    void setScaleWithInterpole(const sead::Vector3f& rScale);
    void setRotateWithInterpole(const sead::Vector3f& rRotate);
    void setInterpoleFrame(s32 frame);

    LiveActor* mHost = nullptr;
    sead::Vector3f mOffset = sead::Vector3f::zero;
    bool mIsHostHidden = false;
    bool mIsForceHide = false;
    sead::Vector3f mStartScale = {1.0f, 1.0f, 1.0f};
    sead::Vector3f mEndScale = {1.0f, 1.0f, 1.0f};
    sead::Vector3f mStartOffset = sead::Vector3f::zero;
    sead::Vector3f mEndOffset = sead::Vector3f::zero;
    sead::Vector3f mStartRotate = sead::Vector3f::zero;
    sead::Vector3f mRotate = sead::Vector3f::zero;
    sead::Vector3f mEndRotate = sead::Vector3f::zero;
    s32 mInterpoleStep = 0;
    s32 mInterpoleFrame = 0;
};

static_assert(sizeof(SimpleCircleShadowXZ) == 0x1c0);
}  // namespace al
