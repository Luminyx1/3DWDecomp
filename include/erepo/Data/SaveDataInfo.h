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

    void tryGet(T* pValue) const
    {
        if (isValid()) {
            *pValue = get();
        }
    }

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

    void tryGet(f32* pValue) const
    {
        if (isValid()) {
            *pValue = get();
        }
    }

    void set(f32 value) { mValue = value; }
    void reset() { mValue = std::numeric_limits<f32>::quiet_NaN(); }

private:
    f32 mValue;
};

class SaveDataInfo {
public:
    static constexpr s32 cValueNum = 19;
    static constexpr s32 cFloatNum = 8;
    static constexpr s32 cPlayStyleTimeNum = 5;

    enum ValueIndex {
        cValueIndex_ActiveTime = 0,
        cValueIndex_LastDailyReportTime = 1,
        cValueIndex_SleepTime = 2,
        cValueIndex_PlayTime = 3,
        cValueIndex_PlayStyleTime = 4,
    };

    SaveDataInfo();
    SaveDataInfo(const SaveDataInfo& rOther) = default;
    virtual ~SaveDataInfo();

    OptionalValue<u32>& getValue(s32 index) { return mValues[index]; }
    const OptionalValue<u32>& getValue(s32 index) const { return mValues[index]; }

    OptionalValue<u32>& getPlayStyleTime(s32 style, s32 index)
    {
        return mValues[cValueIndex_PlayStyleTime + style * cPlayStyleTimeNum + index];
    }

    const OptionalValue<u32>& getPlayStyleTime(s32 style, s32 index) const
    {
        return mValues[cValueIndex_PlayStyleTime + style * cPlayStyleTimeNum + index];
    }

    OptionalFloat& getFloat(s32 index) { return mFloats[index]; }
    const OptionalFloat& getFloat(s32 index) const { return mFloats[index]; }

    OptionalValue<u32> mValues[cValueNum];
    OptionalFloat mFloats[cFloatNum];
};

}  // namespace erepo
