#pragma once
#include <nn/types.h>

namespace nn::atk::detail {
template <typename T>
class ValueArray {
public:
    ValueArray() : mValues(nullptr), mCount(0) {}
    // memory holds count entries; the array borrows this storage and clears its values.
    void Initialize(void* memory, int count) {
        mValues = static_cast<T*>(memory);
        mCount = count;
        Reset();
    }
    void Finalize() { mValues = nullptr; mCount = 0; }
    void Reset() {
        for (int i = 0; i < mCount; ++i) mValues[i] = T();
    }
    // other supplies entries up to this array's capacity; remaining entries are cleared.
    ValueArray& operator=(const ValueArray& other) {
        int count = other.mCount < mCount ? other.mCount : mCount;
        for (int i = 0; i < count; ++i) mValues[i] = other.mValues[i];
        for (int i = count; i < mCount; ++i) mValues[i] = T();
        return *this;
    }

private:
    friend class OutputAdditionalParam;
    T* mValues;
    int mCount;
};
}
