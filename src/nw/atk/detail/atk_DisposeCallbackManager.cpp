#include <nn/atk/atk_DisposeCallbackManager.h>

namespace nn::atk::detail::driver {
DisposeCallbackManager::DisposeCallbackManager() {}

// callback is appended to the list of objects notified when storage is disposed.
void DisposeCallbackManager::RegisterDisposeCallback(DisposeCallback* callback) {
    mCallbacks.push_back(*callback);
}

// callback is removed from the notification list without destroying the object.
void DisposeCallbackManager::UnregisterDisposeCallback(DisposeCallback* callback) {
    mCallbacks.erase(mCallbacks.iterator_to(*callback));
}

size_t DisposeCallbackManager::GetCallbackCount() const {
    return std::distance(mCallbacks.begin(), mCallbacks.end());
}

// memory and size identify the half-open storage range passed to each callback.
void DisposeCallbackManager::Dispose(const void* memory, size_t size) {
    auto it = GetInstance().mCallbacks.begin();
    while (it != GetInstance().mCallbacks.end()) {
        auto& callback = *it++;
        callback.InvalidateData(memory, static_cast<const u8*>(memory) + size);
    }
}

DisposeCallbackManager& DisposeCallbackManager::GetInstance() {
    static DisposeCallbackManager instance;
    return instance;
}

}
