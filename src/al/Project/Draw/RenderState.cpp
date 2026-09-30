#include "Project/Draw/RenderState.hpp"

#include <nn/g3d/g3d_MaterialObj.h>

namespace al {

/**
 * Creates a render state with an opaque white blend color.
 */
RenderState::RenderState() {
    _4 = 0;
    _0 = false;
    _c = false;
    mBlendColor = {1.0f, 1.0f, 1.0f, 1.0f};
    _8 = 0;
}

/**
 * Writes the blend alpha into the material's blend color shader parameter.
 * @param pDrawContext Draw context.
 * @param pMaterialObj Material to update.
 */
void RenderState::Use(agl::DrawContext* pDrawContext, nn::g3d::MaterialObj* pMaterialObj) const {
    s32 index = pMaterialObj->GetResource()->FindShaderParamIndex("cBlendColor");

    if (index == nn::util::ResDic::Npos)
        return;

    sead::Color4f* blendColor = pMaterialObj->EditShaderParam<sead::Color4f>(index);
    blendColor->a = mBlendColor.a;
}

}  // namespace al
