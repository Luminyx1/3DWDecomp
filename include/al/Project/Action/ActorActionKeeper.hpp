#pragma once

#include <basis/seadTypes.h>

namespace al {
    class ActionAnimCtrl;
    class ActionBgmCtrl;
    class ActionEffectCtrl;
    class ActionFlagCtrl;
    class ActionOceanWaveCtrl;
    class ActionPadAndCameraCtrl;
    class ActionScreenEffectCtrl;
    class ActionSeCtrl;
    class LiveActor;
    class NerveActionCtrl;

    /// Holds the controllers that play an actor's action resources (animations, effects, sounds...).
    class ActorActionKeeper {
    public:
        static ActorActionKeeper* tryCreate(LiveActor* pActor, const char* pArchiveName, const char* pActionListName);

        ActorActionKeeper(LiveActor* pActor, const char* pArchiveName, ActionAnimCtrl* pAnimCtrl,
                          NerveActionCtrl* pNerveActionCtrl, ActionFlagCtrl* pFlagCtrl, ActionEffectCtrl* pEffectCtrl,
                          ActionSeCtrl* pSeCtrl, ActionBgmCtrl* pBgmCtrl, ActionOceanWaveCtrl* pOceanWaveCtrl,
                          ActionPadAndCameraCtrl* pPadAndCameraCtrl, ActionScreenEffectCtrl* pScreenEffectCtrl);

        void startAction(const char* pActionName);
        void tryStartActionNoAnim(const char* pActionName);
        void startBgmAction(const char* pActionName);
        void startEffectAction(const char* pActionName);
        void updatePrev();
        void updatePost();
        void updateSeActionCtrl();
        void tryUpdateSeEffect(f32 prevFrame, f32 frame);
        void init();

        LiveActor* mActor;                              // _0
        const char* mArchiveName;                       // _8
        bool mIsActionRunning;                          // _10
        ActionAnimCtrl* mAnimCtrl;                      // _18
        NerveActionCtrl* mNerveActionCtrl;              // _20
        ActionFlagCtrl* mFlagCtrl;                      // _28
        ActionEffectCtrl* mEffectCtrl;                  // _30
        ActionSeCtrl* mSeCtrl;                          // _38
        ActionBgmCtrl* mBgmCtrl;                        // _40
        ActionOceanWaveCtrl* mOceanWaveCtrl;            // _48
        ActionPadAndCameraCtrl* mPadAndCameraCtrl;      // _50
        ActionScreenEffectCtrl* mScreenEffectCtrl;      // _58
    };
};
