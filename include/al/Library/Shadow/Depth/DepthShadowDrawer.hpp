#pragma once

#include <basis/seadTypes.h>
#include <common/aglShaderEnum.h>

namespace agl {
class ShaderProgram;
}

namespace agl::sdw {
class DepthShadow;
}

namespace al {

class DepthShadowDrawer {
public:
    void setupShaderProgram(s32 viewIndex, const agl::ShaderProgram* pProgram);

    agl::sdw::DepthShadow* getDepthShadow() const { return mDepthShadow; }

    bool isPreDraw() const { return mIsPreDraw; }

    s32 getDrawShadowIndex() const { return mDrawShadowIndex; }

    agl::ShaderMode drawToDepthShadow(agl::ShaderMode shaderMode) const;
    void setupDepthShadow(s32 viewIndex);

    bool isEnable() const { return mIsEnable; }

    void setDrawFlags(bool a4, bool a5) {
        _a4 = a4;
        _a5 = a5;
    }

private:
    u8 _0[0x18];
    agl::sdw::DepthShadow* mDepthShadow;
    u8 _20[0xa4 - 0x20];
    bool _a4;
    bool _a5;
    u8 _a6[0xab - 0xa6];
    bool mIsEnable;
    bool mIsPreDraw;
    s32 mDrawShadowIndex;
};

}  // namespace al
