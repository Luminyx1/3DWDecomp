#pragma once
#include <nn/os.h>
#include <nn/util/util_IntrusiveList.h>
#include <atomic>

namespace nn::atk {
namespace detail {
class StreamSoundPlayer;
class IStreamDataDecoder {
public:
    struct CacheProfile {
        s64 cachePosition;
        size_t cachedLength;
        s64 currentPosition;
        StreamSoundPlayer* player;
    };
    struct DecodeProfile {
        os::Tick decodeTicks;
        int decodedSampleCount;
        os::Tick fsAccessTicks;
        size_t fsReadSize;
    };
};
}
class TaskProfile {
public:
    class LoadStreamBlock {
    public:
        TimeSpan GetTotalTime() const;
        os::Tick GetBeginTick() const;
        os::Tick GetEndTick() const;
        float GetRemainingCachePercentage() const;
        size_t GetCachedLength() const;
        detail::StreamSoundPlayer* GetStreamSoundPlayer() const;
        void SetData(const os::Tick& begin, const os::Tick& end,
                     const detail::IStreamDataDecoder::CacheProfile& cache);
    private:
        os::Tick mBegin, mEnd;
        s64 mCachePosition;
        size_t mCachedLength;
        s64 mCurrentPosition;
        detail::StreamSoundPlayer* mPlayer;
    };
    class LoadOpusStreamBlock {
    public:
        TimeSpan GetTotalTime() const;
        os::Tick GetBeginTick() const;
        os::Tick GetEndTick() const;
        float GetRemainingCachePercentage() const;
        size_t GetCachedLength() const;
        TimeSpan GetDecodeTime() const;
        int GetDecodedSampleCount() const;
        TimeSpan GetFsAccessTime() const;
        size_t GetFsReadSize() const;
        detail::StreamSoundPlayer* GetStreamSoundPlayer() const;
        void SetData(const os::Tick& begin, const os::Tick& end,
                     const detail::IStreamDataDecoder::DecodeProfile& decode,
                     const detail::IStreamDataDecoder::CacheProfile& cache);
    private:
        os::Tick mBegin, mEnd;
        s64 mCachePosition;
        size_t mCachedLength;
        s64 mCurrentPosition;
        os::Tick mDecodeTicks, mFsAccessTicks;
        size_t mFsReadSize;
        int mDecodedSampleCount;
        detail::StreamSoundPlayer* mPlayer;
    };
private:
    u64 mType;
    union {
        LoadStreamBlock mStream;
        LoadOpusStreamBlock mOpus;
    };
};
class TaskProfileLogger;
template <typename T> class AtkProfileReader {
    friend class TaskProfileLogger;
private:
    util::IntrusiveListNode mNode;
    u8 mReserved10[8];
    T* mRecords;
    int mCapacity;
    int mWriteIndex;
    int mReadIndex;
    std::atomic<int> mCount;
};
class TaskProfileLogger {
public:
    TaskProfileLogger();
    void Record(const TaskProfile& profile);
    void RegisterReader(AtkProfileReader<TaskProfile>& reader);
    void UnregisterReader(const AtkProfileReader<TaskProfile>& reader);
    void SetProfilingEnabled(bool enabled);
    void Finalize();
private:
    using Reader = AtkProfileReader<TaskProfile>;
    using ReaderList = util::IntrusiveList<Reader, util::IntrusiveListMemberNodeTraits<Reader, &Reader::mNode>>;
    ReaderList mReaders;
    os::Mutex mMutex;
    bool mEnabled;
};
static_assert(sizeof(TaskProfile::LoadStreamBlock) == 0x30, "Stream task profile size");
static_assert(sizeof(TaskProfile::LoadOpusStreamBlock) == 0x50, "Opus task profile size");
static_assert(sizeof(TaskProfile) == 0x58, "Task profile record size");
static_assert(sizeof(AtkProfileReader<TaskProfile>) == 0x30, "Profile reader size");
static_assert(sizeof(TaskProfileLogger) == 0x38, "Task profile logger size");
}
