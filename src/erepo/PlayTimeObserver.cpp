#include <erepo/PlayTimeObserver.h>

#include <controller/seadController.h>
#include <controller/seadControllerMgr.h>
#include <math/seadMathCalcCommon.h>
#include <nn/oe.h>
#include <nn/time.h>
#include <prim/seadFormatPrint.h>

#include <erepo/Data/SaveDataInfo.h>
#include <erepo/Data/SendData.h>
#include <erepo/Manager.h>

namespace erepo {

namespace {
const u32 cBeaconIntervals[] = {0, 60, 300, 900, 1800, 3600};
}

/**
 * Constructs the observer.
 */
PlayTimeObserver::PlayTimeObserver() {}

/**
 * Clears the multiplayer times.
 * @param pHeap Unused.
 */
void PlayTimeObserver::initialize(sead::Heap* pHeap)
{
    for (s32 i = 0; i < cPlayerNumMax; i++) {
        mPlayerNumTimes[i] = 0.0f;
        mSavedPlayerNumTimes[i] = 0.0f;
    }
}

/**
 * Loads the saved play times from the save data info.
 */
void PlayTimeObserver::load()
{
    Manager* manager = Manager::instance();

    if (manager == nullptr) {
        return;
    }

    SaveDataInfo info = manager->getSaveDataInfo();

    info.getValue(SaveDataInfo::cValueIndex_PlayTime).tryGet(&mSavedPlayTime);
    info.getValue(SaveDataInfo::cValueIndex_ActiveTime).tryGet(&mSavedActiveTime);
    info.getValue(SaveDataInfo::cValueIndex_SleepTime).tryGet(&mSavedSleepTime);

    for (s32 i = 0; i < cPlayerNumMax; i++) {
        f32 time = 0.0f;
        info.getFloat(i).tryGet(&time);
        mSavedPlayerNumTimes[i] = time;
    }
}

/**
 * Stores the total play times into the save data info.
 * @param pData Unused.
 */
void PlayTimeObserver::save(SaveData* pData) const
{
    Manager* manager = Manager::instance();

    if (manager == nullptr) {
        return;
    }

    SaveDataInfo& info = manager->getSaveDataInfo();
    info.getValue(SaveDataInfo::cValueIndex_PlayTime)
        .set(static_cast<u32>(static_cast<f32>(mSavedPlayTime) + mPlayTime));
    info.getValue(SaveDataInfo::cValueIndex_ActiveTime).set(mSavedActiveTime + mActiveTime);
    info.getValue(SaveDataInfo::cValueIndex_SleepTime).set(mSavedSleepTime + mSleepTime);

    for (s32 i = 0; i < cPlayerNumMax; i++) {
        info.getFloat(i).set(mPlayerNumTimes[i] + mSavedPlayerNumTimes[i]);
    }
}

/**
 * Advances the play time, sends the active beacon when due and tracks how many
 * controllers are being actively used.
 * @param rArg Frame time and system message.
 */
void PlayTimeObserver::update(const Manager::UpdateArg& rArg)
{
    mPlayTime = rArg.deltaTime + mPlayTime;

    if (mBeaconIndex == 0 || mPlayTime > static_cast<f32>(mNextBeaconTime)) {
        sendActiveBeacon_();
        mBeaconIndex = mBeaconIndex + 1 >= 5 ? 5 : mBeaconIndex + 1;
        mNextBeaconTime =
            static_cast<u32>(mPlayTime + static_cast<f32>(cBeaconIntervals[mBeaconIndex]));
    }

    const s32 num = sead::Mathi::min(sead::ControllerMgr::instance()->getControllerNum(),
                                     cPlayerNumMax);

    for (s32 i = 0; i < num; i++) {
        sead::Controller* controller = sead::ControllerMgr::instance()->getController(i);

        u32 time = 0;

        if (controller != nullptr && controller->isConnected()) {
            if (controller->getTrigMask() != 0 || controller->getHoldMask() != 0) {
                mControllerActiveTimes[i] = 30;
                continue;
            }

            if (controller->getLeftStick().x == 0.0f && controller->getLeftStick().y == 0.0f &&
                controller->getRightStick().x == 0.0f &&
                controller->getRightStick().y == 0.0f) {
                continue;
            }

            time = 30;
        }

        mControllerActiveTimes[i] = time;
    }

    s32 activeNum = 0;

    for (s32 i = 0; i < cPlayerNumMax; i++) {
        mControllerActiveTimes[i] -= rArg.deltaTime;

        if (mControllerActiveTimes[i] != 0) {
            activeNum++;
        }
    }

    if (activeNum >= 1 && activeNum <= 4) {
        mPlayerNumTimes[activeNum - 1] += rArg.deltaTime;
    }
}

/**
 * Sends the active beacon for the current interval.
 * @return Whether the data was queued.
 */
bool PlayTimeObserver::sendActiveBeacon_()
{
    SendData* data =
        createSendData_(sead::SafeString("erepo_active_beacon"), 2, 0, 0, StringId(), false);
    if (data == nullptr) {
        return false;
    }

    data->addData(KeyString(sead::SafeString("IntervalTime")),
                  cBeaconIntervals[mBeaconIndex]);
    data->addData(KeyString(sead::SafeString("UpdatedTime")),
                  static_cast<u32>(mPlayTime));
    return data->requestSave();
}

/**
 * Sends the play time report.
 * @param rId Report id.
 * @return Whether the data was queued.
 */
bool PlayTimeObserver::report(const StringId& rId)
{
    SendData* data = createSendData_(sead::SafeString("erepo_time"), 16, 0, 0, rId, true);

    if (data == nullptr) {
        return false;
    }

    mActiveTime = nn::oe::GetProgramTotalActiveTime().GetSeconds();
    data->addData(KeyString(sead::SafeString("ActiveTime")), mActiveTime);
    data->addData(KeyString(sead::SafeString("UpdatedTime")),
                  static_cast<u32>(mPlayTime));
    data->addData(KeyString(sead::SafeString("SleepTime")),
                  static_cast<u32>(mSleepTime));

    Manager* manager = Manager::instance();

    if (manager != nullptr) {
        const SaveDataInfo& info = manager->getSaveDataInfo();

        if (info.getValue(SaveDataInfo::cValueIndex_ActiveTime).isValid()) {
            data->addData(KeyString(sead::SafeString("ActiveTotalTime")),
                          mActiveTime + mSavedActiveTime);
        }

        if (info.getValue(SaveDataInfo::cValueIndex_PlayTime).isValid()) {
            data->addData(KeyString(sead::SafeString("UpdatedTotalTime")),
                          static_cast<u32>(mPlayTime + static_cast<f32>(mSavedPlayTime)));
        }

        if (info.getValue(SaveDataInfo::cValueIndex_SleepTime).isValid()) {
            data->addData(KeyString(sead::SafeString("SleepTotalTime")),
                          static_cast<u32>(mSleepTime + mSavedSleepTime));
        }

        for (s32 i = 0; i < cPlayerNumMax; i++) {
            if (mPlayerNumTimes[i] > 0.0f) {
                KeyString key;
                key.appendWithSeadFormat("MultiPlayTime_%@", i + 1);
                data->addData(key, static_cast<u32>(mPlayerNumTimes[i]));
            }

            if (mSavedPlayerNumTimes[i] > 0.0f) {
                KeyString key;
                key.appendWithSeadFormat("MultiPlayTimeTotal_%@", i + 1);
                data->addData(key, static_cast<u32>(mPlayerNumTimes[i] + mSavedPlayerNumTimes[i]));
            }
        }
    }

    return data->requestSave();
}

}  // namespace erepo
