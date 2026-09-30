#include <erepo/Data/SendDataBase.h>

#include <basis/seadNew.h>
#include <erepo/Data/Array.h>
#include <erepo/Data/Struct.h>
#include <erepo/Manager.h>
#include <heap/seadHeap.h>

namespace erepo {

/**
 * Counts newly added values and checks them against the value limit.
 * @param num Number of values being added.
 * @return Whether the values still fit.
 */
bool SendDataBase::addSendDataNum_(s32 num)
{
    mSendDataNum += num;
    s32 max = mDataNum;

    if (Manager::instance()->isFlagOn(Manager::EFlag::cSystemReportTag)) {
        max++;
    }

    return mSendDataNum <= max;
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::FixedSafeString<63>& rKey, bool value)
{
    addLog(sead::SafeString(rKey.cstr()), value);
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::SafeString& rKey, bool value) {}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::FixedSafeString<63>& rKey, u64 value)
{
    addLog(sead::SafeString(rKey.cstr()), value);
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::SafeString& rKey, u64 value) {}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::FixedSafeString<63>& rKey, s64 value)
{
    addLog(sead::SafeString(rKey.cstr()), value);
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::SafeString& rKey, s64 value) {}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::FixedSafeString<63>& rKey, u32 value)
{
    addLog(sead::SafeString(rKey.cstr()), value);
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::SafeString& rKey, u32 value) {}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::FixedSafeString<63>& rKey, s32 value)
{
    addLog(sead::SafeString(rKey.cstr()), value);
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::SafeString& rKey, s32 value) {}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::FixedSafeString<63>& rKey, f32 value)
{
    addLog(sead::SafeString(rKey.cstr()), value);
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::SafeString& rKey, f32 value) {}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::FixedSafeString<63>& rKey, const char* value)
{
    addLog(sead::SafeString(rKey.cstr()), value);
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param value Logged value.
 */
void SendDataBase::addLog(const sead::SafeString& rKey, const char* value) {}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param rValue Logged value.
 */
void SendDataBase::addLog(const sead::FixedSafeString<63>& rKey, const sead::SafeString& rValue)
{
    addLog(sead::SafeString(rKey.cstr()), rValue);
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param rValue Logged value.
 */
void SendDataBase::addLog(const sead::SafeString& rKey, const sead::SafeString& rValue) {}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param pValue Logged value.
 * @param size Size of the logged value.
 */
void SendDataBase::addLog(const sead::FixedSafeString<63>& rKey, const void* pValue, size_t size)
{
    addLog(sead::SafeString(rKey.cstr()), pValue, size);
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param pValue Logged value.
 * @param size Size of the logged value.
 */
void SendDataBase::addLog(const sead::SafeString& rKey, const void* pValue, size_t size) {}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param rValue Logged value.
 */
void SendDataBase::addLog(const sead::FixedSafeString<63>& rKey, const Array& rValue)
{
    addLog(sead::SafeString(rKey.cstr()), rValue);
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param rValue Logged value.
 */
void SendDataBase::addLog(const sead::SafeString& rKey, const Array& rValue) {}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param rValue Logged value.
 */
void SendDataBase::addLog(const sead::FixedSafeString<63>& rKey, const Struct& rValue)
{
    addLog(sead::SafeString(rKey.cstr()), rValue);
}

/**
 * Records a log entry for an added value (no-op in release builds).
 * @param rKey Value key.
 * @param rValue Logged value.
 */
void SendDataBase::addLog(const sead::SafeString& rKey, const Struct& rValue) {}

/**
 * Creates a struct sized by member count and stores it in a free slot.
 * @param memberNum Number of members.
 * @return The struct, or nullptr on failure.
 */
Struct* SendDataBase::CreateStruct(s32 memberNum)
{
    Struct* pStruct =
        Struct::createWithMemberNum(memberNum, Manager::instance()->getHeap(), mIsNothrow);
    if (!pStruct) {
        return nullptr;
    }

    if (!pStruct->isBufferReady()) {
        delete pStruct;
        return nullptr;
    }

    if (Struct** pSlot = findEmptySlot_(mStructs)) {
        *pSlot = pStruct;
        return pStruct;
    }

    return nullptr;
}

/**
 * Creates a struct of a given buffer size and stores it in a free slot.
 * @param bufferSize Buffer size in bytes.
 * @return The struct, or nullptr on failure.
 */
Struct* SendDataBase::CreateStructWithBufferSize(s32 bufferSize)
{
    Struct* pStruct =
        Struct::createWithBufferSize(bufferSize, Manager::instance()->getHeap(), mIsNothrow);
    if (!pStruct) {
        sead::FormatFixedSafeString<128> message("構造体データが作成できませんでした");
        return nullptr;
    }

    if (Struct** pSlot = findEmptySlot_(mStructs)) {
        *pSlot = pStruct;
        return pStruct;
    }

    return nullptr;
}

/**
 * Creates an array of structs sized by member count and stores it in a free slot.
 * @param num Number of structs.
 * @param memberNum Number of members per struct.
 * @return The array, or nullptr on failure.
 */
Array* SendDataBase::CreateStructArray(s32 num, s32 memberNum)
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
            "メモリ不足で構造体配列データが作成できませんでした[allocatable size : %u]",
            pHeap->getMaxAllocatableSize(sizeof(void*)));
    } else {
        pArray->constructStructBuffer(num, memberNum, pHeap);

        if (Array** pSlot = findEmptySlot_(mArrays)) {
            *pSlot = pArray;
            return pArray;
        }
    }

    return nullptr;
}

/**
 * Creates an array of structs of a given buffer size and stores it in a free slot.
 * @param num Number of structs.
 * @param bufferSize Buffer size per struct.
 * @return The array, or nullptr on failure.
 */
Array* SendDataBase::CreateStructArrayWithBufferSize(s32 num, s32 bufferSize)
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
            "メモリ不足で構造体配列データが作成できませんでした[allocatable size : %u]",
            pHeap->getMaxAllocatableSize(sizeof(void*)));
    } else {
        pArray->constructStructBufferWithBufferSize(num, bufferSize, pHeap);

        if (Array** pSlot = findEmptySlot_(mArrays)) {
            *pSlot = pArray;
            return pArray;
        }
    }

    return nullptr;
}

