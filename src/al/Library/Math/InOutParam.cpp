#include "Library/Math/InOutParam.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"

namespace al {

void InOutParam::init(const ByamlIter& iter) {
    iter.tryGetFloatByKey(&mInMin, "InMin");
    iter.tryGetFloatByKey(&mInMax, "InMax");
    iter.tryGetFloatByKey(&mOutMin, "OutMin");
    iter.tryGetFloatByKey(&mOutMax, "OutMax");
}

f32 InOutParam::calcLeapValue(f32 value) const {
    return lerpValue(value, mInMin, mInMax, mOutMin, mOutMax);
}

f32 InOutParam::calcEaseInValue(f32 value) const {
    f32 lerp = lerpValue(value, mInMin, mInMax, 0.0f, 1.0f);
    f32 eased = easeIn(lerp);
    return mOutMin + eased * (mOutMax - mOutMin);
}

f32 InOutParam::calcEaseOutValue(f32 value) const {
    f32 lerp = lerpValue(value, mInMin, mInMax, 0.0f, 1.0f);
    f32 eased = easeOut(lerp);
    return mOutMin + eased * (mOutMax - mOutMin);
}

f32 InOutParam::calcEaseInOutValue(f32 value) const {
    f32 lerp = lerpValue(value, mInMin, mInMax, 0.0f, 1.0f);
    f32 eased = easeInOut(lerp);
    return mOutMin + eased * (mOutMax - mOutMin);
}

f32 InOutParam::calcSqrtValue(f32 value) const {
    f32 lerp = lerpValue(value, mInMin, mInMax, 0.0f, 1.0f);
    f32 eased = sead::Mathf::sqrt(lerp);
    return mOutMin + eased * (mOutMax - mOutMin);
}

f32 InOutParam::calcSquareValue(f32 value) const {
    f32 lerp = lerpValue(value, mInMin, mInMax, 0.0f, 1.0f);
    f32 eased = sead::Mathf::square(lerp);
    return mOutMin + eased * (mOutMax - mOutMin);
}

}  // namespace al
