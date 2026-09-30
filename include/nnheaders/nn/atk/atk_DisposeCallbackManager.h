#pragma once

#include <nn/atk/atk_SoundDataManager.h>

namespace nn::atk::detail::driver {
class DisposeCallbackManager {
public:
    static __attribute__((noinline)) DisposeCallbackManager& GetInstance();
    DisposeCallbackManager();
    void RegisterDisposeCallback(DisposeCallback* callback);
    void UnregisterDisposeCallback(DisposeCallback* callback);
    size_t GetCallbackCount() const;
    void Dispose(const void* memory, size_t size);

private:
    using CallbackList = nn::util::IntrusiveList<DisposeCallback,
        nn::util::IntrusiveListMemberNodeTraits<DisposeCallback, &DisposeCallback::m_DisposeLink>>;
    CallbackList mCallbacks;
};
static_assert(sizeof(DisposeCallbackManager) == 0x10, "DisposeCallbackManager size");
}
