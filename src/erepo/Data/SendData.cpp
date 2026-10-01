#include <erepo/Data/SendData.h>

#include <basis/seadNew.h>
#include <erepo/Data/Array.h>
#include <erepo/Data/Struct.h>
#include <erepo/Manager.h>
#include <heap/seadHeap.h>
#include <nn/prepo.h>

namespace erepo {

/**
 * Constructs send data backed by a play report and sets its event id.
 * @param pHeap Heap to allocate from.
 * @param rEventId Event id.
 * @param dataNum Maximum number of values.
 * @param arrayNum Number of array slots.
 * @param structNum Number of struct slots.
 * @param rReporterId Id of the reporter creating the data.
 * @param isNothrow Whether to use the nothrow allocator.
 */
SendData::SendData(sead::Heap* pHeap, const EventIdString& rEventId, s32 dataNum,
                   s32 arrayNum, s32 structNum, const StringId& rReporterId, bool isNothrow)
    : SendDataBase(pHeap, rEventId, rReporterId, isNothrow)
{
    initialize_(dataNum, arrayNum, structNum, pHeap);
    setEventId_(rEventId);
}

/**
 * Frees the play report and all owned data.
 */
SendData::~SendData()
{
    finalize_();
}

/**
 * Creates send data on the manager heap.
 * @param rEventId Event id.
 * @param dataNum Maximum number of values.
 * @param arrayNum Number of array slots.
 * @param structNum Number of struct slots.
 * @param rReporterId Id of the reporter creating the data.
 * @param isNothrow Whether to use the nothrow allocator.
 * @return The send data, or nullptr on failure.
 */
SendData* SendData::CreateSendData(const EventIdString& rEventId, s32 dataNum,
                                   s32 arrayNum, s32 structNum, const StringId& rReporterId,
                                   bool isNothrow)
{
    Manager* pManager = Manager::instance();

    if (pManager == nullptr) {
        return nullptr;
    }

    sead::Heap* pHeap = pManager->getHeap();

    if (pHeap == nullptr) {
        return nullptr;
    }

    SendData* pSendData;

    if (isNothrow) {
        pSendData = new (pHeap, std::nothrow)
            SendData(pHeap, rEventId, dataNum, arrayNum, structNum, rReporterId, isNothrow);
    } else {
        pSendData = new (pHeap)
            SendData(pHeap, rEventId, dataNum, arrayNum, structNum, rReporterId, isNothrow);
    }

    if (pSendData == nullptr) {
        return nullptr;
    }

    if (!pSendData->isValid_()) {
        delete pSendData;
        return nullptr;
    }

    return pSendData;
}

/**
 * Creates send data on the manager heap.
 * @param rArg Creation parameters.
 * @return The send data, or nullptr on failure.
 */
SendData* SendData::CreateSendData(const CreateArg& rArg)
{
    return CreateSendData(rArg.eventId, rArg.dataNum, rArg.arrayNum, rArg.structNum,
                          rArg.reporterId, rArg.isNothrow);
}

/**
 * Adds a bool value to the report.
 * @param rKey Value key.
 * @param value Value.
 * @return Whether the value was added.
 */
bool SendData::addData(const KeyString& rKey, bool value)
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(1)) {
        return false;
    }

    addLog(rKey, value);
    return mReport->Add(rKey.cstr(), value).IsSuccess();
}

/**
 * Adds a u64 value to the report.
 * @param rKey Value key.
 * @param value Value.
 * @return Whether the value was added.
 */
bool SendData::addData(const KeyString& rKey, u64 value)
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(1)) {
        return false;
    }

    addLog(rKey, value);
    nn::prepo::Any64BitId id = {value};
    return mReport->Add(rKey.cstr(), id).IsSuccess();
}

/**
 * Adds a s64 value to the report.
 * @param rKey Value key.
 * @param value Value.
 * @return Whether the value was added.
 */
bool SendData::addData(const KeyString& rKey, s64 value)
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(1)) {
        return false;
    }

    addLog(rKey, value);
    return mReport->Add(rKey.cstr(), value).IsSuccess();
}

/**
 * Adds a u32 value to the report.
 * @param rKey Value key.
 * @param value Value.
 * @return Whether the value was added.
 */
bool SendData::addData(const KeyString& rKey, u32 value)
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(1)) {
        return false;
    }

    addLog(rKey, value);
    return mReport->Add(rKey.cstr(), static_cast<s64>(value)).IsSuccess();
}

