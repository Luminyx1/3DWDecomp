#pragma once

#include <basis/seadTypes.h>
#include <limits>

namespace erepo {

template <typename T>
class OptionalValue {
public:
    OptionalValue() : mValue(0), mIsValid(false) {}
    OptionalValue(const OptionalValue& rOther) : OptionalValue()
    {
        if (rOther.mIsValid) {
            set(rOther.mValue);
        }
    }

    ~OptionalValue() { reset(); }

    bool isValid() const { return mIsValid; }
    T get() const { return mValue; }
    void set(T value)
    {
        const bool isValid = mIsValid;
        mValue = value;
        if (!isValid) {
            mIsValid = true;
        }
    }

    void reset()
    {
        if (mIsValid) {
            mIsValid = false;
        }
    }

private:
    T mValue;
    bool mIsValid;
};

class OptionalFloat {
public:
    OptionalFloat() : mValue(std::numeric_limits<f32>::quiet_NaN()) {}
    OptionalFloat(const OptionalFloat& rOther)
    {
        if (rOther.isValid()) {
            set(rOther.get());
        } else {
            mValue = std::numeric_limits<f32>::quiet_NaN();
        }
    }

    bool isValid() const
    {
        return (*reinterpret_cast<const u32*>(&mValue) & 0x7fffffff) <= 0x7f800000;
    }

    f32 get() const { return mValue; }
    void set(f32 value) { mValue = value; }
    void reset() { mValue = std::numeric_limits<f32>::quiet_NaN(); }

private:
    f32 mValue;
};

class SaveDataInfo {
public:
    static constexpr s32 cValueNum = 19;
    static constexpr s32 cFloatNum = 8;

    SaveDataInfo();
    SaveDataInfo(const SaveDataInfo& rOther) = default;
    virtual ~SaveDataInfo();

    OptionalValue<u32> mValues[cValueNum];
    OptionalFloat mFloats[cFloatNum];
};

}  // namespace erepo
