#include <erepo/Manager.h>

#include <heap/seadExpHeap.h>
#include <nn/time.h>
#include <thread/seadThread.h>

#include <erepo/Data/SendThread.h>
#include <erepo/NetworkStatusObserver.h>
#include <erepo/PlayStyleObserver.h>
#include <erepo/PlayTimeObserver.h>
#include <erepo/Reporter.h>
#include <erepo/UserInfoObserver.h>

namespace erepo {

namespace {
void deleteReporters(Manager::ReporterArray& rReporters)
{
    if (rReporters.data() == nullptr) {
        return;
    }

    Reporter** end = rReporters.dataEnd();

    for (Reporter** it = rReporters.dataBegin(); it != end; ++it) {
        delete *it;
    }

    rReporters.freeBuffer();
}

sead::DateTime sLaunchTime(0);
s32 sUpdateCounter;
}  // namespace

SEAD_SINGLETON_DISPOSER_IMPL(Manager)

/**
 * Constructs the manager with empty reporter lists and no send thread.
 */
Manager::Manager() = default;

/**
 * Creates the reporter heap, the send thread and all reporters.
 * @param rArg Heap, thread and user settings.
 * @return Whether initialization happened (false if already initialized).
 */
bool Manager::initialize(const InitializeArg& rArg)
{
    if (mFlags.isOn(EFlag::cInitialized)) {
        return false;
    }

    if (!nn::time::IsInitialized()) {
        nn::time::Initialize();
    }

    mHeap = sead::ExpHeap::create(rArg.heapSize, "PlayReporter", rArg.heap, 8,
                                  sead::Heap::cHeapDirection_Forward, false);
    mHeap->enableLock(true);

    auto* sendThread =
        new (rArg.heap) SendThread(rArg.heap, rArg.threadPriority, rArg.sendQueueSize,
                                   rArg.threadStackSize, rArg.threadMessageQueueSize);
    mSendThread = sendThread;
    sendThread->setAffinity(rArg.threadAffinity);
    mSendThread->start();

    mReporterLists[0].type = ReporterType::cSystem;
    mReporterLists[0].reporters.allocBuffer(1, rArg.heap);
    {
        auto* reporter = new (rArg.heap) Reporter(new (rArg.heap) UserInfoObserver());

        if (reporter != nullptr) {
            reporter->setTimingOn(Reporter::ETiming::cStartup);
            mReporterLists[0].reporters.pushBack(reporter);
        }
    }

    mReporterLists[1].type = ReporterType::cGame;
    mReporterLists[1].reporters.allocBuffer(4, rArg.heap);
    {
        auto* reporter = new (rArg.heap) Reporter(new (rArg.heap) PlayTimeObserver());

        if (reporter != nullptr) {
            reporter->setTimingOn(Reporter::ETiming::cDaily);
            reporter->setTimingOn(Reporter::ETiming::cStartup);
            mReporterLists[1].reporters.pushBack(reporter);
        }
    }

    {
        auto* reporter = new (rArg.heap) Reporter(new (rArg.heap) PlayStyleObserver());

        if (reporter != nullptr) {
            reporter->setTimingOn(Reporter::ETiming::cDaily);
            reporter->setTimingOn(Reporter::ETiming::cStartup);
            mReporterLists[1].reporters.pushBack(reporter);
        }
    }

    mNetworkStatusObserver = new (rArg.heap) NetworkStatusObserver();
    {
        auto* reporter = new (rArg.heap) Reporter(mNetworkStatusObserver);

        if (reporter != nullptr) {
            reporter->setTimingOn(Reporter::ETiming::cStartup);
            mReporterLists[1].reporters.pushBack(reporter);
        }
    }

    mUid = rArg.uid;

    for (auto& reporter : mReporterLists[0].reporters) {
        reporter.initialize(rArg.heap);
    }

    for (auto& reporter : mReporterLists[1].reporters) {
        reporter.initialize(rArg.heap);
    }

    if (rArg.isSystemReportTagEnabled) {
        mFlags.setOn(EFlag::cSystemReportTag);
    } else {
        mFlags.setOff(EFlag::cSystemReportTag);
    }

    mFlags.setOn(EFlag::cInitialized);
    return true;
}

/**
 * Applies the startup settings and requests the startup report and initial
 * load.
 * @param rArg Application ids, name/versions and option flags.
 * @return Whether the manager was initialized.
 */
bool Manager::startUp(const StartupArg& rArg)
{
    if (rArg.isSaveLoadEnabled) {
        mFlags.setOn(EFlag::cEnableSaveLoad);
    } else {
        mFlags.setOff(EFlag::cEnableSaveLoad);
    }

    if (rArg.isLogEnabled) {
        mFlags.setOn(EFlag::cEnableLog);
    } else {
        mFlags.setOff(EFlag::cEnableLog);
    }

    if (!mFlags.isOn(EFlag::cInitialized)) {
        return false;
    }

    std::atomic<u32>& flags = mFlags.getRaw();
    u32 expected = flags.load(std::memory_order_relaxed);

    while (!flags.compare_exchange_weak(
        expected,
        (expected & ~((1u << EFlag::cStartupRequested) | (1u << EFlag::cStartupFinished))) |
            (1u << EFlag::cStartupRequested),
        std::memory_order_relaxed)) {
    }

    mReporterLists[0].setting = new (mHeap) ReporterSetting();
    {
        ReporterSetting::Arg arg;
        arg.applicationId = rArg.applicationIds[0];
        arg.name = rArg.name;
        arg.version = rArg.versions[0];
        mReporterLists[0].setting->set(arg);
    }

    mReporterLists[1].setting = new (mHeap) ReporterSetting();
    {
        ReporterSetting::Arg arg;
        arg.applicationId = rArg.applicationIds[1];
        arg.name = rArg.name;
        arg.version = rArg.versions[1];
        mReporterLists[1].setting->set(arg);
    }

    requestLoadData();
    return true;
}

/**
 * Requests loading the save data if no save/load is in progress.
 * @return Whether the request was accepted.
 */
bool Manager::requestLoadData()
{
    if (mSaveLoadState != cSaveLoadState_None) {
        return false;
    }

    mSaveLoadState = cSaveLoadState_Load;
    mSaveLoadType = 0;
    return true;
}

/**
 * Stores values loaded by the game into the save data info.
 * @param value0 First value.
 * @param value1 Time of the last daily report.
 * @param value2 Third value.
 * @param value3 Fourth value.
 * @param pTimes0 Five play style times for the first play style.
 * @param pTimes1 Five play style times for the second play style.
 * @param pTimes2 Five play style times for the third play style.
 * @param pFloats Four float values.
 */
void Manager::loadData(u32 value0, u32 value1, u32 value2, u32 value3, u32* pTimes0, u32* pTimes1,
                       u32* pTimes2, f32* pFloats)
{
    if (mSaveLoadState != cSaveLoadState_Load) {
        return;
    }

    mSaveDataInfo.getValue(SaveDataInfo::cValueIndex_ActiveTime).set(value0);
    mSaveDataInfo.getValue(SaveDataInfo::cValueIndex_LastDailyReportTime).set(value1);
    mSaveDataInfo.getValue(SaveDataInfo::cValueIndex_SleepTime).set(value2);
    mSaveDataInfo.getValue(SaveDataInfo::cValueIndex_PlayTime).set(value3);

    for (s32 i = 0; i < SaveDataInfo::cPlayStyleTimeNum; i++) {
        mSaveDataInfo.getPlayStyleTime(0, i).set(pTimes0[i]);
        mSaveDataInfo.getPlayStyleTime(1, i).set(pTimes1[i]);
        mSaveDataInfo.getPlayStyleTime(2, i).set(pTimes2[i]);
    }

    for (s32 i = 0; i < 4; i++) {
        mSaveDataInfo.getFloat(i).set(pFloats[i]);
    }
}

/**
 * Lets every reporter load from the save data info and finishes loading.
 */
void Manager::setLoadFinish()
{
    if (mSaveLoadState != cSaveLoadState_Load) {
        return;
    }

    for (mSaveLoadType = 0; static_cast<s32>(mSaveLoadType) != ReporterType::size();
         mSaveLoadType = mSaveLoadType + 1) {
        for (auto& reporter : mReporterLists[mSaveLoadType].reporters) {
            reporter.load();
        }
    }

    mSaveLoadState = cSaveLoadState_Finish;
}

/**
 * Writes the valid save data info values into the game's buffers.
 * @param pValue0 First value.
 * @param pValue1 Time of the last daily report.
 * @param pValue2 Third value.
 * @param pValue3 Fourth value.
 * @param pTimes0 Five play style times for the first play style.
 * @param pTimes1 Five play style times for the second play style.
 * @param pTimes2 Five play style times for the third play style.
 * @param pFloats Four float values.
 */
void Manager::saveData(u32* pValue0, u32* pValue1, u32* pValue2, u32* pValue3, u32* pTimes0,
                       u32* pTimes1, u32* pTimes2, f32* pFloats)
{
    mSaveDataInfo.getValue(SaveDataInfo::cValueIndex_ActiveTime).tryGet(pValue0);
    mSaveDataInfo.getValue(SaveDataInfo::cValueIndex_LastDailyReportTime).tryGet(pValue1);
    mSaveDataInfo.getValue(SaveDataInfo::cValueIndex_SleepTime).tryGet(pValue2);
    mSaveDataInfo.getValue(SaveDataInfo::cValueIndex_PlayTime).tryGet(pValue3);

    for (s32 i = 0; i < SaveDataInfo::cPlayStyleTimeNum; i++) {
        mSaveDataInfo.getPlayStyleTime(0, i).tryGet(&pTimes0[i]);
        mSaveDataInfo.getPlayStyleTime(1, i).tryGet(&pTimes1[i]);
        mSaveDataInfo.getPlayStyleTime(2, i).tryGet(&pTimes2[i]);
    }

    for (s32 i = 0; i < 4; i++) {
        mSaveDataInfo.getFloat(i).tryGet(&pFloats[i]);
    }
}

/**
 * Marks a pending save as finished.
 */
void Manager::setSaveFinish()
{
    if (mSaveLoadState == cSaveLoadState_Save) {
        mSaveLoadState = cSaveLoadState_Finish;
    }
}

/**
 * Queues send data on the send thread.
 * @param pData Data to send.
 * @return Whether the data was queued.
 */
bool Manager::requestSend_(SendDataBase* pData)
{
    return mSendThread->requestSave(pData);
}

/**
 * Lets every reporter save into the save data info and requests a game save.
 * @return Whether the request was accepted.
 */
bool Manager::requestSaveData()
{
    if (mSaveLoadState != cSaveLoadState_None) {
        return false;
    }

    for (mSaveLoadType = 0; static_cast<s32>(mSaveLoadType) != ReporterType::size();
         mSaveLoadType = mSaveLoadType + 1) {
        for (auto& reporter : mReporterLists[mSaveLoadType].reporters) {
            reporter.save(nullptr);
        }
    }

    mSaveLoadState = cSaveLoadState_Save;
    mSaveLoadType = 0;
    return true;
}

/**
 * Advances startup, reporters and the send thread by one frame.
 * @param rArg Frame time and system message.
 */
void Manager::update(const UpdateArg& rArg)
{
    if (!mFlags.isOn(EFlag::cStartupFinished)) {
        if (mFlags.isOn(EFlag::cStartupRequested)) {
            updateBeginStartup_();
        }

        return;
    }

    if (mFlags.isOn(EFlag::cWaitFinishSendingAsync)) {
        waitFinishSendingAsync_(mWaitTimeoutMs);
    }

    if (!mFlags.isOn(EFlag::cWaitFinishSending) && !mFlags.isOn(EFlag::cWaitFinishSendingAsync)) {
        updateFinishStartup_(rArg);
    }

    if (mFlags.isOn(EFlag::cRequestImmediateTransmission)) {
        mSendThread->sendMessage(SendThread::cMsgImmediateTransmission,
                                 sead::MessageQueue::BlockType::NonBlocking);
        mFlags.setOff(EFlag::cRequestImmediateTransmission);
    }

    mSendThread->sendMessage(SendThread::cMsgSave, sead::MessageQueue::BlockType::NonBlocking);
}

void Manager::updateBeginStartup_()
{
    switch (mSaveLoadState) {
    case cSaveLoadState_Load:
        updateLoadData_();
        break;
    case cSaveLoadState_Save:
        updateSaveData_();
        break;
    case cSaveLoadState_Finish:
        mSaveLoadState = cSaveLoadState_None;
        [[fallthrough]];
    case cSaveLoadState_None: {
        sendStartupReport_(mReporterLists[0].type);
        sendStartupReport_(mReporterLists[1].type);

        std::atomic<u32>& flags = mFlags.getRaw();
        u32 expected = flags.load(std::memory_order_relaxed);

        while (!flags.compare_exchange_weak(
            expected,
            (expected & ~((1u << EFlag::cStartupRequested) | (1u << EFlag::cStartupFinished))) |
                (1u << EFlag::cStartupFinished),
            std::memory_order_relaxed)) {
        }

        break;
    }
    }
}

/**
 * Stops waiting for pending sends once the timeout expired.
 * @param timeoutMs Timeout in milliseconds (negative waits forever).
 * @return Whether the wait is still active.
 */
bool Manager::waitFinishSendingAsync_(s32 timeoutMs)
{
    s64 elapsed = mWaitStartTime.diffToNow().toMilliSeconds();

    if (timeoutMs >= 0 && elapsed > timeoutMs) {
        mFlags.setOff(EFlag::cWaitFinishSendingAsync);
        return false;
    }

    return true;
}

void Manager::updateFinishStartup_(const UpdateArg& rArg)
{
    s32 counter = sUpdateCounter;
    sUpdateCounter = counter + 1;

    if (counter % 3600 == 0 && checkSendDailyReport_()) {
        for (auto& reporter : mReporterLists[0].reporters) {
            reporter.update(rArg);
        }

        sendDailyReport_(mReporterLists[0].type);

        if (mReporterLists[0].requestFlag.testAndClear(ReporterType::cSystem)) {
            for (auto& reporter : mReporterLists[0].reporters) {
                reporter.report(mReporterLists[0].requestId);
            }

            requestSaveData();
        }

        for (auto& reporter : mReporterLists[1].reporters) {
            reporter.update(rArg);
        }

        sendDailyReport_(mReporterLists[1].type);

        if (mReporterLists[1].requestFlag.testAndClear(ReporterType::cSystem)) {
            for (auto& reporter : mReporterLists[1].reporters) {
                reporter.report(mReporterLists[1].requestId);
            }

            requestSaveData();
        }

        mSaveDataInfo.getValue(SaveDataInfo::cValueIndex_LastDailyReportTime).set(getCurrentDateTime());
        requestSaveData();
    } else {
        for (auto& reporter : mReporterLists[0].reporters) {
            reporter.update(rArg);
        }

        if (mReporterLists[0].requestFlag.testAndClear(ReporterType::cSystem)) {
            for (auto& reporter : mReporterLists[0].reporters) {
                reporter.report(mReporterLists[0].requestId);
            }

            requestSaveData();
        }

        for (auto& reporter : mReporterLists[1].reporters) {
            reporter.update(rArg);
        }

        if (mReporterLists[1].requestFlag.testAndClear(ReporterType::cSystem)) {
            for (auto& reporter : mReporterLists[1].reporters) {
                reporter.report(mReporterLists[1].requestId);
            }

            requestSaveData();
        }
    }

    updateSaveLoad_();
}

/**
 * Finishes a pending save or load when allowed and resets a finished one.
 */
void Manager::updateSaveLoad_()
{
    switch (mSaveLoadState) {
    case cSaveLoadState_Load:
        if (mFlags.isOn(EFlag::cEnableSaveLoad)) {
            mSaveLoadState = cSaveLoadState_Finish;
        }

        break;
    case cSaveLoadState_Save:
        updateSaveData_();
        break;
    case cSaveLoadState_Finish:
        mSaveLoadState = cSaveLoadState_None;
        break;
    }
}

/**
 * Sends the startup report of every reporter of a list.
 * @param type Reporter list.
 * @return Always true.
 */
bool Manager::sendStartupReport_(ReporterType type)
{
    for (auto& reporter : mReporterLists[type].reporters) {
        reporter.report(StringId(Reporter::cStartupReportId));
    }

    return true;
}

/**
 * Checks whether a day passed since the last daily report.
 * @return Whether the daily report is due.
 */
bool Manager::checkSendDailyReport_()
{
    if (!mSaveDataInfo.getValue(SaveDataInfo::cValueIndex_LastDailyReportTime).isValid()) {
        return false;
    }

    u32 lastTime = mSaveDataInfo.getValue(SaveDataInfo::cValueIndex_LastDailyReportTime).get();
    u32 nowTime = getCurrentDateTime();

    if (lastTime > nowTime) {
        return false;
    }

    return sead::DateSpan(nowTime - lastTime).getSpan() >= 24 * 60 * 60;
}

/**
 * Sends the daily report of every reporter of a list.
 * @param type Reporter list.
 */
void Manager::sendDailyReport_(ReporterType type)
{
    for (auto& reporter : mReporterLists[type].reporters) {
        reporter.report(StringId(Reporter::cDailyReportId));
    }
}

/**
 * Gets the current time.
 * @return Current unix time.
 */
u64 Manager::getCurrentDateTime() const
{
    sead::DateTime now(0);
    now.setNow();
    return now.getUnixTime();
}

/**
 * Destroys the reporters, the send thread and the reporter heap.
 */
void Manager::finalize()
{
    deleteReporters(mReporterLists[0].reporters);
    deleteReporters(mReporterLists[1].reporters);

    mSendThread->quitAndWaitDoneSingleThread(false);

    if (mSendThread != nullptr) {
        delete mSendThread;
        mSendThread = nullptr;
    }

    mHeap->destroy();
    mHeap = nullptr;
}

const sead::DateTime& Manager::getLaunchTime() const
{
    return sLaunchTime;
}

/**
 * Unused hook for copying game data into the save data info.
 * @return Always false.
 */
bool Manager::applySaveDataInfoFromGameData()
{
    return false;
}

/**
 * Unused hook for copying the save data info into game data.
 * @return Always false.
 */
bool Manager::applyGameDataFromSaveDataInfo()
{
    return false;
}

/**
 * Sets the controller style used when it cannot be detected.
 * @param style Controller style.
 */
void Manager::setCurrentControllerStyle(EControllerStyle style)
{
    mControllerStyle = style;
}

/**
 * Gets the fallback controller style.
 * @return Controller style.
 */
EControllerStyle Manager::getCurrentControllerStyle() const
{
    return mControllerStyle;
}

/**
 * Finishes a pending load when save/load is enabled.
 */
void Manager::updateLoadData_()
{
    if (mFlags.isOn(EFlag::cEnableSaveLoad)) {
        mSaveLoadState = cSaveLoadState_Finish;
    }
}

/**
 * Finishes a pending save when save/load is enabled.
 */
void Manager::updateSaveData_()
{
    if (mFlags.isOn(EFlag::cEnableSaveLoad)) {
        mSaveLoadState = cSaveLoadState_Finish;
    }
}

/**
 * Does nothing.
 */
void Manager::resetSaveState_() {}

/**
 * Starts waiting for pending sends across frames.
 * @param timeoutMs Timeout in milliseconds (negative waits forever).
 * @return Whether a new wait was started.
 */
bool Manager::requestWaitFinishSendingAsync(s32 timeoutMs)
{
    mWaitTimeoutMs = timeoutMs;

    if (mFlags.isOn(EFlag::cWaitFinishSendingAsync)) {
        return false;
    }

    mFlags.setOn(EFlag::cWaitFinishSendingAsync);
    mWaitStartTime.setNow();
    return true;
}

/**
 * Checks whether the send thread is sending.
 * @return Whether data is being sent.
 */
bool Manager::isSending() const
{
    if (mSendThread == nullptr) {
        return false;
    }

    return mSendThread->isSending();
}

/**
 * Checks whether the calling thread is the send thread.
 * @return Whether the current thread is the send thread.
 */
bool Manager::isSendThread() const
{
    if (mSendThread == nullptr) {
        return false;
    }

    sead::Thread* thread = sead::ThreadMgr::instance()->getCurrentThread();
    return (thread != nullptr) && thread->getId() == mSendThread->getId();
}

/**
 * Blocks until the send thread finished sending or the timeout expired.
 * @param timeoutMs Timeout in milliseconds (negative waits forever).
 * @return Whether sending finished.
 */
bool Manager::waitFinishSending(s32 timeoutMs)
{
    mFlags.setOn(EFlag::cWaitFinishSending);
    mWaitStartTime.setNow();

    while (mSendThread != nullptr) {
        if (!mSendThread->isSending()) {
            return true;
        }

        s64 elapsed = mWaitStartTime.diffToNow().toMilliSeconds();

        if (timeoutMs >= 0 && elapsed > timeoutMs) {
            mFlags.setOff(EFlag::cWaitFinishSending);
            return false;
        }

        if (mSendThread->getQueuedNum() <= 0) {
            mSendThread->sendMessage(SendThread::cMsgSave,
                                     sead::MessageQueue::BlockType::NonBlocking);
        }

        sead::Thread::sleep(sead::TickSpan(sead::TickSpan::getFrequency() / 2));
    }

    return true;
}

}  // namespace erepo
