#pragma once

#include <basis/seadTypes.h>
#include <math/seadBoundBox.h>
#include <math/seadVector.h>

namespace nn::g3d {
class MaterialObj;
class ModelObj;
class ResMaterial;
class ResModel;
class ResRenderInfo;
class ResShadingModel;
class ResShape;
class ShadingModelObj;
class ShapeObj;
class SkeletonObj;
class ViewVolume;
}  // namespace nn::g3d

namespace agl {
class DrawContext;
}

namespace al {
enum BlendType : s32 {
    BlendType_Xlu = 0,
    BlendType_OnlyLightBuf = 1,
    BlendType_XluWithNrm = 2,
};
}  // namespace al

namespace al {
class GpuMemAllocator;
class ModelShaderAssign;
class MeshDrawerTable;
class ShaderHolder;
class SimpleModelG3D;
}  // namespace al

namespace nn::g3d {
class ShaderSelector;
}

class alModelCafe;

namespace alModelFunction {
f32 calcBoundingSphere(const alModelCafe* pModel);
void calcBoundingBox(sead::BoundBox3f* pBox, const alModelCafe* pModel);

void updateRenderMaterialUbo(nn::g3d::MaterialObj* pMaterial);
s32 getRoughnessPresetIndex(const nn::g3d::ResRenderInfo* pRenderInfo);
void updateRenderCloudLayerUbo(nn::g3d::MaterialObj* pMaterial);
void drawModelShape(const nn::g3d::SkeletonObj* pSkeleton, const nn::g3d::MaterialObj* pMaterial,
                    const nn::g3d::ShapeObj* pShape, const nn::g3d::ShadingModelObj* pShadingModel,
                    const al::ModelShaderAssign* pShaderAssign,
                    const nn::g3d::ViewVolume* pViewVolume, s32 viewIndex, s32 bufferIndex,
                    s32 lodIndex);
bool isExistBoundingNode(const nn::g3d::ResShape* pShape);
bool isModelShapeDraw(const al::SimpleModelG3D* pModel, s32 index,
                      const nn::g3d::ViewVolume& rViewVolume);
al::MeshDrawerTable* createMeshDrawerTableFromAssignShader(const al::SimpleModelG3D* pModel, s32 type);
nn::g3d::ShaderSelector* createShaderSelector(al::GpuMemAllocator* pAllocator,
                                              const nn::g3d::ShapeObj* pShape,
                                              const nn::g3d::MaterialObj* pMaterial,
                                              nn::g3d::ResShadingModel* pShadingModel,
                                              s32 optionNum, const char* const* pOptionNames,
                                              const char* const* pOptionValues, bool);
nn::g3d::ShaderSelector* createShaderSelectorFromAssignShader(al::GpuMemAllocator* pAllocator,
                                                              const nn::g3d::ShapeObj* pShape,
                                                              const nn::g3d::MaterialObj* pMaterial);
void prepareModelShapeDrawDeferredGraphicsContext(agl::DrawContext* pContext,
                                                  const al::SimpleModelG3D* pModel, s32 index,
                                                  bool isAlphaMask, bool isXlu);
void prepareModelShapeDrawDeferredGraphicsContextByRenderState(agl::DrawContext* pContext,
                                                               const al::SimpleModelG3D* pModel,
                                                               s32 index);
void prepareModelShapeDrawDeferredGraphicsContextByCustom(agl::DrawContext* pContext,
                                                          const al::SimpleModelG3D* pModel,
                                                          s32 index, al::BlendType blendType);
void createMeshDrawerTableDisplayList(const al::MeshDrawerTable* pTable, const alModelCafe* pModel,
                                      bool);
void createMeshDrawerTableDisplayListRenderStateInvalidate(const al::MeshDrawerTable* pTable,
                                                           const alModelCafe* pModel, bool);
bool isShaderAssignAlphaMask(const nn::g3d::ModelObj* pModel, s32 index);
bool isShaderAssignAlphaMask(const nn::g3d::MaterialObj* pMaterial);
s32 getShaderAssignAlphaFunc(const nn::g3d::ModelObj* pModel, s32 index);
const char* getShaderAssignAlphaFunc(const nn::g3d::MaterialObj* pMaterial);
bool isShaderUsingThickness(const nn::g3d::ShadingModelObj& rShadingModel);
bool isShaderIndirect(const nn::g3d::ShadingModelObj& rShadingModel);
bool isShaderUsingRefractTex(const nn::g3d::ShadingModelObj& rShadingModel);
bool isMaterialUsingModelLight(const nn::g3d::ResMaterial* pMaterial);
s32 getMaterialDrawPriority(const nn::g3d::ResMaterial* pMaterial);
bool isMaterialApplyRenderState(const nn::g3d::ResMaterial* pMaterial);
bool isMaterialXluWithNrm(const nn::g3d::ResMaterial* pMaterial);
bool isMaterialOnlyBlendLightBuf(const nn::g3d::ResMaterial* pMaterial);
void bindShaderParamAndConvertParamCallback(nn::g3d::ResModel* pModel,
                                            const al::ShaderHolder* pShaderHolder);
bool isShaderAssignXlu(const nn::g3d::ModelObj* pModel, s32 index);
bool isShaderAssignXlu(const nn::g3d::MaterialObj* pMaterial);
bool isShaderAssignDeferred(const nn::g3d::ModelObj* pModel, s32 index);
bool isShaderAssignDeferred(const nn::g3d::MaterialObj* pMaterial);
bool isShaderAssignForward(const nn::g3d::ModelObj* pModel, s32 index);
bool isShaderAssignForward(const nn::g3d::MaterialObj* pMaterial);
}  // namespace alModelFunction
