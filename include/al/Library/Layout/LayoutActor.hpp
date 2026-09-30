#pragma once

#include <prim/seadSafeString.h>

#include "Library/HostIO/IUseHioNode.hpp"
#include "Library/Layout/IUseLayout.hpp"
#include "Library/Layout/IUseLayoutAction.hpp"
#include "Library/Message/IUseMessageSystem.hpp"
#include "Library/Nerve/IUseNerve.hpp"
#include "Library/Scene/IUseSceneObjHolder.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"
#include "Project/Camera/Core/IUseCameraDirector.hpp"
#include "Project/Effect/Core/IUseEffectKeeper.hpp"

namespace al {
class NerveKeeper;
class LayoutKeeper;
class LayoutActionKeeper;
class LayoutTextPaneAnimator;
class EffectKeeper;
class AudioKeeper;
class HitReactionKeeper;
class LayoutSceneInfo;
class LayoutPartsActorKeeper;
class SceneCameraInfo;
class SceneObjHolder;
class MessageSystem;
class Nerve;

class LayoutActor : public IUseHioNode,
                    public IUseNerve,
                    public IUseLayout,
                    public IUseLayoutAction,
                    public IUseMessageSystem,
                    public IUseCamera,
                    public IUseAudioKeeper,
                    public IUseEffectKeeper,
                    public IUseSceneObjHolder {
public:
    LayoutActor(const char* pName);

    virtual void appear();
    virtual void kill();
    virtual void movement();
    virtual void calcAnim(bool isRecursive);

    NerveKeeper* getNerveKeeper() const override { return mNerveKeeper; }
    const char* getName() const override { return mName.cstr(); }
    EffectKeeper* getEffectKeeper() const override { return mEffectKeeper; }
    AudioKeeper* getAudioKeeper() const override { return mAudioKeeper; }
    LayoutActionKeeper* getLayoutActionKeeper() const override { return mLayoutActionKeeper; }
    LayoutKeeper* getLayoutKeeper() const override { return mLayoutKeeper; }
    SceneCameraInfo* getSceneCameraInfo() const override;
    SceneObjHolder* getSceneObjHolder() const override;
    const MessageSystem* getMessageSystem() const override;

    virtual void control() {}

    void syncAction();
    void initLayoutKeeper(LayoutKeeper* pLayoutKeeper);
    void initActionKeeper();
    void initTextPaneAnimator(LayoutTextPaneAnimator* pAnimator);
    void initHitReactionKeeper(HitReactionKeeper* pKeeper);
    void initSceneInfo(LayoutSceneInfo* pSceneInfo);
    void initLayoutPartsActorKeeper(s32 capacity);
    void initEffectKeeper(EffectKeeper* pEffectKeeper);
    void initAudioKeeper(AudioKeeper* pAudioKeeper);
    void initNerve(const Nerve* pNerve, s32 maxStates);
    void setMainGroupName(const char* pGroupName);

    bool isAlive() const { return mIsAlive; }
    LayoutTextPaneAnimator* getTextPaneAnimator() const { return mTextPaneAnimator; }
    HitReactionKeeper* getHitReactionKeeper() const { return mHitReactionKeeper; }
    LayoutSceneInfo* getLayoutSceneInfo() const { return mLayoutSceneInfo; }
    LayoutPartsActorKeeper* getLayoutPartsActorKeeper() const { return mLayoutPartsActorKeeper; }

private:
    sead::FixedSafeString<0x80> mName;
    NerveKeeper* mNerveKeeper;
    LayoutKeeper* mLayoutKeeper;
    LayoutActionKeeper* mLayoutActionKeeper;
    LayoutTextPaneAnimator* mTextPaneAnimator;
    EffectKeeper* mEffectKeeper;
    AudioKeeper* mAudioKeeper;
    HitReactionKeeper* mHitReactionKeeper;
    LayoutSceneInfo* mLayoutSceneInfo;
    LayoutPartsActorKeeper* mLayoutPartsActorKeeper;
    bool mIsAlive;
};
}  // namespace al