/**
 * Saves the data immediately.
 * @param pResult Receives the detailed result if not null.
 * @return Whether the data was saved.
 */
bool SendDataBase::save(ESendResult* pResult)
{
    ESendResult result = saveImpl_();

    if (pResult) {
        *pResult = result;
    }

    return result == ESendResult::cSuccess;
}

/**
 * Saves the data, adding the system report tag if enabled.
 * @return The save result.
 */
SendDataBase::ESendResult SendDataBase::saveImpl_()
{
    ESendResult result = checkCanSave_();

    if (static_cast<s32>(result) != ESendResult::cNone) {
        return result;
    }

    mFlags.setOn(EFlag::cSaving);

    if (Manager::instance()->isFlagOn(Manager::EFlag::cSystemReportTag)) {
        addData(sead::FixedSafeString<63>(sead::SafeString("system_report_tag")),
                mReporterId.getId());
    }

    result = saveInnerData_();

    const u32 savingMask = 1u << EFlag::cSaving;
    const u32 savedMask = 1u << EFlag::cSaved;
    u32 bits = mFlags.getRaw().load(std::memory_order_relaxed);

    while (!mFlags.getRaw().compare_exchange_weak(bits, (bits & ~(savingMask | savedMask)) | savedMask,
                                                  std::memory_order_relaxed)) {
    }

    return result;
}

/**
 * Hands the data to the manager for sending, deleting it if that fails.
 * @return Whether the request was accepted.
 */