/**
 * Adds a s32 value to the report.
 * @param rKey Value key.
 * @param value Value.
 * @return Whether the value was added.
 */
bool SendData::addData(const KeyString& rKey, s32 value)
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(1)) {
        return false;
    }

    addLog(rKey, value);
    return mReport->Add(rKey.cstr(), static_cast<s64>(value)).IsSuccess();
}

/**
 * Adds a f32 value to the report.
 * @param rKey Value key.
 * @param value Value.
 * @return Whether the value was added.
 */
bool SendData::addData(const KeyString& rKey, f32 value)
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(1)) {
        return false;
    }

    addLog(rKey, value);
    return mReport->Add(rKey.cstr(), value).IsSuccess();
}

/**
 * Adds a string value to the report.
 * @param rKey Value key.
 * @param value Value.
 * @return Whether the value was added.
 */
bool SendData::addData(const KeyString& rKey, const char* value)
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(1)) {
        return false;
    }

    addLog(rKey, value);
    return mReport->Add(rKey.cstr(), value).IsSuccess();
}

/**
 * Adds a string value to the report.
 * @param rKey Value key.
 * @param rValue Value.
 * @return Whether the value was added.
 */
bool SendData::addData(const KeyString& rKey, const sead::SafeString& rValue)
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(1)) {
        return false;
    }

    addLog(rKey, rValue);
    return mReport->Add(rKey.cstr(), rValue.cstr()).IsSuccess();
}

/**
 * Adds a binary value to the report.
 * @param rKey Value key.
 * @param pValue Value.
 * @param size Size of the value in bytes.
 * @return Whether the value was added.
 */
bool SendData::addData(const KeyString& rKey, const void* pValue, size_t size)
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(1)) {
        return false;
    }

    addLog(rKey, pValue, size);
    return mReport->Add(rKey.cstr(), pValue, size).IsSuccess();
}

/**
 * Adds a array value to the report.
 * @param rKey Value key.
 * @param rValue Value.
 * @return Whether the value was added.
 */
bool SendData::addData(const KeyString& rKey, const Array& rValue)
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(1)) {
        return false;
    }

    addLog(rKey, rValue);
    return mReport->Add(rKey.cstr(), rValue.getArray()).IsSuccess();
}

/**
 * Adds a struct value to the report.
 * @param rKey Value key.
 * @param rValue Value.
 * @return Whether the value was added.
 */
bool SendData::addData(const KeyString& rKey, const Struct& rValue)
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(1)) {
        return false;
    }

    addLog(rKey, rValue);
    return mReport->Add(rKey.cstr(), rValue.getStruct()).IsSuccess();
}

/**
 * Adds the internet connection status values to the report.
 * @return Whether the values were added.
 */
bool SendData::addInternetConnectionStatus()
{
    if (!isInitialized()) {
        return false;
    }

    if (!addSendDataNum_(3)) {
        return false;
    }

    return nn::prepo::AddInternetConnectionStatus(mReport, "NetworkInterface", "LinkLevel",
                                                  "FrequencyBand")
        .IsSuccess();
}

/**
 * Gets the size of the report data.
 * @return The size in bytes, or 0 without a report.
 */
size_t SendData::GetSize() const
{
    if (mReport == nullptr) {
        return 0;
    }

    return mReport->GetSize();
}

/**
 * Gets the number of values in the report.
 * @return The value count, or -1 without a report.
 */
s32 SendData::GetCount() const
{
    if (mReport == nullptr) {
        return -1;
    }

    return mReport->GetCount();
}

/**
 * Removes all values from the report.
 * @return Whether the report was cleared.
 */
bool SendData::clear()
{
    if (!isInitialized()) {
        return false;
    }

    clearLog_();
    mReport->Clear();
    return true;
}

/**
 * Adds the session id to the report.
 * @return The prepo result.
 */
nn::Result SendData::addSessionId()
{
    return nn::prepo::AddSessionId(mReport, "session_id");
}

/**
 * Checks whether the report and its buffer are ready.
 * @return Whether the data can be used.
 */
bool SendData::isValid_() const
{
    if (!isInitialized()) {
        return false;
    }

    if (mReport == nullptr) {
        return false;
    }

    if (!mReportBuffer.isBufferReady()) {
        return false;
    }

    if (mReportBuffer.size() < 1) {
        return false;
    }

    return mReport->GetSize() != 0;
}

