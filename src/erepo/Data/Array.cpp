#include <erepo/Data/Array.h>

#include <basis/seadNew.h>
#include <erepo/Data/Struct.h>
#include <erepo/Manager.h>

namespace erepo {

/**
 * Constructs an empty array without buffers.
 */
Array::Array() = default;

/**
 * Frees the array buffer and deletes all owned structs.
 */
Array::~Array()
{
    mBuffer.freeBuffer();
    if (mStructs.isBufferReady()) {
        for (auto* pStruct : mStructs) {
            delete pStruct;
        }

        mStructs.freeBuffer();
    }
}

/**
 * Appends a bool element.
 * @param rValue Element value.
 */
template <>
void Array::addData<bool>(const bool& rValue)
{
    if (mArray.GetBuffer()) {
        mArray.Add(rValue);
    }
}

/**
 * Appends a signed 32-bit element.
 * @param rValue Element value.
 */
template <>
void Array::addData<s32>(const s32& rValue)
{
    if (mArray.GetBuffer()) {
        mArray.Add(static_cast<s64>(rValue));
    }
}

/**
 * Appends an unsigned 32-bit element.
 * @param rValue Element value.
 */
template <>
void Array::addData<u32>(const u32& rValue)
{
    if (mArray.GetBuffer()) {
        mArray.Add(static_cast<s64>(rValue));
    }
}

/**
 * Appends a signed 64-bit element.
 * @param rValue Element value.
 */
template <>
void Array::addData<s64>(const s64& rValue)
{
    if (mArray.GetBuffer()) {
        mArray.Add(rValue);
    }
}

/**
 * Appends an unsigned 64-bit element as a 64-bit id.
 * @param rValue Element value.
 */
template <>
void Array::addData<u64>(const u64& rValue)
{
    if (mArray.GetBuffer()) {
        nn::prepo::Any64BitId id = {rValue};
        mArray.Add(id);
    }
}

/**
 * Appends a float element.
 * @param rValue Element value.
 */
template <>
void Array::addData<f32>(const f32& rValue)
{
    if (mArray.GetBuffer()) {
        mArray.Add(rValue);
    }
}

/**
 * Appends a string element.
 * @param rValue Element value.
 */
template <>
void Array::addData<sead::SafeString>(const sead::SafeString& rValue)
{
    if (mArray.GetBuffer()) {
        mArray.Add(rValue.cstr());
    }
}

/**
 * Appends a struct element.
 * @param rValue Element value.
 */
template <>
void Array::addData<Struct>(const Struct& rValue)
{
    if (mArray.GetBuffer()) {
        mArray.Add(rValue.getStruct());
    }
}

/**
 * Allocates the buffer for bool elements.
 * @param num Number of elements.
 * @param pHeap Heap to allocate from.
 * @return Whether the buffer was allocated.
 */
template <>
bool Array::constructBuffer<bool>(s32 num, sead::Heap* pHeap)
{
    return constructBuffer_(num * sizeof(bool), pHeap);
}

/**
 * Allocates the buffer for signed 32-bit elements.
 * @param num Number of elements.
 * @param pHeap Heap to allocate from.
 * @return Whether the buffer was allocated.
 */
template <>
bool Array::constructBuffer<s32>(s32 num, sead::Heap* pHeap)
{
    return constructBuffer_(static_cast<s32>(num * sizeof(s64)), pHeap);
}

/**
 * Allocates the buffer for unsigned 32-bit elements.
 * @param num Number of elements.
 * @param pHeap Heap to allocate from.
 * @return Whether the buffer was allocated.
 */
template <>
bool Array::constructBuffer<u32>(s32 num, sead::Heap* pHeap)
{
    return constructBuffer_(static_cast<s32>(num * sizeof(s64)), pHeap);
}

/**
 * Allocates the buffer for signed 64-bit elements.
 * @param num Number of elements.
 * @param pHeap Heap to allocate from.
 * @return Whether the buffer was allocated.
 */
template <>
bool Array::constructBuffer<s64>(s32 num, sead::Heap* pHeap)
{
    return constructBuffer_(static_cast<s32>(num * sizeof(s64)), pHeap);
}

/**
 * Allocates the buffer for unsigned 64-bit elements.
 * @param num Number of elements.
 * @param pHeap Heap to allocate from.
 * @return Whether the buffer was allocated.
 */
template <>
bool Array::constructBuffer<u64>(s32 num, sead::Heap* pHeap)
{
    return constructBuffer_(static_cast<s32>(num * sizeof(s64)), pHeap);
}

/**
 * Allocates the buffer for float elements.
 * @param num Number of elements.
 * @param pHeap Heap to allocate from.
 * @return Whether the buffer was allocated.
 */
template <>
bool Array::constructBuffer<f32>(s32 num, sead::Heap* pHeap)
{
    return constructBuffer_(static_cast<s32>(num * sizeof(s64)), pHeap);
}

/**
 * Allocates the buffer for string elements.
 * @param num Number of elements.
 * @param length Buffer size per element.
 * @param pHeap Heap to allocate from.
 * @return Whether the buffer was allocated.
 */
template <>
bool Array::constructBuffer<sead::SafeString>(s32 num, s32 length, sead::Heap* pHeap)
{
    return constructBuffer_(num * length, pHeap);
}

/**
 * Allocates the buffer and struct slots for struct elements sized by member count.
 * @param num Number of elements.
 * @param memberNum Number of members per struct.
 * @param pHeap Heap to allocate from.
 * @return Whether both buffers were allocated.
 */
bool Array::constructStructBuffer(s32 num, s32 memberNum, sead::Heap* pHeap)
{
    if (!constructBuffer_(Struct::calcBufferSize(memberNum) * num, pHeap)) {
        return false;
    }

    if (!mStructs.tryAllocBuffer(num, pHeap)) {
        return false;
    }

    mStructs.fill(nullptr);
    return true;
}

/**
 * Allocates the buffer and struct slots for struct elements of a given size.
 * @param num Number of elements.
 * @param bufferSize Buffer size per struct.
 * @param pHeap Heap to allocate from.
 * @return Whether both buffers were allocated.
 */
bool Array::constructStructBufferWithBufferSize(s32 num, s32 bufferSize, sead::Heap* pHeap)
{
    if (!constructBuffer_(bufferSize * num, pHeap)) {
        return false;
    }

    if (!mStructs.tryAllocBuffer(num, pHeap)) {
        return false;
    }

    mStructs.fill(nullptr);
    return true;
}

/**
 * Creates a struct sized by member count and stores it in a free slot.
 * @param memberNum Number of members.
 * @return The struct, or nullptr on failure.
 */
Struct* Array::CreateStruct(s32 memberNum)
{
    Struct* pStruct = Struct::createWithMemberNum(memberNum, Manager::instance()->getHeap(), true);
    if (!pStruct) {
        return nullptr;
    }

    for (s32 i = 0; i < mStructs.size(); i++) {
        if (!mStructs(i)) {
            mStructs(i) = pStruct;
            return pStruct;
        }
    }

    return nullptr;
}

/**
 * Creates a struct of a given buffer size and stores it in a free slot.
 * @param bufferSize Buffer size in bytes.
 * @return The struct, or nullptr on failure.
 */
Struct* Array::CreateStructWithBufferSize(s32 bufferSize)
{
    Struct* pStruct = Struct::createWithBufferSize(bufferSize, Manager::instance()->getHeap(), true);
    if (!pStruct) {
        return nullptr;
    }

    for (s32 i = 0; i < mStructs.size(); i++) {
        if (!mStructs(i)) {
            mStructs(i) = pStruct;
            return pStruct;
        }
    }

    return nullptr;
}

}  // namespace erepo
