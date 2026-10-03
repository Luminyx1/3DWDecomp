#pragma once

#include <gfx/seadColor.h>

namespace agl {
class DrawContext;
}

namespace nn::g3d {
class MaterialObj;
}

namespace al {

class RenderState {
public:
    RenderState();

    void Use(agl::DrawContext* pDrawContext, nn::g3d::MaterialObj* pMaterialObj) const;

    bool isEnable() const { return _0; }
    s32 getMode() const { return _4; }
    s32 getBlendMode() const { return _8; }
    bool isAlphaTest() const { return _c; }

private:
    bool _0;
    s32 _4;
    s32 _8;
    bool _c;
    sead::Color4f mBlendColor;
};

static_assert(sizeof(RenderState) == 0x20);

}  // namespace al
