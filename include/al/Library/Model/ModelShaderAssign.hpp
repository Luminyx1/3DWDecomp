#pragma once

#include <common/aglShaderLocation.h>
#include <g3d/aglModelShaderAssign.h>

namespace agl {
class DrawContext;
class ShaderProgram;
}  // namespace agl

namespace nn::g3d {
class MaterialObj;
class ResMaterial;
class ResShaderProgram;
class ResShadingModel;
class ResShape;
}  // namespace nn::g3d

namespace sead {
class Heap;
}

namespace al {

class ModelShaderAssign {
public:
    ModelShaderAssign();
    ~ModelShaderAssign();

    void create(sead::Heap* pHeap);
    void setBuffer(void* pBuffer);
    void bind(const nn::g3d::ResMaterial* pMaterial, const nn::g3d::ResShape* pShape,
              const nn::g3d::ResShadingModel* pShadingModel,
              const nn::g3d::ResShaderProgram* pProgram);
    void activateMaterialUniformBlock(agl::DrawContext* pContext,
                                      const nn::g3d::MaterialObj* pMaterial,
                                      s32 bufferIndex) const;

    const nn::g3d::ResShaderProgram* getResShaderProgram() const { return mResShaderProgram; }
    const agl::g3d::ModelShaderAttribute& getAttribute() const { return mAttribute; }
    const agl::g3d::ModelShaderSampler& getSampler() const { return mSampler; }

private:
    void clear_();
    void updateLocation_(const nn::g3d::ResMaterial* pMaterial,
                         const nn::g3d::ResShadingModel* pShadingModel, const char* pName);

    const agl::ShaderProgram* mShaderProgram = nullptr;
    const nn::g3d::ResShaderProgram* mResShaderProgram = nullptr;
    agl::UniformBlockLocation mMaterialBlockLocation;
    agl::g3d::ModelShaderAttribute mAttribute;
    agl::g3d::ModelShaderSampler mSampler;
    u8* mFetchShaderBuffer = nullptr;
};

static_assert(sizeof(ModelShaderAssign) == 0x3f0);

}  // namespace al