/**
 * Allocates the play report and its buffer.
 * @param pHeap Heap to allocate from.
 * @return Whether the report was allocated.
 */
bool SendData::initializeInnerData_(sead::Heap* pHeap)
{
    if (mIsNothrow) {
        mReport = new (pHeap, std::nothrow) nn::prepo::PlayReport();
    } else {
        mReport = new (pHeap) nn::prepo::PlayReport();
    }

    if (mReport == nullptr) {
        sead::FormatFixedSafeString<128> message(
            "メモリ不足で内部送信データが作成できませんでした[allocatable size : %u]",
            pHeap->getMaxAllocatableSize(sizeof(void*)));
        return false;
    }

    const size_t size = nn::prepo::PlayReport::CalcBufferSize(mDataNum + 1);

    if (size > 0x4000) {
        return false;
    }

    if (!mReportBuffer.tryAllocBuffer(size, pHeap)) {
        return false;
    }

    mReport->SetBuffer(mReportBuffer.getBufferPtr(), size);
    return true;
}

/**
 * Frees the play report and its buffer.
 */
void SendData::finalizeInnerData_()
{
    if (mReport != nullptr) {
        delete mReport;
        mReport = nullptr;
    }

    mReportBuffer.freeBuffer();
}

/**
 * Sets the event id on the play report.
 * @param rEventId Event id.
 * @return Whether the event id was accepted.
 */
bool SendData::trySetInnerDataEventId_(const EventIdString& rEventId)
{
    return mReport->SetEventId(rEventId.cstr()).IsSuccess();
}

/**
 * Saves the play report, for the stored user if there is one.
 * @return cSuccess or cFailure.
 */
SendDataBase::ESendResult SendData::saveInnerData_()
{
    nn::Result result;

    if (mUid.IsValid()) {
        result = mReport->Save(mUid);
    } else {
        result = mReport->Save();
    }

    return result.IsSuccess() ? ESendResult::cSuccess : ESendResult::cFailure;
}

/**
 * Creates an array of numeric elements and stores it in a free slot.
 * @param num Number of elements.
 * @return The array, or nullptr on failure.
 */
template <typename T>
Array* SendDataBase::CreateArray(s32 num)
{
    sead::Heap* pHeap = Manager::instance()->getHeap();
    Array* pArray;

    if (mIsNothrow) {
        pArray = new (pHeap, std::nothrow) Array();
    } else {
        pArray = new (pHeap) Array();
    }

    if (!pArray) {
        sead::FormatFixedSafeString<128> message(
            "メモリ不足で配列データが作成できませんでした[allocatable size : %u]",
            pHeap->getMaxAllocatableSize(sizeof(void*)));
        return nullptr;
    }

    if (pArray->constructBuffer<T>(num, pHeap)) {
        if (Array** pSlot = findEmptySlot_(mArrays)) {
            *pSlot = pArray;
            return pArray;
        }
    }

    delete pArray;
    return nullptr;
}

/**
 * Creates an array of string elements and stores it in a free slot.
 * @param num Number of elements.
 * @param length Buffer size per element.
 * @return The array, or nullptr on failure.
 */
template <typename T>
Array* SendDataBase::CreateArray(s32 num, s32 length)
{
    sead::Heap* pHeap = Manager::instance()->getHeap();
    Array* pArray;

    if (mIsNothrow) {
        pArray = new (pHeap, std::nothrow) Array();
    } else {
        pArray = new (pHeap) Array();
    }

    if (!pArray) {
        sead::FormatFixedSafeString<128> message(
            "メモリ不足で配列データが作成できませんでした[allocatable size : %u]",
            pHeap->getMaxAllocatableSize(sizeof(void*)));
        return nullptr;
    }

    if (pArray->constructBuffer<T>(num, length, pHeap)) {
        if (Array** pSlot = findEmptySlot_(mArrays)) {
            *pSlot = pArray;
            return pArray;
        }
    }

    delete pArray;
    return nullptr;
}

template Array* SendDataBase::CreateArray<s32>(s32 num);
template Array* SendDataBase::CreateArray<u32>(s32 num);
template Array* SendDataBase::CreateArray<s64>(s32 num);
template Array* SendDataBase::CreateArray<u64>(s32 num);
template Array* SendDataBase::CreateArray<f32>(s32 num);
template Array* SendDataBase::CreateArray<sead::SafeString>(s32 num, s32 length);

}  // namespace erepo
