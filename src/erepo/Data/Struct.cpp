#include <erepo/Data/Struct.h>

#include <basis/seadNew.h>

namespace erepo {

/**
 * Constructs an empty struct without a buffer.
 */
Struct::Struct() = default;

/**
 * Frees the struct buffer.
 */
Struct::~Struct()
{
    mBuffer.freeBuffer();
}

/**
 * Allocates the struct buffer and hands it to the prepo struct.
 * @param size Buffer size in bytes.
 * @param pHeap Heap to allocate from.
 * @return Whether the buffer was allocated.
 */
bool Struct::constructBuffer_(s32 size, sead::Heap* pHeap)
{
    if (!mBuffer.tryAllocBuffer(size, pHeap)) {
        return false;
    }
    mStruct.SetBuffer(mBuffer.getBufferPtr(), size);
    return true;
}

/**
 * Computes the buffer size needed for a number of members.
 * @param memberNum Number of members.
 * @return Buffer size in bytes.
 */
s32 Struct::calcBufferSize(s32 memberNum)
{
    return static_cast<u32>(memberNum) * 130;
}

/**
 * Creates a struct with a buffer sized for a number of members.
 * @param memberNum Number of members.
 * @param pHeap Heap to allocate from.
 * @param isNothrow Whether to use the nothrow allocator.
 * @return The struct, or nullptr on failure.
 */
Struct* Struct::createWithMemberNum(s32 memberNum, sead::Heap* pHeap, bool isNothrow)
{
    Struct* pStruct;
    if (isNothrow) {
        pStruct = new (pHeap, std::nothrow) Struct();
    } else {
        pStruct = new (pHeap) Struct();
    }
    if (!pStruct) {
        return nullptr;
    }
    if (!pStruct->constructBuffer_(calcBufferSize(memberNum), pHeap)) {
        delete pStruct;
        return nullptr;
    }
    return pStruct;
}

/**
 * Creates a struct with a buffer of a given size.
 * @param bufferSize Buffer size in bytes.
 * @param pHeap Heap to allocate from.
 * @param isNothrow Whether to use the nothrow allocator.
 * @return The struct, or nullptr on failure.
 */
Struct* Struct::createWithBufferSize(s32 bufferSize, sead::Heap* pHeap, bool isNothrow)
{
    Struct* pStruct;
    if (isNothrow) {
        pStruct = new (pHeap, std::nothrow) Struct();
    } else {
        pStruct = new (pHeap) Struct();
    }
    if (!pStruct) {
        return nullptr;
    }
    if (!pStruct->constructBuffer_(bufferSize, pHeap)) {
        delete pStruct;
        return nullptr;
    }
    return pStruct;
}

/**
 * Adds a bool member.
 * @param rKey Member key.
 * @param value Member value.
 */
void Struct::addData(const sead::SafeString& rKey, bool value)
{
    if (mStruct.GetBuffer()) {
        mStruct.Add(rKey.cstr(), value);
    }
}

/**
 * Adds a signed 32-bit member.
 * @param rKey Member key.
 * @param value Member value.
 */
void Struct::addData(const sead::SafeString& rKey, s32 value)
{
    if (mStruct.GetBuffer()) {
        mStruct.Add(rKey.cstr(), static_cast<s64>(value));
    }
}

/**
 * Adds an unsigned 32-bit member.
 * @param rKey Member key.
 * @param value Member value.
 */
void Struct::addData(const sead::SafeString& rKey, u32 value)
{
    if (mStruct.GetBuffer()) {
        mStruct.Add(rKey.cstr(), static_cast<s64>(value));
    }
}

/**
 * Adds a signed 64-bit member.
 * @param rKey Member key.
 * @param value Member value.
 */
void Struct::addData(const sead::SafeString& rKey, s64 value)
{
    if (mStruct.GetBuffer()) {
        mStruct.Add(rKey.cstr(), value);
    }
}

/**
 * Adds an unsigned 64-bit member as a 64-bit id.
 * @param rKey Member key.
 * @param value Member value.
 */
void Struct::addData(const sead::SafeString& rKey, u64 value)
{
    if (mStruct.GetBuffer()) {
        nn::prepo::Any64BitId id = {value};
        mStruct.Add(rKey.cstr(), id);
    }
}

/**
 * Adds a float member.
 * @param rKey Member key.
 * @param value Member value.
 */
void Struct::addData(const sead::SafeString& rKey, f32 value)
{
    if (mStruct.GetBuffer()) {
        mStruct.Add(rKey.cstr(), value);
    }
}

/**
 * Adds a string member.
 * @param rKey Member key.
 * @param value Member value.
 */
void Struct::addData(const sead::SafeString& rKey, const char* value)
{
    if (mStruct.GetBuffer()) {
        mStruct.Add(rKey.cstr(), value);
    }
}

/**
 * Adds a string member.
 * @param rKey Member key.
 * @param rValue Member value.
 */
void Struct::addData(const sead::SafeString& rKey, const sead::SafeString& rValue)
{
    if (mStruct.GetBuffer()) {
        mStruct.Add(rKey.cstr(), rValue.cstr());
    }
}

}  // namespace erepo
