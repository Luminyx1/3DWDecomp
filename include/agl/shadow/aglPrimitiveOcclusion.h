#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "common/aglShaderEnum.h"

namespace sead {
class Camera;
class PerspectiveProjection;
class Projection;

namespace hostio {
class Context;
}  // namespace hostio
}  // namespace sead

namespace agl {
class ShaderProgram;
class TextureData;
}  // namespace agl

namespace agl::sdw {

/**
 * Ambient/directional occlusion from analytic primitives (spheres).
 */
class PrimitiveOcclusion {
public:
    struct CreateArg {
        s32 mContextNum;
        bool _4;
        bool _5;
        bool _6;
        bool _7;
        bool _8;
        bool _9;
        bool _a;
        bool _b;
    };
    static_assert(sizeof(CreateArg) == 0xc);

    PrimitiveOcclusion();
    ~PrimitiveOcclusion();

    // NOTE: only the slot of init() is known from callers.
    virtual void genMessage(sead::hostio::Context* pContext);
    virtual void init(const CreateArg& rArg);

    void setShaderSphereAo(const ShaderProgram* pProgram);
    void setShaderSphereDo(const ShaderProgram* pProgram);
    void setShaderMakeTableSphereDo(const ShaderProgram* pProgram);
    void clearRequest();
    void calcContext(s32 context, const sead::Camera& rCamera,
                     const sead::Projection& rProjection);
    void calcView(s32 context, const sead::Camera* pCamera,
                  const sead::PerspectiveProjection* pProjection);
    ShaderMode drawPrecomputeSphereDo(s32 context, ShaderMode shaderMode) const;
    ShaderMode drawDo(s32 context, const TextureData& rNormal, const TextureData& rDepth,
                      bool isView, ShaderMode shaderMode) const;
    ShaderMode drawAo(s32 context, const TextureData& rNormal, const TextureData& rDepth,
                      bool isView, ShaderMode shaderMode) const;

private:
    u8 _8[0x888 - 0x8];
};

static_assert(sizeof(PrimitiveOcclusion) == 0x888);

}  // namespace agl::sdw
