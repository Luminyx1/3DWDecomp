#include "Library/LiveActor/Util/ActorSceneUtil.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Scene/SceneStopCtrl.hpp"

namespace al {
/**
 * Requests the scene to stop for a number of frames.
 * @param pActor The requesting actor.
 * @param stopFrames The number of frames to stop.
 * @param delayFrames The number of frames before the stop starts.
 * @param isStopEffect Whether effects stop too.
 * @param isStopAudio Whether audio stops too.
 */
void stopScene(const LiveActor* pActor, s32 stopFrames, s32 delayFrames, bool isStopEffect,
               bool isStopAudio) {
    pActor->getSceneInfo()->sceneStopCtrl->reqeustStopScene(stopFrames, delayFrames, isStopEffect,
                                                            isStopAudio);
}
}  // namespace al
