#pragma once
#include "Library/Nerve/NerveStateBase.hpp"
#include "math/seadVector.h"

struct RenderMaterialIndirectParam {
    float interpolationRate;
    float eta;
    sead::Vector4f absorbRefraction;
    sead::Vector4f selectiveReflection;
    sead::Vector4f modelLightColor;
};
static_assert(sizeof(RenderMaterialIndirectParam) == 0x38);

class ChameleonStateMic : public al::ActorStateBase {
public:
    ChameleonStateMic(al::LiveActor*, RenderMaterialIndirectParam*, const RenderMaterialIndirectParam*);
    ~ChameleonStateMic() override;
    void appear() override;
    void exeMic();
private:
    RenderMaterialIndirectParam* mpParam;
    const RenderMaterialIndirectParam* mpTarget;
};
static_assert(sizeof(ChameleonStateMic) == 0x30);

namespace ChameleonStateUtil {
void updateIndirectParam(RenderMaterialIndirectParam*, const RenderMaterialIndirectParam*);
void updateIndirectParam(RenderMaterialIndirectParam*, float, float, const sead::Vector4f&, const sead::Vector4f&, const sead::Vector4f&);
void setRenderMaterialIndirectParam(al::LiveActor*, const RenderMaterialIndirectParam*);
}
