#include "Library/Controller/PadRumbleFunction.hpp"

#include "Library/Controller/PadRumbleDirector.hpp"
#include "Library/Layout/LayoutActor.hpp"
#include "Library/Layout/LayoutSceneInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Player/PlayerUtil.hpp"

namespace alPadRumbleFunction {
/**
 * Returns the pad rumble director of an actor's scene.
 * @param pActor actor
 * @return the pad rumble director
 */
al::PadRumbleDirector* getPadRumbleDirector(const al::LiveActor* pActor) {
    return pActor->getSceneInfo()->padRumbleDirector;
}

/**
 * Returns the pad rumble director of a layout actor's scene.
 * @param pActor layout actor
 * @return the pad rumble director
 */
al::PadRumbleDirector* getPadRumbleDirector(const al::LayoutActor* pActor) {
    return pActor->getLayoutSceneInfo()->getPadRumbleDirector();
}

/**
 * Pauses all active rumbles.
 * @param pActor actor
 */
void pauseActivePadRumbles(al::LiveActor* pActor) {
    if (al::PadRumbleDirector* director = getPadRumbleDirector(pActor)) {
        director->pauseActiveRumbles();
    }
}

/**
 * Resumes all paused rumbles.
 * @param pActor actor
 */
void resumePausedPadRumbles(al::LiveActor* pActor) {
    if (al::PadRumbleDirector* director = getPadRumbleDirector(pActor)) {
        director->resumeActiveRumbles();
    }
}

/**
 * Starts a one-shot rumble without position.
 * @param pActor actor
 * @param pName rumble name
 * @param port controller port, or a negative value for every living player
 * @param isFlag flag passed to the director
 */
void startPadRumble(const al::LiveActor* pActor, const char* pName, s32 port, bool isFlag) {
    startPadRumbleNo3D(pActor, pName, port, isFlag);
}

/**
 * Starts a one-shot rumble without position.
 * @param pActor actor
 * @param pName rumble name
 * @param port controller port, or a negative value for every living player
 * @param isFlag flag passed to the director
 */
void startPadRumbleNo3D(const al::LiveActor* pActor, const char* pName, s32 port, bool isFlag) {
    al::PadRumbleDirector* director = getPadRumbleDirector(pActor);
    if (!director) {
        return;
    }

    if (port >= 0) {
        director->startRumbleNo3D(pName, al::PadRumbleParam(), port, isFlag);
        return;
    }

    s32 playerNum = al::getPlayerNumMaxComplete(pActor);
    for (s32 i = 0; i < playerNum; i++) {
        if (al::isPlayerDead(pActor, i)) {
            continue;
        }

        al::PadRumbleParam param;
        director->startRumbleNo3D(pName, param, al::getPlayerPort(pActor, i), isFlag);
    }
}

/**
 * Stops the direct rumble of a port.
 * @param pActor actor
 * @param port controller port
 */
void stopPadRumbleDirect(const al::LiveActor* pActor, s32 port) {
    if (al::PadRumbleDirector* director = getPadRumbleDirector(pActor)) {
        director->stopPadRumbleDirect(port);
    }
}

/**
 * Starts a one-shot rumble at a position.
 * @param pDirector pad rumble director
 * @param rPos rumble position
 * @param pName rumble name
 * @param near distance of full volume
 * @param far distance of zero volume
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumble(al::PadRumbleDirector* pDirector, const sead::Vector3f& rPos,
                    const char* pName, f32 near, f32 far, s32 port, bool isFlag) {
    al::PadRumbleParam param(near, far);
    pDirector->startRumble(pName, rPos, param, port, isFlag);
}

/**
 * Starts a one-shot rumble at a position.
 * @param pDirector pad rumble director
 * @param rPos rumble position
 * @param pName rumble name
 * @param rParam rumble parameters
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleWithParam(al::PadRumbleDirector* pDirector, const sead::Vector3f& rPos,
                             const char* pName, const al::PadRumbleParam& rParam, s32 port,
                             bool isFlag) {
    pDirector->startRumble(pName, rPos, rParam, port, isFlag);
}

/**
 * Starts a one-shot rumble with distance parameters but without position.
 * @param pActor actor
 * @param pName rumble name
 * @param near distance of full volume
 * @param far distance of zero volume
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumble(const al::LiveActor* pActor, const char* pName, f32 near, f32 far, s32 port,
                    bool isFlag) {
    al::PadRumbleParam param(near, far);
    if (al::PadRumbleDirector* director = getPadRumbleDirector(pActor)) {
        director->startRumbleNo3D(pName, param, port, isFlag);
    }
}

/**
 * Starts a one-shot rumble without position.
 * @param pActor actor
 * @param pName rumble name
 * @param rParam rumble parameters
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleNo3DWithParam(const al::LiveActor* pActor, const char* pName,
                                 const al::PadRumbleParam& rParam, s32 port, bool isFlag) {
    if (al::PadRumbleDirector* director = getPadRumbleDirector(pActor)) {
        director->startRumbleNo3D(pName, rParam, port, isFlag);
    }
}

/**
 * Starts a one-shot rumble at a position.
 * @param pActor actor
 * @param rPos rumble position
 * @param pName rumble name
 * @param rParam rumble parameters
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleWithParam(const al::LiveActor* pActor, const sead::Vector3f& rPos,
                             const char* pName, const al::PadRumbleParam& rParam, s32 port,
                             bool isFlag) {
    if (al::PadRumbleDirector* director = getPadRumbleDirector(pActor)) {
        director->startRumble(pName, rPos, rParam, port, isFlag);
    }
}

/**
 * Starts a one-shot rumble at a position.
 * @param pActor actor
 * @param rPos rumble position
 * @param pName rumble name
 * @param near distance of full volume
 * @param far distance of zero volume
 * @param port controller port, or a negative value for every living player
 * @param isFlag flag passed to the director
 */
void startPadRumblePos(const al::LiveActor* pActor, const sead::Vector3f& rPos, const char* pName,
                       f32 near, f32 far, s32 port, bool isFlag) {
    al::PadRumbleDirector* director = getPadRumbleDirector(pActor);
    if (!director) {
        return;
    }

    if (port >= 0) {
        director->startRumble(pName, rPos, al::PadRumbleParam(near, far), port, isFlag);
        return;
    }

    s32 playerNum = al::getPlayerNumMaxComplete(pActor);
    for (s32 i = 0; i < playerNum; i++) {
        if (al::isPlayerDead(pActor, i)) {
            continue;
        }

        al::PadRumbleParam param(near, far);
        director->startRumble(pName, rPos, param, al::getPlayerPort(pActor, i), isFlag);
    }
}

/**
 * Starts a one-shot rumble without position.
 * @param pDirector pad rumble director, may be nullptr
 * @param pName rumble name
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleNo3D(al::PadRumbleDirector* pDirector, const char* pName, s32 port,
                        bool isFlag) {
    if (pDirector) {
        pDirector->startRumbleNo3D(pName, al::PadRumbleParam(), port, isFlag);
    }
}

/**
 * Starts a one-shot rumble without position.
 * @param pDirector pad rumble director, may be nullptr
 * @param pName rumble name
 * @param rParam rumble parameters
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleNo3DWithParam(al::PadRumbleDirector* pDirector, const char* pName,
                                 const al::PadRumbleParam& rParam, s32 port, bool isFlag) {
    if (pDirector) {
        pDirector->startRumbleNo3D(pName, rParam, port, isFlag);
    }
}

/**
 * Starts a one-shot rumble without position.
 * @param pDirector pad rumble director, may be nullptr
 * @param pName rumble name
 * @param volumeLeft left volume
 * @param volumeRight right volume
 * @param pitchLeft left pitch
 * @param pitchRight right pitch
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleNo3DWithParam(al::PadRumbleDirector* pDirector, const char* pName,
                                 f32 volumeLeft, f32 volumeRight, f32 pitchLeft, f32 pitchRight,
                                 s32 port, bool isFlag) {
    if (pDirector) {
        al::PadRumbleParam param(0.0f, 0.0f, volumeLeft, volumeRight, pitchLeft, pitchRight);
        pDirector->startRumbleNo3D(pName, param, port, isFlag);
    }
}

/**
 * Starts a one-shot rumble without position.
 * @param pActor actor
 * @param pName rumble name
 * @param volumeLeft left volume
 * @param volumeRight right volume
 * @param pitchLeft left pitch
 * @param pitchRight right pitch
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleNo3DWithParam(const al::LiveActor* pActor, const char* pName, f32 volumeLeft,
                                 f32 volumeRight, f32 pitchLeft, f32 pitchRight, s32 port,
                                 bool isFlag) {
    startPadRumbleNo3DWithParam(getPadRumbleDirector(pActor), pName, volumeLeft, volumeRight,
                                pitchLeft, pitchRight, port, isFlag);
}

/**
 * Stops a one-shot rumble.
 * @param pDirector pad rumble director, may be nullptr
 * @param pName rumble name
 * @param port controller port
 */
void stopPadRumbleOneTime(al::PadRumbleDirector* pDirector, const char* pName, s32 port) {
    if (pDirector) {
        pDirector->stopPadRumbleOneTime(pName, port);
    }
}

/**
 * Stops a one-shot rumble.
 * @param pActor actor
 * @param pName rumble name
 * @param port controller port
 */
void stopPadRumbleOneTime(const al::LiveActor* pActor, const char* pName, s32 port) {
    stopPadRumbleOneTime(getPadRumbleDirector(pActor), pName, port);
}

/**
 * Starts a looping rumble.
 * @param pDirector pad rumble director, may be nullptr
 * @param pName rumble name
 * @param pPos rumble position
 * @param near unused
 * @param far unused
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleLoop(al::PadRumbleDirector* pDirector, const char* pName,
                        const sead::Vector3f* pPos, f32 near, f32 far, s32 port, bool isFlag) {
    if (pDirector) {
        pDirector->startRumbleLoop(pName, pPos, al::PadRumbleParam(), port, isFlag);
    }
}

/**
 * Starts a looping rumble.
 * @param pDirector pad rumble director, may be nullptr
 * @param pName rumble name
 * @param pPos rumble position
 * @param rParam rumble parameters
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleLoopWithParam(al::PadRumbleDirector* pDirector, const char* pName,
                                 const sead::Vector3f* pPos, const al::PadRumbleParam& rParam,
                                 s32 port, bool isFlag) {
    if (pDirector) {
        pDirector->startRumbleLoop(pName, pPos, rParam, port, isFlag);
    }
}

/**
 * Starts a looping rumble.
 * @param pActor actor
 * @param pName rumble name
 * @param pPos rumble position
 * @param near distance of full volume
 * @param far distance of zero volume
 * @param port controller port, or a negative value for every living player
 * @param isFlag flag passed to the director
 */
void startPadRumbleLoop(const al::LiveActor* pActor, const char* pName, const sead::Vector3f* pPos,
                        f32 near, f32 far, s32 port, bool isFlag) {
    al::PadRumbleDirector* director = getPadRumbleDirector(pActor);
    if (!director) {
        return;
    }

    if (port >= 0) {
        director->startRumbleLoop(pName, pPos, al::PadRumbleParam(near, far), port, isFlag);
        return;
    }

    s32 playerNum = al::getPlayerNumMaxComplete(pActor);
    for (s32 i = 0; i < playerNum; i++) {
        if (al::isPlayerDead(pActor, i)) {
            continue;
        }

        al::PadRumbleParam param(near, far);
        director->startRumbleLoop(pName, pPos, param, al::getPlayerPort(pActor, i), isFlag);
    }
}

/**
 * Starts a looping rumble.
 * @param pActor actor
 * @param pName rumble name
 * @param pPos rumble position
 * @param rParam rumble parameters
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleLoopWithParam(const al::LiveActor* pActor, const char* pName,
                                 const sead::Vector3f* pPos, const al::PadRumbleParam& rParam,
                                 s32 port, bool isFlag) {
    if (al::PadRumbleDirector* director = getPadRumbleDirector(pActor)) {
        director->startRumbleLoop(pName, pPos, rParam, port, isFlag);
    }
}

/**
 * Starts a looping rumble without distance attenuation.
 * @param pDirector pad rumble director
 * @param pName rumble name
 * @param pPos rumble position
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleLoopNo3D(al::PadRumbleDirector* pDirector, const char* pName,
                            const sead::Vector3f* pPos, s32 port, bool isFlag) {
    pDirector->startRumbleLoopNo3D(pName, pPos, al::PadRumbleParam(), port, isFlag);
}

/**
 * Starts a looping rumble without distance attenuation.
 * @param pDirector pad rumble director
 * @param pName rumble name
 * @param pPos rumble position
 * @param rParam rumble parameters
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleLoopNo3DWithParam(al::PadRumbleDirector* pDirector, const char* pName,
                                     const sead::Vector3f* pPos, const al::PadRumbleParam& rParam,
                                     s32 port, bool isFlag) {
    pDirector->startRumbleLoopNo3D(pName, pPos, rParam, port, isFlag);
}

/**
 * Starts a looping rumble without distance attenuation.
 * @param pActor actor
 * @param pName rumble name
 * @param pPos rumble position
 * @param port controller port, or a negative value for every living player
 * @param isFlag flag passed to the director
 */
void startPadRumbleLoopNo3D(const al::LiveActor* pActor, const char* pName,
                            const sead::Vector3f* pPos, s32 port, bool isFlag) {
    al::PadRumbleDirector* director = getPadRumbleDirector(pActor);
    if (port >= 0) {
        director->startRumbleLoopNo3D(pName, pPos, al::PadRumbleParam(), port, isFlag);
        return;
    }

    s32 playerNum = al::getPlayerNumMaxComplete(pActor);
    for (s32 i = 0; i < playerNum; i++) {
        if (al::isPlayerDead(pActor, i)) {
            continue;
        }

        al::PadRumbleParam param;
        director->startRumbleLoopNo3D(pName, pPos, param, al::getPlayerPort(pActor, i), isFlag);
    }
}

/**
 * Starts a looping rumble without distance attenuation.
 * @param pActor actor
 * @param pName rumble name
 * @param pPos rumble position
 * @param rParam rumble parameters
 * @param port controller port
 * @param isFlag flag passed to the director
 */
void startPadRumbleLoopNo3DWithParam(const al::LiveActor* pActor, const char* pName,
                                     const sead::Vector3f* pPos, const al::PadRumbleParam& rParam,
                                     s32 port, bool isFlag) {
    getPadRumbleDirector(pActor)->startRumbleLoopNo3D(pName, pPos, rParam, port, isFlag);
}

/**
 * Stops a looping rumble.
 * @param pDirector pad rumble director
 * @param pName rumble name
 * @param pPos rumble position
 * @param port controller port
 */
void stopPadRumbleLoop(al::PadRumbleDirector* pDirector, const char* pName,
                       const sead::Vector3f* pPos, s32 port) {
    pDirector->stopRumbleLoop(pName, pPos, port);
}

/**
 * Stops a looping rumble.
 * @param pActor actor
 * @param pName rumble name
 * @param pPos rumble position
 * @param port controller port, or a negative value for every living player
 */
void stopPadRumbleLoop(const al::LiveActor* pActor, const char* pName, const sead::Vector3f* pPos,
                       s32 port) {
    al::PadRumbleDirector* director = getPadRumbleDirector(pActor);
    if (port >= 0) {
        director->stopRumbleLoop(pName, pPos, port);
        return;
    }

    s32 playerNum = al::getPlayerNumMaxComplete(pActor);
    for (s32 i = 0; i < playerNum; i++) {
        if (al::isPlayerDead(pActor, i)) {
            continue;
        }

        director->stopRumbleLoop(pName, pPos, al::getPlayerPort(pActor, i));
    }
}

/**
 * Checks whether a looping rumble is playing.
 * @param pDirector pad rumble director
 * @param pName rumble name
 * @param pPos rumble position
 * @param port controller port
 * @return whether the rumble is playing
 */
bool checkIsAlivePadRumbleLoop(al::PadRumbleDirector* pDirector, const char* pName,
                               const sead::Vector3f* pPos, s32 port) {
    return pDirector->checkIsAliveRumbleLoop(pName, pPos, port);
}

/**
 * Checks whether a looping rumble is playing.
 * @param pActor actor
 * @param pName rumble name
 * @param pPos rumble position
 * @param port controller port
 * @return whether the rumble is playing
 */
bool checkIsAlivePadRumbleLoop(const al::LiveActor* pActor, const char* pName,
                               const sead::Vector3f* pPos, s32 port) {
    return getPadRumbleDirector(pActor)->checkIsAliveRumbleLoop(pName, pPos, port);
}

/**
 * Starts a looping rumble whose volume and pitch can be changed.
 * @param pActor actor
 * @param pName rumble name
 * @param pPos rumble position
 * @param port controller port
 */
void startPadRumbleLoopControlable(const al::LiveActor* pActor, const char* pName,
                                   const sead::Vector3f* pPos, s32 port) {
    startPadRumbleLoopNo3D(pActor, pName, pPos, port, false);
}

/**
 * Changes the volume of a looping rumble.
 * @param pActor actor
 * @param pName rumble name
 * @param pPos rumble position
 * @param volumeLeft left volume
 * @param volumeRight right volume
 * @param port controller port, or a negative value for every living player
 */
void changePadRumbleLoopVolmue(const al::LiveActor* pActor, const char* pName,
                               const sead::Vector3f* pPos, f32 volumeLeft, f32 volumeRight,
                               s32 port) {
    al::PadRumbleDirector* director = getPadRumbleDirector(pActor);
    if (port >= 0) {
        director->changeRumbleLoopVolume(pName, pPos, volumeLeft, volumeRight, port);
        return;
    }

    s32 playerNum = al::getPlayerNumMaxComplete(pActor);
    for (s32 i = 0; i < playerNum; i++) {
        if (al::isPlayerDead(pActor, i)) {
            continue;
        }

        director->changeRumbleLoopVolume(pName, pPos, volumeLeft, volumeRight,
                                         al::getPlayerPort(pActor, i));
    }
}

/**
 * Changes the volume of a looping rumble, eased in over a range.
 * @param pActor actor
 * @param pName rumble name
 * @param pPos rumble position
 * @param value current value
 * @param min value of zero volume
 * @param max value of full volume
 * @param volumeLeft full left volume
 * @param volumeRight full right volume
 * @param port controller port, or a negative value for every living player
 */
void changePadRumbleLoopVolmueEaseInRange(const al::LiveActor* pActor, const char* pName,
                                          const sead::Vector3f* pPos, f32 value, f32 min, f32 max,
                                          f32 volumeLeft, f32 volumeRight, s32 port) {
    f32 rate = al::easeIn(al::calcRate01(value, min, max));
    changePadRumbleLoopVolmue(pActor, pName, pPos, rate * volumeLeft, rate * volumeRight, port);
}

/**
 * Changes the pitch of a looping rumble.
 * @param pActor actor
 * @param pName rumble name
 * @param pPos rumble position
 * @param pitchLeft left pitch
 * @param pitchRight right pitch
 * @param port controller port, or a negative value for every living player
 */
void changePadRumbleLoopPitch(const al::LiveActor* pActor, const char* pName,
                              const sead::Vector3f* pPos, f32 pitchLeft, f32 pitchRight,
                              s32 port) {
    al::PadRumbleDirector* director = getPadRumbleDirector(pActor);
    if (port >= 0) {
        director->changeRumbleLoopPitch(pName, pPos, pitchLeft, pitchRight, port);
        return;
    }

    s32 playerNum = al::getPlayerNumMaxComplete(pActor);
    for (s32 i = 0; i < playerNum; i++) {
        if (al::isPlayerDead(pActor, i)) {
            continue;
        }

        director->changeRumbleLoopPitch(pName, pPos, pitchLeft, pitchRight,
                                        al::getPlayerPort(pActor, i));
    }
}

/**
 * Starts a rumble from raw motor values.
 * @param pActor actor
 * @param volumeLeft left amplitude
 * @param pitchLeft left pitch
 * @param frequencyLeft left frequency
 * @param volumeRight right amplitude
 * @param pitchRight right pitch
 * @param frequencyRight right frequency
 * @param port controller port, or a negative value for every living player
 */
void startPadRumbleDirectValue(const al::LiveActor* pActor, f32 volumeLeft, f32 pitchLeft,
                               f32 frequencyLeft, f32 volumeRight, f32 pitchRight,
                               f32 frequencyRight, s32 port) {
    al::PadRumbleDirector* director = getPadRumbleDirector(pActor);
    if (port >= 0) {
        director->startRumbleDirectValue(volumeLeft, pitchLeft, frequencyLeft, volumeRight,
                                         pitchRight, frequencyRight, port);
        return;
    }

    s32 playerNum = al::getPlayerNumMaxComplete(pActor);
    for (s32 i = 0; i < playerNum; i++) {
        if (al::isPlayerDead(pActor, i)) {
            continue;
        }

        director->startRumbleDirectValue(volumeLeft, pitchLeft, frequencyLeft, volumeRight,
                                         pitchRight, frequencyRight, al::getPlayerPort(pActor, i));
    }
}

/**
 * Stops a rumble started from raw motor values.
 * @param pActor actor
 * @param port controller port, or a negative value for every living player
 */
void stopPadRumbleDirectValue(const al::LiveActor* pActor, s32 port) {
    al::PadRumbleDirector* director = getPadRumbleDirector(pActor);
    if (port >= 0) {
        director->stopRumbleDirectValue(port);
        return;
    }

    s32 playerNum = al::getPlayerNumMaxComplete(pActor);
    for (s32 i = 0; i < playerNum; i++) {
        if (al::isPlayerDead(pActor, i)) {
            continue;
        }

        director->stopRumbleDirectValue(al::getPlayerPort(pActor, i));
    }
}

/**
 * Starts a one-shot rumble with a volume.
 * @param pActor actor
 * @param pName rumble name
 * @param volumeLeft left volume
 * @param volumeRight right volume
 * @param port controller port, or a negative value for every living player
 */
void startPadRumbleWithVolume(const al::LiveActor* pActor, const char* pName, f32 volumeLeft,
                              f32 volumeRight, s32 port) {
    al::PadRumbleDirector* director = getPadRumbleDirector(pActor);
    if (port >= 0) {
        director->startRumbleWithVolume(pName, volumeLeft, volumeRight, port);
        return;
    }

    s32 playerNum = al::getPlayerNumMaxComplete(pActor);
    for (s32 i = 0; i < playerNum; i++) {
        if (al::isPlayerDead(pActor, i)) {
            continue;
        }

        director->startRumbleWithVolume(pName, volumeLeft, volumeRight,
                                        al::getPlayerPort(pActor, i));
    }
}

/**
 * Starts a one-shot rumble with a volume.
 * @param pDirector pad rumble director
 * @param pName rumble name
 * @param volumeLeft left volume
 * @param volumeRight right volume
 * @param port controller port
 */
void startPadRumbleWithVolume(al::PadRumbleDirector* pDirector, const char* pName, f32 volumeLeft,
                              f32 volumeRight, s32 port) {
    pDirector->startRumbleWithVolume(pName, volumeLeft, volumeRight, port);
}

/**
 * Fills rumble parameters from distances and a volume.
 * @param pParam parameters to fill
 * @param near distance of full volume
 * @param far distance of zero volume
 * @param volume volume of both sides
 */
void makePadRumbleParamNearFarVolume(al::PadRumbleParam* pParam, f32 near, f32 far, f32 volume) {
    makePadRumbleParamNearFarVolumeLR(pParam, near, far, volume, volume);
}

/**
 * Fills rumble parameters from distances and volumes.
 * @param pParam parameters to fill
 * @param near distance of full volume
 * @param far distance of zero volume
 * @param volumeLeft left volume
 * @param volumeRight right volume
 */
void makePadRumbleParamNearFarVolumeLR(al::PadRumbleParam* pParam, f32 near, f32 far,
                                       f32 volumeLeft, f32 volumeRight) {
    makePadRumbleParamNearFarVolumePitchLR(pParam, near, far, volumeLeft, volumeRight, 1.0f, 1.0f);
}

/**
 * Fills rumble parameters from distances, a volume and a pitch.
 * @param pParam parameters to fill
 * @param near distance of full volume
 * @param far distance of zero volume
 * @param volume volume of both sides
 * @param pitch pitch of both sides
 */
void makePadRumbleParamNearFarVolumePitch(al::PadRumbleParam* pParam, f32 near, f32 far,
                                          f32 volume, f32 pitch) {
    makePadRumbleParamNearFarVolumePitchLR(pParam, near, far, volume, volume, pitch, pitch);
}

/**
 * Fills rumble parameters from distances, volumes and pitches.
 * @param pParam parameters to fill
 * @param near distance of full volume
 * @param far distance of zero volume
 * @param volumeLeft left volume
 * @param volumeRight right volume
 * @param pitchLeft left pitch
 * @param pitchRight right pitch
 */
void makePadRumbleParamNearFarVolumePitchLR(al::PadRumbleParam* pParam, f32 near, f32 far,
                                            f32 volumeLeft, f32 volumeRight, f32 pitchLeft,
                                            f32 pitchRight) {
    pParam->far = far;
    pParam->near = near;
    pParam->volumeLeft = volumeLeft;
    pParam->volumeRight = volumeRight;
    pParam->pitchLeft = pitchLeft;
    pParam->pitchRight = pitchRight;
    pParam->_18 = 0;
}
}  // namespace alPadRumbleFunction
