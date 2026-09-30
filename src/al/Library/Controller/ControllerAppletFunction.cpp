#include <nn/hid.h>
#include <prim/seadStringUtil.h>

#include "Library/Controller/GamePadSystem.hpp"
#include "Project/Audio/System/AudioSystem.hpp"

namespace al {
/**
 * Shows the controller support applet.
 * @param pSystem game pad system
 * @param minPlayerNum minimum number of players
 * @param maxPlayerNum maximum number of players
 * @return whether the applet succeeded
 */
bool tryCallControllerApplet(const GamePadSystem* pSystem, s32 minPlayerNum, s32 maxPlayerNum) {
    if (pSystem->getAudioSystem() != nullptr) {
        pSystem->getAudioSystem()->pauseSystemImmediately(true, "コントローラサポートアプレット",
                                                          true);
    }
    nn::hid::ControllerSupportArg arg;
    arg.SetDefault();
    arg.mMinPlayerCount = minPlayerNum;
    arg.mMaxPlayerCount = maxPlayerNum;
    arg.mTakeOverConnection = true;
    arg.mLeftJustify = true;
    arg.mPermitJoyconDual = true;
    arg.mSingleMode = maxPlayerNum == 1;
    arg.mUseColors = false;
    if (maxPlayerNum >= 2) {
        arg.mUsingControllerNames = true;
        for (s32 i = 0; i < maxPlayerNum; i++) {
            sead::StringUtil::convertUtf16ToUtf8(arg.mControllerNames[i], 0x81,
                                                 pSystem->getPadName(i).cstr(), -1);
        }
    }
    nn::hid::ControllerSupportResultInfo resultInfo;
    nn::Result result = nn::hid::ShowControllerSupport(&resultInfo, arg);
    if (pSystem->getAudioSystem() != nullptr) {
        pSystem->getAudioSystem()->pauseSystemImmediately(false, "コントローラサポートアプレット",
                                                          true);
    }
    return result.IsSuccess();
}

/**
 * Shows the controller support applet with custom arguments.
 * @param pSystem game pad system
 * @param pArg applet arguments
 * @param pResultInfo applet result
 * @param isUseControllerNames whether to show the pad names
 * @return whether the applet succeeded
 */
bool tryCallControllerApplet(const GamePadSystem* pSystem, nn::hid::ControllerSupportArg* pArg,
                             nn::hid::ControllerSupportResultInfo* pResultInfo,
                             bool isUseControllerNames) {
    if (pSystem->getAudioSystem() != nullptr) {
        pSystem->getAudioSystem()->pauseSystemImmediately(true, "コントローラサポートアプレット",
                                                          true);
    }
    if (isUseControllerNames && pArg->mMaxPlayerCount >= 2) {
        pArg->mUsingControllerNames = true;
        for (s32 i = 0; i < pArg->mMaxPlayerCount; i++) {
            sead::StringUtil::convertUtf16ToUtf8(pArg->mControllerNames[i], 0x81,
                                                 pSystem->getPadName(i).cstr(), -1);
        }
    }
    nn::Result result = nn::hid::ShowControllerSupport(pResultInfo, *pArg);
    if (pSystem->getAudioSystem() != nullptr) {
        pSystem->getAudioSystem()->pauseSystemImmediately(false, "コントローラサポートアプレット",
                                                          true);
    }
    return result.IsSuccess();
}
}  // namespace al
