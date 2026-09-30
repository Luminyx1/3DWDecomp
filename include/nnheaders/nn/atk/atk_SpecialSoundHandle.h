#pragma once
#include <nn/atk/atk_SoundHandle.h>

namespace nn::atk {
class StreamSoundHandle;
class SequenceSoundHandle;
namespace detail {
// These declarations expose the attachment fields used by the specialized handles.
class StreamSound : public BasicSound {
public:
    static const RuntimeTypeInfo* GetRuntimeTypeInfoStatic() {
        static const RuntimeTypeInfo s_TypeInfo(BasicSound::GetRuntimeTypeInfoStatic());
        return &s_TypeInfo;
    }
private:
    friend class nn::atk::StreamSoundHandle;
    u8 _210[0x10];
    StreamSoundHandle* mTempHandle;
};
class SequenceSound : public BasicSound {
public:
    static const RuntimeTypeInfo* GetRuntimeTypeInfoStatic() {
        static const RuntimeTypeInfo s_TypeInfo(BasicSound::GetRuntimeTypeInfoStatic());
        return &s_TypeInfo;
    }
private:
    friend class nn::atk::SequenceSoundHandle;
    u8 _210[0x10];
    SequenceSoundHandle* mTempHandle;
};
// sound is tested against T's type and its descendants; an unrelated sound returns null.
template <typename T> T* SoundCast(BasicSound* sound) {
    const RuntimeTypeInfo* target = T::GetRuntimeTypeInfoStatic();
    for (const RuntimeTypeInfo* info = sound->GetRuntimeTypeInfo(); info; info = info->parent)
        if (info == target) return static_cast<T*>(sound);
    return nullptr;
}
}
class StreamSoundHandle {
public:
    explicit StreamSoundHandle(SoundHandle* handle);
    void detail_AttachSoundAsTempHandle(detail::StreamSound* sound);
    void DetachSound();
private:
    detail::StreamSound* mSound;
};
class SequenceSoundHandle {
public:
    explicit SequenceSoundHandle(SoundHandle* handle);
    void detail_AttachSoundAsTempHandle(detail::SequenceSound* sound);
    void DetachSound();
private:
    detail::SequenceSound* mSound;
};
}