bool SendDataBase::requestSave()
{
    if (mFlags.setOn(EFlag::cRequestSave) && Manager::instance()->requestSend_(this)) {
        return true;
    }

    delete this;
    return false;
}

/**
 * Dumps the data (no-op in release builds).
 */
void SendDataBase::dump() {}

/**
 * Constructs uninitialized send data.
 * @param pHeap Heap for the data (unused here).
 * @param rEventId Event id (unused here).
 * @param rReporterId Id of the reporter creating the data.
 * @param isNothrow Whether to use the nothrow allocator.
 */
SendDataBase::SendDataBase(sead::Heap* pHeap, const sead::FixedSafeString<31>& rEventId,
                           const StringId& rReporterId, bool isNothrow)
    : mIsNothrow(isNothrow), mDataNum(0), mUid(), mReporterId(rReporterId), mSendDataNum(0)
{
}

/**
 * Allocates the inner data and the array and struct slots.
 * @param dataNum Maximum number of values.
 * @param arrayNum Number of array slots.
 * @param structNum Number of struct slots.
 * @param pHeap Heap to allocate from, or nullptr for the manager heap.
 */
void SendDataBase::initialize_(s32 dataNum, s32 arrayNum, s32 structNum, sead::Heap* pHeap)
{
    if (isInitialized()) {
        return;
    }

    mDataNum = dataNum;
    mSendDataNum = 0;

    if (!pHeap) {
        pHeap = Manager::instance()->getHeap();
    }

    mUid = Manager::instance()->getUid();

    if (!initializeInnerData_(pHeap)) {
        return;
    }

    if (arrayNum > 0) {
        if (!mArrays.tryAllocBuffer(arrayNum, pHeap)) {
            return;
        }

        mArrays.fill(nullptr);
    }

    if (structNum > 0) {
        if (!mStructs.tryAllocBuffer(structNum, pHeap)) {
            return;
        }

        mStructs.fill(nullptr);
    }

    mFlags.setOn(EFlag::cInitialized);
}

/**
 * Frees the inner data and all owned arrays and structs.
 */
void SendDataBase::finalize_()
{
    if (!isInitialized()) {
        return;
    }

    finalizeInnerData_();

    if (mArrays.isBufferReady()) {
        for (auto* pArray : mArrays) {
            delete pArray;
        }

        mArrays.freeBuffer();
    }

    if (mStructs.isBufferReady()) {
        for (auto* pStruct : mStructs) {
            delete pStruct;
        }

        mStructs.freeBuffer();
    }

    mFlags.setOff(EFlag::cInitialized);
}

/**
 * Sets the lowercased event id on the inner data.
 * @param rEventId Event id.
 */
void SendDataBase::setEventId_(const sead::FixedSafeString<31>& rEventId)
{
    if (!isInitialized()) {
        return;
    }

    sead::FixedSafeString<31> eventId(rEventId);
    eventId.replaceCharList("ABCDEFGHIJKLMNOPQRSTUVWXYZ", "abcdefghijklmnopqrstuvwxyz");

    if (trySetInnerDataEventId_(eventId)) {
        mFlags.setOn(EFlag::cInitialized);
    } else {
        mFlags.setOff(EFlag::cInitialized);
    }
}

/**
 * Checks whether the data can be saved now.
 * @return cNone if it can be saved, otherwise the reason.
 */
SendDataBase::ESendResult SendDataBase::checkCanSave_() const
{
    if (!isInitialized()) {
        return ESendResult::cNotInitialized;
    }

    if (mFlags.isOn(EFlag::cSaved)) {
        return ESendResult::cBusy;
    }

    if (mFlags.isOn(EFlag::cSaving)) {
        return ESendResult::cBusy;
    }

    return ESendResult::cNone;
}

/**
 * Clears the log entries (no-op in release builds).
 */
void SendDataBase::clearLog_() {}

}  // namespace erepo
