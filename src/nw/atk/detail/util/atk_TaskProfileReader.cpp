#include <nn/atk/atk_TaskProfileReader.h>

namespace nn::atk {
TimeSpan TaskProfile::LoadStreamBlock::GetTotalTime() const {
    return os::ConvertToTimeSpan(mEnd - mBegin);
}

os::Tick TaskProfile::LoadStreamBlock::GetBeginTick() const { return mBegin; }
os::Tick TaskProfile::LoadStreamBlock::GetEndTick() const { return mEnd; }
float TaskProfile::LoadStreamBlock::GetRemainingCachePercentage() const {
    if (!mCachedLength || mCurrentPosition < mCachePosition) return 0.0f;
    float position = static_cast<float>(mCachePosition);
    float length = static_cast<float>(mCachedLength);
    return (length + position - static_cast<float>(mCurrentPosition)) / length * 100.0f;
}

size_t TaskProfile::LoadStreamBlock::GetCachedLength() const { return mCachedLength; }
detail::StreamSoundPlayer* TaskProfile::LoadStreamBlock::GetStreamSoundPlayer() const { return mPlayer; }
// begin and end delimit the task; cache supplies the decoder's cache range and owner.
void TaskProfile::LoadStreamBlock::SetData(const os::Tick& begin, const os::Tick& end,
    const detail::IStreamDataDecoder::CacheProfile& cache) {
    mBegin = begin;
    mEnd = end;
    mCachePosition = cache.cachePosition;
    mCachedLength = cache.cachedLength;
    mCurrentPosition = cache.currentPosition;
    mPlayer = cache.player;
}

TimeSpan TaskProfile::LoadOpusStreamBlock::GetTotalTime() const {
    return os::ConvertToTimeSpan(mEnd - mBegin);
}

os::Tick TaskProfile::LoadOpusStreamBlock::GetBeginTick() const { return mBegin; }
os::Tick TaskProfile::LoadOpusStreamBlock::GetEndTick() const { return mEnd; }
float TaskProfile::LoadOpusStreamBlock::GetRemainingCachePercentage() const {
    if (!mCachedLength || mCurrentPosition < mCachePosition) return 0.0f;
    float position = static_cast<float>(mCachePosition);
    float length = static_cast<float>(mCachedLength);
    return (length + position - static_cast<float>(mCurrentPosition)) / length * 100.0f;
}

size_t TaskProfile::LoadOpusStreamBlock::GetCachedLength() const { return mCachedLength; }
TimeSpan TaskProfile::LoadOpusStreamBlock::GetDecodeTime() const { return os::ConvertToTimeSpan(mDecodeTicks); }
int TaskProfile::LoadOpusStreamBlock::GetDecodedSampleCount() const { return mDecodedSampleCount; }
TimeSpan TaskProfile::LoadOpusStreamBlock::GetFsAccessTime() const { return os::ConvertToTimeSpan(mFsAccessTicks); }
size_t TaskProfile::LoadOpusStreamBlock::GetFsReadSize() const { return mFsReadSize; }
detail::StreamSoundPlayer* TaskProfile::LoadOpusStreamBlock::GetStreamSoundPlayer() const { return mPlayer; }
// begin and end delimit the task; decode supplies decoding and file-I/O measurements,
// while cache supplies the cached range, current stream position, and owning player.
void TaskProfile::LoadOpusStreamBlock::SetData(const os::Tick& begin, const os::Tick& end,
    const detail::IStreamDataDecoder::DecodeProfile& decode,
    const detail::IStreamDataDecoder::CacheProfile& cache) {
    mBegin = begin;
    mEnd = end;
    mDecodeTicks = decode.decodeTicks;
    mFsAccessTicks = decode.fsAccessTicks;
    mFsReadSize = decode.fsReadSize;
    mDecodedSampleCount = decode.decodedSampleCount;
    mCachePosition = cache.cachePosition;
    mCachedLength = cache.cachedLength;
    mCurrentPosition = cache.currentPosition;
    mPlayer = cache.player;
}

TaskProfileLogger::TaskProfileLogger() : mMutex(true), mEnabled(false) {}
// profile is copied to each registered reader that has a free record slot.
void TaskProfileLogger::Record(const TaskProfile& profile) {
    mMutex.Lock();
    for (auto& reader : mReaders) {
        if (reader.mCount.load(std::memory_order_acquire) < reader.mCapacity) {
            reader.mRecords[reader.mWriteIndex++] = profile;
            if (reader.mWriteIndex == reader.mCapacity) reader.mWriteIndex = 0;
            reader.mCount.fetch_add(1, std::memory_order_acq_rel);
        }
    }

    mMutex.Unlock();
}

// reader receives future records through its existing ring-buffer storage.
void TaskProfileLogger::RegisterReader(AtkProfileReader<TaskProfile>& reader) {
    mMutex.Lock();
    mReaders.push_back(reader);
    mMutex.Unlock();
}

// reader is removed from the registered list without releasing its storage.
void TaskProfileLogger::UnregisterReader(const AtkProfileReader<TaskProfile>& reader) {
    mMutex.Lock();
    mReaders.erase(mReaders.iterator_to(const_cast<AtkProfileReader<TaskProfile>&>(reader)));
    mMutex.Unlock();
}

// enabled is the profiling flag consulted by callers before they record tasks.
void TaskProfileLogger::SetProfilingEnabled(bool enabled) { mEnabled = enabled; }
void TaskProfileLogger::Finalize() {
    mMutex.Lock();
    mReaders.clear();
    mMutex.Unlock();
}
}
