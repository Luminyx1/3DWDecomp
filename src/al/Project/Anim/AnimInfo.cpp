#include "Project/Anim/AnimInfo.hpp"

#include <cstring>

#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * Gets the last frame of the animation.
 * @return Frame count.
 */
s32 AnimResInfo::getFrameMax() const {
    return frameMax;
}

/**
 * Checks whether the animation loops.
 * @return Whether the animation loops.
 */
bool AnimResInfo::isLoop() const {
    return isLoopAnim;
}

/**
 * Constructs a table with room for a number of animations.
 * @param maxInfos Maximum number of animations.
 */
AnimInfoTable::AnimInfoTable(s32 maxInfos) {
    mResInfos = new AnimResInfo[maxInfos];
}

/**
 * Adds an animation to the table.
 * @param pName Animation name.
 * @param pResAnim Animation resource.
 * @param frameMax Last frame of the animation.
 * @param isLoop Whether the animation loops.
 */
void AnimInfoTable::add(const char* pName, void* pResAnim, f32 frameMax, bool isLoop) {
    AnimResInfo& info = mResInfos[mInfoCount];
    info.name = createStringIfInStack(pName);
    info.resAnim = pResAnim;
    info.frameMax = frameMax;
    info.isLoopAnim = isLoop;
    mInfoCount++;
}

const AnimResInfo* AnimInfoTable::findAnimInfo(const char* pName) const {
    if (!mIsSorted) {
        for (s32 i = 0; i < mInfoCount; i++) {
            const AnimResInfo* info = &mResInfos[i];

            if (isEqualString(info->name, pName)) {
                return info;
            }
        }

        return nullptr;
    }

    s32 lo = 0;
    s32 hi = mInfoCount;

    while (lo < hi) {
        s32 last = hi - 1;
        s32 mid = (lo + last) >> 1;
        const AnimResInfo* info = &mResInfos[mid];
        s32 result = strcmp(info->name, pName);

        if (result > 0) {
            hi = mid;
        } else if (result < 0) {
            lo = mid + 1;
        } else {
            return info;
        }
    }

    return nullptr;
}

const AnimResInfo* AnimInfoTable::tryFindAnimInfo(const char* pName) const {
    if (!mIsSorted) {
        for (s32 i = 0; i < mInfoCount; i++) {
            const AnimResInfo* info = &mResInfos[i];

            if (isEqualString(info->name, pName)) {
                return info;
            }
        }

        return nullptr;
    }

    s32 lo = 0;
    s32 hi = mInfoCount;

    while (lo < hi) {
        s32 last = hi - 1;
        s32 mid = (lo + last) >> 1;
        const AnimResInfo* info = &mResInfos[mid];
        s32 result = strcmp(info->name, pName);

        if (result > 0) {
            hi = mid;
        } else if (result < 0) {
            lo = mid + 1;
        } else {
            return info;
        }
    }

    return nullptr;
}

void AnimInfoTable::sort() {
    s32 num = mInfoCount;
    AnimResInfo* infos = mResInfos;

    if (num >= 2 && infos != nullptr) {
        AnimResInfo value;

        for (s32 i = num / 2; i > 0; i--) {
            value = infos[i - 1];
            s32 parent = i;
            s32 child = parent * 2;

            while (child <= num) {
                if (child < num && strcmp(infos[child - 1].name, infos[child].name) < 0) {
                    child++;
                }

                if (strcmp(value.name, infos[child - 1].name) >= 0) {
                    break;
                }

                infos[parent - 1] = infos[child - 1];
                parent = child;
                child = parent * 2;
            }

            infos[parent - 1] = value;
        }

        for (s32 i = num; i >= 2; i--) {
            s32 last = i - 1;
            value = infos[last];
            infos[last] = infos[0];
            s32 parent = 1;
            s32 child = 2;

            while (child <= last) {
                if (child < last && strcmp(infos[child - 1].name, infos[child].name) < 0) {
                    child++;
                }

                if (strcmp(value.name, infos[child - 1].name) >= 0) {
                    break;
                }

                infos[parent - 1] = infos[child - 1];
                parent = child;
                child = parent * 2;
            }

            infos[parent - 1] = value;
        }
    }

    mIsSorted = true;
}

}  // namespace al
