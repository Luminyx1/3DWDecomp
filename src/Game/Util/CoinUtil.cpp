#include "Util/CoinUtil.hpp"
#include "Library/Audio/System/AudioVolumeCtrl.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "MapObj/ItemAssistRotateParam.hpp"
#include "Util/AreaObjUtil.hpp"

namespace {
/// Assist rotation parameters shared by every coin.
const ItemAssistRotateParam sCoinAssistRotateParam(60, 20.0f, false, 0.0f, 0);
}  // namespace

namespace CoinUtil {
/**
 * Gets the assist rotation parameters of coins.
 * @return the coin assist rotation parameters
 */
const ItemAssistRotateParam* getCoinAssistRotateParam() {
    return &sCoinAssistRotateParam;
}

/**
 * Starts the spin of a coin when the mic is blown into while the coin is visible on screen
 * (or inside a frame-out control area).
 * @param pActor coin actor
 * @param pNerve spin nerve to set
 * @return true if the spin was started
 */
bool tryStartSpinCoinIfMicInputOn(al::LiveActor* pActor, const al::Nerve* pNerve) {
    if (!al::isMicInputOn(pActor)) {
        return false;
    }

    if (!rc::isInAreaObj(pActor, rc::AreaObjType::FrameOutCtrlArea)) {
        sead::Vector2f screenPos = {0.0f, 0.0f};
        al::calcScreenPosFromWorldPos(&screenPos, pActor, al::getTrans(pActor), 0);

        if (!al::isInScreen(screenPos, 0.0f)) {
            return false;
        }
    }

    al::setNerve(pActor, pNerve);
    return true;
}
}  // namespace CoinUtil
