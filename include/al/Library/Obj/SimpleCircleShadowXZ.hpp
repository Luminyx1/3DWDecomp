#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class SimpleCircleShadowXZ : public LiveActor {
public:
    SimpleCircleShadowXZ(const char*);

    void makeActorAppeared() override;
    void control() override;

    void initSimpleCircleShadow(LiveActor*, const ActorInitInfo&, const char*, const char*);
    void updatePose();
    void syncHostVisible();
    void setOffsetWithInterpole(const sead::Vector3f&);
    void setScaleWithInterpole(const sead::Vector3f&);
    void setRotateWithInterpole(const sead::Vector3f&);
    void setInterpoleFrame(s32);

    LiveActor* mHostActor = nullptr;                          // _148
    sead::Vector3f mOffset = sead::Vector3f::zero;            // _150
    bool mIsHostHidden = false;                               // _15c
    bool mIsSyncHostHide = false;                             // _15d
    sead::Vector3f mScaleStart = {1.0f, 1.0f, 1.0f};          // _160
    sead::Vector3f mScaleEnd = {1.0f, 1.0f, 1.0f};            // _16c
    sead::Vector3f mOffsetStart = sead::Vector3f::zero;       // _178
    sead::Vector3f mOffsetEnd = sead::Vector3f::zero;         // _184
    sead::Vector3f mRotateStart = sead::Vector3f::zero;       // _190
    sead::Vector3f mRotate = sead::Vector3f::zero;            // _19c
    sead::Vector3f mRotateEnd = sead::Vector3f::zero;         // _1a8
    s32 mInterpoleStep = 0;                                   // _1b4
    s32 mInterpoleFrame = 0;                                  // _1b8
};
}  // namespace al
