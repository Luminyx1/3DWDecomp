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

void PlayTimeObserver::load()
{
    Manager* manager = Manager::instance();
    if (!manager) {
        return;
    }
    SaveDataInfo info = manager->getSaveDataInfo();
    if (info.mValues[3].isValid()) {
        mSavedPlayTime = info.mValues[3].get();
    }
    if (info.mValues[0].isValid()) {
        mSavedActiveTime = info.mValues[0].get();
    }
    if (info.mValues[2].isValid()) {
        mSavedSleepTime = info.mValues[2].get();
    }
    for (s32 i = 0; i < cPlayerNumMax; i++) {
        mSavedPlayerNumTimes[i] = info.mFloats[i].isValid() ? info.mFloats[i].get() : 0.0f;
    }
}

/**
 * Stores the total play times into the save data info.
 * @param pData Unused.
 */
void PlayTimeObserver::save(SaveData* pData) const
{
    Manager* manager = Manager::instance();
    if (!manager) {
        return;
    }
    SaveDataInfo& info = manager->getSaveDataInfo();
    info.mValues[3].set(static_cast<u32>(static_cast<f32>(mSavedPlayTime) + mPlayTime));
    info.mValues[0].set(mSavedActiveTime + mActiveTime);
    info.mValues[2].set(mSavedSleepTime + mSleepTime);
    for (s32 i = 0; i < cPlayerNumMax; i++) {
        info.mFloats[i].set(mPlayerNumTimes[i] + mSavedPlayerNumTimes[i]);
    }
}

void PlayTimeObserver::update(const Manager::UpdateArg& rArg)
{
    mPlayTime = rArg.deltaTime + mPlayTime;
    if (mBeaconIndex == 0 || mPlayTime > static_cast<f32>(mNextBeaconTime)) {
        sendActiveBeacon_();
        mBeaconIndex = mBeaconIndex + 1 >= 5 ? 5 : mBeaconIndex + 1;
        mNextBeaconTime =
            static_cast<u32>(mPlayTime + static_cast<f32>(cBeaconIntervals[mBeaconIndex]));
    }

    s32 num = sead::ControllerMgr::instance()->getControllerNum();
    if (num > 0) {
        num = sead::Mathi::min(num, cPlayerNumMax);
        for (s32 i = 0; i < num; i++) {
            sead::Controller* controller = sead::ControllerMgr::instance()->getController(i);
            if (!controller || !controller->isConnected()) {
                mControllerActiveTimes[i] = 0;
                continue;
            }
            if (controller->getTrigMask() != 0 || controller->getHoldMask() != 0) {
                mControllerActiveTimes[i] = 30;
                continue;
            }
            if (controller->getLeftStick().x != 0.0f || controller->getLeftStick().y != 0.0f ||
                controller->getRightStick().x != 0.0f || controller->getRightStick().y != 0.0f) {
                mControllerActiveTimes[i] = 30;
            }
        }
    }

    s32 activeNum = 0;
    for (s32 i = 0; i < cPlayerNumMax; i++) {
        mControllerActiveTimes[i] -= rArg.deltaTime;
        if (mControllerActiveTimes[i] != 0) {
            activeNum++;
        }
    }
    const s32 index = activeNum - 1;
    if (static_cast<u32>(index) >= 4) {
        return;
    }
    mPlayerNumTimes[index] += rArg.deltaTime;
}

/**
 * Sends the active beacon for the current interval.
 * @return Whether the data was queued.
 */
bool PlayTimeObserver::sendActiveBeacon_()
{
    SendData* data =
        createSendData_(sead::SafeString("erepo_active_beacon"), 2, 0, 0, StringId(), false);
    if (!data) {
        return false;
    }
    data->addData(sead::FixedSafeString<63>(sead::SafeString("IntervalTime")),
                  cBeaconIntervals[mBeaconIndex]);
    data->addData(sead::FixedSafeString<63>(sead::SafeString("UpdatedTime")),
                  static_cast<u32>(mPlayTime));
    return data->requestSave();
}

bool PlayTimeObserver::report(const StringId& rId)
{
    SendData* data = createSendData_(sead::SafeString("erepo_time"), 16, 0, 0, rId, true);
    if (!data) {
        return false;
    }

    mActiveTime = nn::oe::GetProgramTotalActiveTime().GetSeconds();
    data->addData(sead::FixedSafeString<63>(sead::SafeString("ActiveTime")), mActiveTime);
    data->addData(sead::FixedSafeString<63>(sead::SafeString("UpdatedTime")),
                  static_cast<u32>(mPlayTime));
    data->addData(sead::FixedSafeString<63>(sead::SafeString("SleepTime")),
                  static_cast<u32>(mSleepTime));

    Manager* manager = Manager::instance();
    if (manager) {
        const SaveDataInfo& info = manager->getSaveDataInfo();
        if (info.mValues[0].isValid()) {
            data->addData(sead::FixedSafeString<63>(sead::SafeString("ActiveTotalTime")),
                          mActiveTime + mSavedActiveTime);
        }
        if (info.mValues[3].isValid()) {
            data->addData(sead::FixedSafeString<63>(sead::SafeString("UpdatedTotalTime")),
                          static_cast<u32>(mPlayTime + static_cast<f32>(mSavedPlayTime)));
        }
        if (info.mValues[2].isValid()) {
            data->addData(sead::FixedSafeString<63>(sead::SafeString("SleepTotalTime")),
                          static_cast<u32>(mSleepTime + mSavedSleepTime));
        }

        for (s32 i = 0; i < cPlayerNumMax; i++) {
            if (mPlayerNumTimes[i] > 0.0f) {
                sead::FixedSafeString<63> key;
                key.appendWithSeadFormat("MultiPlayTime_%@", i + 1);
                data->addData(key, static_cast<u32>(mPlayerNumTimes[i]));
            }
            if (mSavedPlayerNumTimes[i] > 0.0f) {
                sead::FixedSafeString<63> key;
                key.appendWithSeadFormat("MultiPlayTimeTotal_%@", i + 1);
                data->addData(key, static_cast<u32>(mPlayerNumTimes[i] + mSavedPlayerNumTimes[i]));
            }
        }
    }
    return data->requestSave();
}

}  // namespace erepo
