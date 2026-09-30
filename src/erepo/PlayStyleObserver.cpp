#include <erepo/PlayStyleObserver.h>

#include <controller/nin/seadNinJoyNpadDevice.h>
#include <controller/seadControllerMgr.h>
#include <math/seadMathCalcCommon.h>
#include <nn/oe.h>
#include <prim/seadFormatPrint.h>

#include <erepo/Data/SendData.h>
#include <erepo/Manager.h>

namespace erepo {

/**
 * Constructs the observer with cleared use times.
 */
PlayStyleObserver::PlayStyleObserver()
{
    clearAllUseInfo_();
}

/**
 * Clears the current and saved use times.
 */
void PlayStyleObserver::clearAllUseInfo_()
{
    for (s32 i = 0; i < 3; i++) {
        for (s32 j = 0; j < 5; j++) {
            mUseInfo[i][j].time = 0.0f;
            mUseInfo[i][j].nonActiveTime = 0.0f;
        }
    }

    for (s32 i = 0; i < 3; i++) {
        for (s32 j = 0; j < 5; j++) {
            mSavedUseInfo[i][j].time = 0.0f;
            mSavedUseInfo[i][j].nonActiveTime = 0.0f;
        }
    }
}

/**
 * Clears the use times, detects the current style and enables mode change
 * notifications.
 * @param pHeap Unused.
 */
void PlayStyleObserver::initialize(sead::Heap* pHeap)
{
    clearAllUseInfo_();
    Manager::UpdateArg arg;
    arg.deltaTime = 1.0f / 60.0f;
    checkCurrentStyle_(arg, true);
    nn::oe::SetOperationModeChangedNotificationEnabled(true);
    nn::oe::SetPerformanceModeChangedNotificationEnabled(true);
}

/**
 * Detects the controller style and, when needed, the play style.
 * @param rArg Frame time and system message.
 * @param isForce Whether to query the operation mode regardless of the message.
 */
void PlayStyleObserver::checkCurrentStyle_(const Manager::UpdateArg& rArg, bool isForce)
{
    auto* npad = sead::ControllerMgr::instance()->getControlDeviceAs<sead::NinJoyNpadDevice*>();

    if (npad) {
        for (s32 i = 0; i < 9; i++) {
            switch (static_cast<s32>(npad->getNpadStyleTag(i))) {
            case 0:
                mControllerStyle = EControllerStyle::FullKey;
                break;
            case 1:
                mControllerStyle = EControllerStyle::Handheld;
                break;
            case 2:
                mControllerStyle = EControllerStyle::Dual;
                break;
            case 5:
                continue;
            default:
                mControllerStyle = EControllerStyle::Unknown;
                break;
            }

            break;
        }
    }

    if (mControllerStyle == EControllerStyle::Unknown) {
        mControllerStyle = Manager::instance()->getCurrentControllerStyle();
    }

    if (rArg.message == 30 || isForce) {
        switch (nn::oe::GetOperationMode()) {
        case nn::oe::OperationMode_Handheld:
            mPlayStyle = EPlayStyle::Handheld;
            break;
        case nn::oe::OperationMode_Docked:
            mPlayStyle = EPlayStyle::Console;
            break;
        default:
            mPlayStyle = EPlayStyle::Unknown;
            break;
        }
    }
}

/**
 * Loads the saved use times from the save data info.
 */
void PlayStyleObserver::load()
{
    Manager* manager = Manager::instance();

    if (!manager) {
        return;
    }

    const SaveDataInfo& info = manager->getSaveDataInfo();

    for (s32 i = 0; i < 3; i++) {
        for (s32 j = 0; j < 5; j++) {
            const auto& value = info.mValues[4 + i * 5 + j];
            mSavedUseInfo[i][j].time = value.isValid() ? static_cast<f32>(value.get()) : 0.0f;
            mSavedUseInfo[i][j].nonActiveTime = 0.0f;
        }
    }
}

/**
 * Stores the total use times into the save data info.
 * @param pData Unused.
 */
void PlayStyleObserver::save(SaveData* pData) const
{
    Manager* manager = Manager::instance();

    if (!manager) {
        return;
    }

    SaveDataInfo& info = manager->getSaveDataInfo();

    for (s32 i = 0; i < 3; i++) {
        for (s32 j = 0; j < 5; j++) {
            info.mValues[4 + i * 5 + j].set(
                static_cast<u32>(mUseInfo[i][j].time + mSavedUseInfo[i][j].time));
        }
    }
}

/**
 * Adds the frame time to the current style's use time.
 * @param rArg Frame time and system message.
 */
void PlayStyleObserver::update(const Manager::UpdateArg& rArg)
{
    checkCurrentStyle_(rArg, false);
    UseInfo& info = mUseInfo[mPlayStyle][mControllerStyle];
    info.time += rArg.deltaTime;
    mFlags.setOn(EFlag::cUpdated);
    _100 = 0;
}

/**
 * Checks whether the controller is active.
 * @return Always true.
 */
bool PlayStyleObserver::isControllerActive_() const
{
    return true;
}

bool PlayStyleObserver::report(const StringId& rId)
{
    SendData* data = createSendData_(sead::SafeString("erepo_playstyle"), 60, 0, 0, rId, true);

    if (!data) {
        return false;
    }

    for (s32 i = 0; i < 3; i++) {
        for (s32 j = 0; j < 5; j++) {
            const UseInfo& useInfo = mUseInfo[i][j];
            const UseInfo& savedUseInfo = mSavedUseInfo[i][j];

            if (useInfo.time > sead::Mathf::epsilon() ||
                savedUseInfo.time > sead::Mathf::epsilon()) {
                sead::FixedSafeString<64> prefix;
                prefix.appendWithSeadFormat("%@_%@_", EPlayStyle::text(i),
                                            EControllerStyle::text(j));

                sead::FixedSafeString<63> key;
                key = prefix;
                key.append("UseTime");
                data->addData(sead::FixedSafeString<63>(key), static_cast<u32>(useInfo.time));

                key = prefix;
                key.append("NonActiveTime");
                data->addData(sead::FixedSafeString<63>(key),
                              static_cast<u32>(useInfo.nonActiveTime));

                key = prefix;
                key.append("UseTimeTotal");
                data->addData(sead::FixedSafeString<63>(key),
                              static_cast<u32>(useInfo.time + savedUseInfo.time));

                key = prefix;
                key.append("NonActiveTimeTotal");
                data->addData(sead::FixedSafeString<63>(key),
                              static_cast<u32>(useInfo.nonActiveTime + savedUseInfo.nonActiveTime));
            }
        }
    }

    return data->requestSave();
}

}  // namespace erepo
