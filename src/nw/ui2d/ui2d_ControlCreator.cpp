#include <nn/ui2d/ui2d_ControlSrc.h>
#include <cstring>
namespace nn::ui2d {
namespace {
struct ResControl {
    u32 signature, size, userNameOffset, paneNamesOffset;
    u16 paneCount, animCount;
    u32 paneFunctionsOffset, animFunctionsOffset;
    char name[1];
};
}

ControlSrc::ControlSrc() : mName(nullptr), mUserName(nullptr), mPaneCount(0), mAnimCount(0),
    mPaneNames(nullptr), mAnimNameOffsets(nullptr), mPaneFunctionOffsets(nullptr),
    mAnimFunctionOffsets(nullptr), mExtData(nullptr) {}
// pResource supplies the packed control data; pExtData supplies optional extended user data.
void ControlSrc::Initialize(const void* pResource, const ResExtUserDataList* pExtData) {
    const auto* resource = static_cast<const ResControl*>(pResource);
    const auto* bytes = static_cast<const char*>(pResource);
    mName = resource->name;
    mUserName = bytes + resource->userNameOffset;
    const u16 paneCount = resource->paneCount;
    mPaneCount = paneCount;
    mAnimCount = resource->animCount;
    mPaneNames = bytes + resource->paneNamesOffset;
    mAnimNameOffsets = reinterpret_cast<const u32*>(bytes + u32(resource->paneNamesOffset + paneCount * 24));
    mPaneFunctionOffsets = reinterpret_cast<const u32*>(bytes + resource->paneFunctionsOffset);
    mAnimFunctionOffsets = reinterpret_cast<const u32*>(bytes + resource->animFunctionsOffset);
    mExtData = pExtData;
}

// index selects a fixed-width pane name; indices at or above the pane count return null.
const char* ControlSrc::GetFunctionalPaneName(int index) const {
    return index < mPaneCount ? mPaneNames + index * 24 : nullptr;
}

// pName identifies the pane's functional role rather than its layout name.
const char* ControlSrc::FindFunctionalPaneName(const char* pName) const {
    for (size_t i = 0; i < mPaneCount; ++i) {
        const char* name = reinterpret_cast<const char*>(mPaneFunctionOffsets) + mPaneFunctionOffsets[i];
        if (std::strcmp(name, pName) == 0) return mPaneNames + i * 24;
    }

    return nullptr;
}

// index selects an animation name from the offset table.
const char* ControlSrc::GetFunctionalAnimName(int index) const {
    return index < mAnimCount ? reinterpret_cast<const char*>(mAnimNameOffsets) + mAnimNameOffsets[index] : nullptr;
}

// pName identifies the animation's functional role.
const char* ControlSrc::FindFunctionalAnimName(const char* pName) const {
    for (size_t i = 0; i < mAnimCount; ++i) {
        const char* name = reinterpret_cast<const char*>(mAnimFunctionOffsets) + mAnimFunctionOffsets[i];
        if (std::strcmp(name, pName) == 0) return reinterpret_cast<const char*>(mAnimNameOffsets) + mAnimNameOffsets[i];
    }

    return nullptr;
}

int ControlSrc::GetExtUserDataCount() const { return mExtData ? mExtData->count : 0; }
const ResExtUserData* ControlSrc::GetExtUserDataArray() const { return mExtData ? mExtData->entries : nullptr; }
// pName identifies an extended user-data entry; null is returned when none matches.
const ResExtUserData* ControlSrc::FindExtUserDataByName(const char* pName) const {
    const ResExtUserData* data = GetExtUserDataArray();
    for (u32 i = 0; i < u32(GetExtUserDataCount()); ++i, ++data) {
        const char* name = data->nameOffset ? reinterpret_cast<const char*>(data) + data->nameOffset : nullptr;
        if (std::strcmp(pName, name) == 0) return data;
    }

    return nullptr;
}
}
