#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>

#include "Library/Execute/IUseExecutor.hpp"

namespace agl {
class DrawContext;
}  // namespace agl

namespace al {
class EffectSystem;

class EffectLayoutDrawer : public IUseExecutor {
public:
    EffectLayoutDrawer(EffectSystem* pEffectSystem, const char* pName, s32 groupId,
                       u32 renderPath);

    void execute() override;
    void draw() const override;
    void drawEffect(agl::DrawContext* pDrawContext, const sead::Matrix44f& rProjMtx,
                    const sead::Matrix34f& rViewMtx, f32 near, f32 far, f32 fovy) const;

    const char* getName() const { return mName; }

private:
    EffectSystem* mEffectSystem;
    const char* mName;
    s32 mGroupId;
    u32 mRenderPath;
};

static_assert(sizeof(EffectLayoutDrawer) == 0x20);
}  // namespace al
