#pragma once

#include <basis/seadTypes.h>

namespace al {

enum class ActionAnimType : s32 { None = -1, Skl, Mcl, Mtp, Mts, Vis };

struct ActionAnimDataInfo {
    ActionAnimDataInfo();

    const char* actionName = nullptr;
    f32 _8 = -1.0f;
    bool isKeepAnim = false;
    bool isActionAnim = false;
};

static_assert(sizeof(ActionAnimDataInfo) == 0x10);

struct ActionAnimCtrlInfo {
    ActionAnimCtrlInfo(s32 sklSize);

    const char* actionName = nullptr;
    s32 sklDataCount;
    ActionAnimDataInfo* sklDatas = nullptr;
    ActionAnimDataInfo mclData;
    ActionAnimDataInfo mtpData;
    ActionAnimDataInfo mtsData;
    ActionAnimDataInfo visData;
    ActionAnimType actionAnimType = ActionAnimType::None;
};

static_assert(sizeof(ActionAnimCtrlInfo) == 0x60);

}  // namespace al

namespace alActionFunction {
const char* getAnimName(const al::ActionAnimCtrlInfo* pCtrlInfo,
                        const al::ActionAnimDataInfo* pDataInfo);
}  // namespace alActionFunction
