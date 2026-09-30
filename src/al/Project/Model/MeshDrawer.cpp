#include "Project/Model/MeshDrawer.hpp"

#include <common/aglDisplayList.h>
#include <common/aglDrawContext.h>
#include <common/aglShaderLocation.h>
#include <driver/aglGraphicsDriverMgr.h>
#include <g3d/aglShaderUtilG3D.h>
#include <gfx/seadGraphics.h>
#include <gfx/seadGraphicsContext.h>
#include <heap/seadHeap.h>
#include <math/seadBoundBox.h>
#include <math/seadGeometry.h>
#include <mc/seadCoreInfo.h>
#include <nn/g3d/g3d_ModelObj.h>
#include <nn/g3d/g3d_ResMaterial.h>
#include <nn/g3d/g3d_ResModel.h>
#include <nn/g3d/g3d_ResShader.h>
#include <nn/g3d/g3d_ResShape.h>
#include <nn/g3d/g3d_SkeletonObj.h>
#include <nn/util/util_VectorApi.h>
#include <shadow/aglDepthShadow.h>

#include "Library/Draw/GraphicsFunction.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/ModelShaderAssign.hpp"
#include "Library/Shader/DeferredRendering/ModelLightParam.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/EnvTextureKeeper.hpp"
#include "Library/Shadow/Depth/DepthShadowDrawer.hpp"
#include "Project/Draw/GpuMemAllocator.hpp"
#include "Project/Model/ModelAdditionalInfo.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

static agl::DrawContext* getDrawContext() {
    return reinterpret_cast<agl::DrawContext*>(GameFrameworkNx::sInstance->mDrawContext);
}

static nn::gfx::CommandBuffer* getCommandBuffer(agl::DrawContext* pContext) {
    return reinterpret_cast<nn::gfx::CommandBuffer*>(reinterpret_cast<uintptr_t>(pContext) + 8);
}

/**
 * Constructs a mesh drawer drawing one shape of all instances of a model.
 * @param pName Name of the shape.
 * @param pModelObj Model object the shape belongs to.
 * @param pShapeObj Shape object.
 * @param pSelector Shader selector used to draw the shape.
 * @param meshNum Maximum number of model instances.
 */
MeshDrawer::MeshDrawer(const char* pName, const nn::g3d::ModelObj* pModelObj,
                       const nn::g3d::ShapeObj* pShapeObj,
                       const nn::g3d::ShaderSelector* pSelector, s32 meshNum)
    : mName(pName), mModelObj(pModelObj), mShapeObj(pShapeObj), mShaderSelector(pSelector) {
    mMaterialObj = pModelObj->GetMaterial(pShapeObj->GetResource()->GetMaterialIndex());
    mShapeIndex = pModelObj->GetResource()->FindShapeIndex(pShapeObj->GetResource()->GetName());
    mDrawPriority = alModelFunction::getMaterialDrawPriority(mMaterialObj->GetResource());
    mIsUsingModelLight = alModelFunction::isMaterialUsingModelLight(mMaterialObj->GetResource());
    mMeshNumMax = meshNum;
    mMeshes = new Mesh*[meshNum];

    for (s32 i = 0; i < mMeshNumMax; i++) {
        mMeshes[i] = new Mesh;
    }

    mShaderAssign = new ModelShaderAssign();
    mShaderAssign->create(nullptr);
    mShaderAssign->bind(mMaterialObj->GetResource(), mShapeObj->GetResource(),
                        mShaderSelector->GetShadingModel()->GetResource(),
                        mShaderSelector->GetProgram());

    mUniformRegisterBuffer = nullptr;
    mUniformRegisterSize2 = 0;

    if (mUniformRegisterSize != 0) {
        mUniformRegisterBuffer = new (0x100) u8[mUniformRegisterSize];

        if (mUniformRegisterSize2 != 0) {
            mUniformRegisterBuffer2 = new (0x100) u8[mUniformRegisterSize2];
        }
    }

    const nn::g3d::ResShaderProgram* program = mShaderSelector->GetProgram();
    const nn::g3d::ResShadingModel* shadingModel = mShaderSelector->GetShadingModel()->GetResource();
    mSkeletonBlockLocation = program->GetUniformBlockLocation(
        shadingModel->GetSkeletonBlockIndex(), nn::g3d::Stage_Vertex);
    mShapeBlockLocation = program->GetUniformBlockLocation(shadingModel->GetShapeBlockIndex(),
                                                           nn::g3d::Stage_Vertex);
}

inline bool MeshDrawer::isDrawMesh(const SimpleModelG3D* pModel) const {
    s32 shapeIndex = mShapeIndex;

    if (!mIsForceDraw && !pModel->mIsVisible) {
        return false;
    }

    return pModel->isShapeVisible(shapeIndex);
}

/**
 * Allocates the depth shadow visibility flags of all instances.
 */
void MeshDrawer::initForDepthShadow() {
    mDepthShadowFlags.tryAllocBuffer(mMeshNumMax, nullptr);

    for (s32 i = 0; i < mMeshNumMax; i++) {
        mDepthShadowFlags[i].makeAllOne();
    }
}

/**
 * Resets the depth shadow visibility flags of all instances.
 */
void MeshDrawer::clearDepthShadowFlag() {
    for (s32 i = 0; i < mMeshNumMax; i++) {
        mDepthShadowFlags[i].makeAllOne();
    }
}

/**
 * Culls all instances against the depth shadow cascades.
 * @param pDrawer Depth shadow drawer.
 */
void MeshDrawer::preDrawToDepthShadow(DepthShadowDrawer* pDrawer) {
    agl::sdw::DepthShadow* depthShadow = pDrawer->getDepthShadow();

    for (s32 i = 0; i < mMeshNum; i++) {
        Mesh* mesh = mMeshes[i];
        sead::BitFlag32& flag = mDepthShadowFlags[i];
        const SimpleModelG3D* model = mesh->model;

        if (model->isDisableDepthShadow() || !isDrawMesh(model)) {
            continue;
        }

        const_cast<SimpleModelG3D*>(model)->calcBoundingForDepth();
        const nn::g3d::ShapeObj* shape = mesh->shapeObj;

        if (alModelFunction::isExistBoundingNode(shape->GetResource())) {
            flag.makeAllZero();
            s32 subMeshNum = shape->GetResource()->GetMesh()->GetSubMeshCount();
            const nn::g3d::Aabb* aabb = shape->GetSubMeshBoundingArray();

            for (s32 j = 0; j < subMeshNum; j++) {
                sead::BoundBox3f box;
                box.set({nn::util::VectorGetX(aabb[j].minimum), nn::util::VectorGetY(aabb[j].minimum),
                         nn::util::VectorGetZ(aabb[j].minimum)},
                        {nn::util::VectorGetX(aabb[j].maximum), nn::util::VectorGetY(aabb[j].maximum),
                         nn::util::VectorGetZ(aabb[j].maximum)});
                flag.set(depthShadow->checkBox(box, sead::CoreInfo::getCurrentCoreId(), 0));
            }

            continue;
        }

        const nn::g3d::Sphere* bounding = shape->GetBounding();

        if (!bounding) {
            bounding = mesh->modelObj->GetBounding();
        }

        if (bounding) {
            sead::Sphere<sead::Vector3f> sphere(
                {nn::util::VectorGetX(bounding->center), nn::util::VectorGetY(bounding->center),
                 nn::util::VectorGetZ(bounding->center)},
                bounding->radius);
            flag.set(depthShadow->checkSphere(sphere, sead::CoreInfo::getCurrentCoreId(), 0));
        } else {
            flag.makeAllOne();
        }
    }
}

/**
 * Compares the draw order of two mesh drawers.
 * @param rOther Other mesh drawer.
 * @return Whether this drawer is drawn before the other one.
 */
bool MeshDrawer::operator<(const MeshDrawer& rOther) const {
    if (mDrawPriority == rOther.mDrawPriority) {
        return mShapeIndex < rOther.mShapeIndex;
    }

    return mDrawPriority < rOther.mDrawPriority;
}

/**
 * Compares the draw order of two mesh drawers.
 * @param rOther Other mesh drawer.
 * @return Whether this drawer is drawn after the other one.
 */
bool MeshDrawer::operator>(const MeshDrawer& rOther) const {
    if (mDrawPriority == rOther.mDrawPriority) {
        return mShapeIndex > rOther.mShapeIndex;
    }

    return mDrawPriority > rOther.mDrawPriority;
}

static void activateRenderState(agl::DrawContext* pContext, const nn::g3d::MaterialObj* pMaterial,
                                s32 type, bool isBlend) {
    if (type == 1) {
        sead::GraphicsContext context;
        setBlendCtrlToContext(&context, pMaterial, isBlend);
        setDepthCtrlToContext(&context, pMaterial);
        setPolygonCtrlToContext(&context, pMaterial);
        setAlphaTestToContext(&context, pMaterial);
        setPolygonOffsetToContext(pContext, &context, pMaterial, 0.0f);
        context.apply(pContext);
    } else if (type == 2) {
        sead::GraphicsContext context;
        context.setColorMask(0, false, false, false, false);
        context.setBlendEnable(false);
        setDepthCtrlToContext(&context, pMaterial);
        setPolygonCtrlToContext(&context, pMaterial);
        context.setPolygonOffsetFrontEnable(true);
        context.apply(pContext);
    }
}

static void activateOptionBlock(agl::DrawContext* pContext,
                                const nn::g3d::ShaderSelector* pSelector) {
    const nn::g3d::ShadingModelObj* shadingModel = pSelector->GetShadingModel();

    if (!shadingModel->IsBlockBufferValid()) {
        return;
    }

    s32 blockIndex = shadingModel->GetResource()->GetOptionBlockIndex();
    size_t size = blockIndex >= 0 ? shadingModel->GetResource()->GetUniformBlockSize(blockIndex) : 0;
    s32 vertexLocation =
        pSelector->GetProgram()->GetUniformBlockLocation(blockIndex, nn::g3d::Stage_Vertex);
    s32 pixelLocation =
        pSelector->GetProgram()->GetUniformBlockLocation(blockIndex, nn::g3d::Stage_Pixel);
    if (vertexLocation >= 0) {
        agl::ShaderLocation location;
        location.setLocation(vertexLocation);
        agl::g3d::ShaderUtilG3D::load(pContext, location, *shadingModel->GetOptionBlock(), size,
                                      0);
    }

    if (pixelLocation >= 0) {
        agl::ShaderLocation location;
        location.setLocation(pixelLocation);
        agl::g3d::ShaderUtilG3D::load(pContext, location, *shadingModel->GetOptionBlock(), size,
                                      0);
    }
}

/**
 * Records a display list activating the render state, shader and material of the shape.
 * @param pAllocator Allocator of the display list memory.
 * @param renderStateType How the render state is activated.
 * @param textureType How the textures are activated.
 * @param materialType How the material uniform block is activated.
 * @param isBlend Whether blending is enabled when the render state is taken from the material.
 */
void MeshDrawer::createDisplayList(GpuMemAllocator* pAllocator,
                                   RENDER_STATE_ACTIVATE_TYPE renderStateType,
                                   TEXTURE_ACTIVATE_TYPE textureType,
                                   MATERIAL_ACTIVATE_TYPE materialType, bool isBlend) {
    mRenderStateType = renderStateType;
    mTextureType = textureType;
    mMaterialType = materialType;
    mDisplayList = new agl::DisplayList();
    sead::Graphics::instance()->lockDrawContext();
    {
        agl::DrawContext context;
        context.setCommandBuffer(mDisplayList);
        agl::GPUMemAddr<u8> buffer = pAllocator->allocMemory("DisplayList", 0x400, 4);
        mDisplayList->beginDisplayListBuffer(buffer, 0x400, true);
        activateRenderState(&context, mMaterialObj, mRenderStateType, isBlend);
        activateOptionBlock(&context, mShaderSelector);
        mShaderAssign->getResShaderProgram()->Load(getCommandBuffer(&context));
        mShaderAssign->getAttribute().activateVertexAttribute(&context);
        mShaderAssign->getAttribute().activateVertexBuffer(&context);

        if (mTextureType == 1) {
            mShaderAssign->getSampler().activate(&context, mMaterialObj);
        }

        if (mMaterialType == 1) {
            mShaderAssign->activateMaterialUniformBlock(&context, mMaterialObj, 0);
        }

        mDisplayList->endDisplayList();
    }

    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * Draws the shape of all visible instances.
 * @param pViewVolume View volume used for culling, may be nullptr.
 * @param viewIndex View index.
 * @param pAdditionalInfo Additional info used to activate environment textures, may be nullptr.
 */
void MeshDrawer::draw(const nn::g3d::ViewVolume* pViewVolume, s32 viewIndex,
                      ModelAdditionalInfo* pAdditionalInfo) const {
    if (mMeshNum < 1) {
        return;
    }

    if (mUniformRegisterBuffer || mUniformRegisterBuffer2) {
        tryChangeShaderMode(getDrawContext(), agl::cShaderMode_UniformBlock);
    }

    if (mDisplayList) {
        nvnCommandBufferCallCommands(agl::driver::getNvnCommandBuffer(getDrawContext()), 1,
                                     mDisplayList->getHandlePtr());
    } else {
        activateRenderState(getDrawContext(), mMaterialObj, mRenderStateType, false);
        activateOptionBlock(getDrawContext(), mShaderSelector);
        agl::DrawContext* context = getDrawContext();
        mShaderAssign->getResShaderProgram()->Load(getCommandBuffer(context));
        mShaderAssign->getAttribute().activateVertexAttribute(context);
        mShaderAssign->getAttribute().activateVertexBuffer(context);

        if (mTextureType == 1) {
            mShaderAssign->getSampler().activate(getDrawContext(), mMaterialObj);
        }

        if (mMaterialType == 1) {
            mShaderAssign->activateMaterialUniformBlock(getDrawContext(), mMaterialObj, 0);
        }
    }

    const EnvTexInfo* prevEnvTexInfo = nullptr;

    for (s32 i = 0; i < mMeshNum; i++) {
        Mesh* mesh = mMeshes[i];
        const SimpleModelG3D* model = mesh->model;

        if (model->isDisableDraw()) {
            continue;
        }

        s32 lodIndex = model->mLodIndex;

        if (lodIndex >= mesh->shapeObj->GetResource()->GetMeshCount()) {
            continue;
        }

        if (!isDrawMesh(model)) {
            continue;
        }

        nn::g3d::MaterialObj* material = const_cast<nn::g3d::MaterialObj*>(mesh->materialObj);
        const nn::g3d::SkeletonObj* skeleton = mesh->modelObj->GetSkeleton();
        s32 bufferIndex = *model->mCurrentBufferIndex;

        if (*reinterpret_cast<const u8*>(model->getResRenderState(mShapeIndex))) {
            model->useCustomRenderState(getDrawContext(), material, mShapeIndex);
        }

        agl::DrawContext* context = getDrawContext();

        if (mTextureType == 0 || model->mIsForceActivateTexture) {
            mShaderAssign->getSampler().activate(context, material);
        }

        if (mMaterialType == 0) {
            mShaderAssign->activateMaterialUniformBlock(context, material, bufferIndex);
        }

        if (mShapeBlockLocation >= 0) {
            agl::ShaderLocation location;
            location.setLocation(mShapeBlockLocation);
            agl::g3d::ShaderUtilG3D::load(getDrawContext(), location,
                                          static_cast<const nn::gfx::Buffer&>(
                                              *mesh->shapeObj->GetShapeBlock(viewIndex, bufferIndex)),
                                          0x100, bufferIndex);
        }

        if (mSkeletonBlockLocation >= 0) {
            agl::ShaderLocation location;
            location.setLocation(mSkeletonBlockLocation);
            agl::g3d::ShaderUtilG3D::load(getDrawContext(), location,
                                          *skeleton->GetMtxBlock(bufferIndex),
                                          skeleton->GetMtxBlockSize(), bufferIndex);
        }

        if (pAdditionalInfo) {
            pAdditionalInfo->activateEnvTexture(mShapeIndex, model);

            if (mIsUsingModelLight) {
                if (!prevEnvTexInfo ||
                    EnvTexId::isEnableTexId(prevEnvTexInfo->getCubeMapId()) !=
                        EnvTexId::isEnableTexId(
                            model->getShape(mShapeIndex).mEnvTexInfo->getCubeMapId())) {
                    pAdditionalInfo->activateModelLightTexture(mShapeIndex, model);
                    prevEnvTexInfo = model->getShape(mShapeIndex).mEnvTexInfo;
                }
            }
        }

        activateUniformBlockAssignArray(*model->mUniformBlockAssignArray);
        const nn::g3d::ShapeObj* shape = mesh->shapeObj;
        bool isExistBounding = alModelFunction::isExistBoundingNode(shape->GetResource());

        if (pViewVolume && isExistBounding) {
            nn::g3d::CullingContext cullingContext;

            while (shape->TestSubMeshIntersection(&cullingContext, *pViewVolume, lodIndex)) {
                shape->GetResource()->GetMesh(lodIndex)->DrawSubMesh(
                    getCommandBuffer(getDrawContext()), cullingContext.submeshIndex,
                    cullingContext.submeshCount, 1);
            }
        } else {
            const nn::g3d::ResMesh* resMesh = shape->GetResource()->GetMesh(lodIndex);
            resMesh->DrawSubMesh(getCommandBuffer(getDrawContext()), 0,
                                 resMesh->GetSubMeshCount(), 1);
        }
    }

    if (mUniformRegisterBuffer || mUniformRegisterBuffer2) {
        tryChangeShaderMode(getDrawContext(), agl::cShaderMode_UniformRegister);
    }
}

/**
 * Draws the shape of all visible instances without environment textures.
 * @param pViewVolume View volume used for culling, may be nullptr.
 * @param viewIndex View index.
 */
void MeshDrawer::drawTest(const nn::g3d::ViewVolume* pViewVolume, s32 viewIndex) const {
    if (mMeshNum < 1) {
        return;
    }

    if (mUniformRegisterBuffer || mUniformRegisterBuffer2) {
        tryChangeShaderMode(getDrawContext(), agl::cShaderMode_UniformBlock);
    }

    if (mDisplayList) {
        nvnCommandBufferCallCommands(agl::driver::getNvnCommandBuffer(getDrawContext()), 1,
                                     mDisplayList->getHandlePtr());
    } else {
        agl::DrawContext* context = getDrawContext();
        mShaderAssign->getResShaderProgram()->Load(getCommandBuffer(context));
        mShaderAssign->getAttribute().activateVertexAttribute(context);
        mShaderAssign->getAttribute().activateVertexBuffer(context);
    }

    for (s32 i = 0; i < mMeshNum; i++) {
        Mesh* mesh = mMeshes[i];
        const SimpleModelG3D* model = mesh->model;
        const nn::g3d::ResRenderState* renderState = model->getResRenderState(mShapeIndex);
        s32 lodIndex = model->mLodIndex;

        if (lodIndex >= mesh->shapeObj->GetResource()->GetMeshCount()) {
            continue;
        }

        if (!renderState || !isDrawMesh(model)) {
            continue;
        }

        nn::g3d::MaterialObj* material = const_cast<nn::g3d::MaterialObj*>(mesh->materialObj);
        const nn::g3d::SkeletonObj* skeleton = mesh->modelObj->GetSkeleton();
        s32 bufferIndex = *model->mCurrentBufferIndex;
        agl::DrawContext* context = getDrawContext();

        if (mTextureType == 0) {
            mShaderAssign->getSampler().activate(context, material);
        }

        if (mMaterialType == 0) {
            mShaderAssign->activateMaterialUniformBlock(context, material, bufferIndex);
        }

        if (mShapeBlockLocation >= 0) {
            agl::ShaderLocation location;
            location.setLocation(mShapeBlockLocation);
            agl::g3d::ShaderUtilG3D::load(getDrawContext(), location,
                                          static_cast<const nn::gfx::Buffer&>(
                                              *mesh->shapeObj->GetShapeBlock(viewIndex, bufferIndex)),
                                          0x100, bufferIndex);
        }

        if (mSkeletonBlockLocation >= 0) {
            agl::ShaderLocation location;
            location.setLocation(mSkeletonBlockLocation);
            agl::g3d::ShaderUtilG3D::load(getDrawContext(), location,
                                          *skeleton->GetMtxBlock(bufferIndex),
                                          skeleton->GetMtxBlockSize(), bufferIndex);
        }

        const nn::g3d::ShapeObj* shape = mesh->shapeObj;
        bool isExistBounding = alModelFunction::isExistBoundingNode(shape->GetResource());

        if (pViewVolume && isExistBounding) {
            nn::g3d::CullingContext cullingContext;

            while (shape->TestSubMeshIntersection(&cullingContext, *pViewVolume, lodIndex)) {
                shape->GetResource()->GetMesh(lodIndex)->DrawSubMesh(
                    getCommandBuffer(getDrawContext()), cullingContext.submeshIndex,
                    cullingContext.submeshCount, 1);
            }
        } else {
            const nn::g3d::ResMesh* resMesh = shape->GetResource()->GetMesh(lodIndex);
            resMesh->DrawSubMesh(getCommandBuffer(getDrawContext()), 0,
                                 resMesh->GetSubMeshCount(), 1);
        }
    }

    if (mUniformRegisterBuffer || mUniformRegisterBuffer2) {
        tryChangeShaderMode(getDrawContext(), agl::cShaderMode_UniformRegister);
    }
}

/**
 * Draws the shape of all visible instances with a simple shader setup.
 * @param pViewVolume View volume used for culling, may be nullptr.
 */
void MeshDrawer::drawSimple(const nn::g3d::ViewVolume* pViewVolume) const {
    if (mMeshNum < 1) {
        return;
    }

    mShaderAssign->getResShaderProgram()->Load(getCommandBuffer(getDrawContext()));

    for (s32 i = 0; i < mMeshNum; i++) {
        Mesh* mesh = mMeshes[i];
        const SimpleModelG3D* model = mesh->model;

        if (model->isDisableDraw() || !isDrawMesh(model)) {
            continue;
        }

        activateUniformBlockAssignArray(*mesh->model->mUniformBlockAssignArray);
        alModelFunction::drawModelShape(
            mesh->modelObj->GetSkeleton(), mesh->materialObj, mesh->shapeObj,
            mShaderSelector->GetShadingModel(), mShaderAssign, pViewVolume, 0,
            *mesh->model->mCurrentBufferIndex, mesh->model->mLodIndex);
    }
}

/**
 * Draws the shape of all visible instances into the depth buffer.
 * @param pViewVolume View volume used for culling, may be nullptr.
 * @param viewIndex View index.
 */
void MeshDrawer::drawDepthOnly(const nn::g3d::ViewVolume* pViewVolume, s32 viewIndex) const {
    if (mMeshNum < 1) {
        return;
    }

    agl::DrawContext* drawContext = getDrawContext();
    mShaderAssign->getResShaderProgram()->Load(getCommandBuffer(drawContext));
    mShaderAssign->getAttribute().activateVertexAttribute(drawContext);
    mShaderAssign->getAttribute().activateVertexBuffer(drawContext);

    for (s32 i = 0; i < mMeshNum; i++) {
        Mesh* mesh = mMeshes[i];
        const SimpleModelG3D* model = mesh->model;

        if (model->isDisableDraw()) {
            continue;
        }

        s32 lodIndex = model->mLodIndex;

        if (lodIndex >= mesh->shapeObj->GetResource()->GetMeshCount()) {
            continue;
        }

        if (!isDrawMesh(model)) {
            continue;
        }

        const nn::g3d::MaterialObj* material = mesh->materialObj;
        const nn::g3d::SkeletonObj* skeleton = mesh->modelObj->GetSkeleton();
        s32 bufferIndex = *model->mCurrentBufferIndex;
        agl::DrawContext* context = getDrawContext();

        if (mIsAlphaTest) {
            mShaderAssign->getSampler().activate(context, material);
        }

        mShaderAssign->activateMaterialUniformBlock(context, material, bufferIndex);
        activateUniformBlockAssignArray(*mesh->model->mUniformBlockAssignArray);

        if (mShapeBlockLocation >= 0) {
            agl::ShaderLocation location;
            location.setLocation(mShapeBlockLocation);
            agl::g3d::ShaderUtilG3D::load(getDrawContext(), location,
                                          static_cast<const nn::gfx::Buffer&>(
                                              *mesh->shapeObj->GetShapeBlock(viewIndex, bufferIndex)),
                                          0x100, bufferIndex);
        }

        if (mSkeletonBlockLocation >= 0) {
            agl::ShaderLocation location;
            location.setLocation(mSkeletonBlockLocation);
            agl::g3d::ShaderUtilG3D::load(getDrawContext(), location,
                                          *skeleton->GetMtxBlock(bufferIndex),
                                          skeleton->GetMtxBlockSize(), bufferIndex);
        }

        const nn::g3d::ShapeObj* shape = mesh->shapeObj;
        bool isExistBounding = alModelFunction::isExistBoundingNode(shape->GetResource());

        if (pViewVolume && isExistBounding) {
            nn::g3d::CullingContext cullingContext;

            while (shape->TestSubMeshIntersection(&cullingContext, *pViewVolume, lodIndex)) {
                shape->GetResource()->GetMesh(lodIndex)->DrawSubMesh(
                    getCommandBuffer(getDrawContext()), cullingContext.submeshIndex,
                    cullingContext.submeshCount, 1);
            }
        } else {
            const nn::g3d::ResMesh* resMesh = shape->GetResource()->GetMesh(lodIndex);
            resMesh->DrawSubMesh(getCommandBuffer(getDrawContext()), 0,
                                 resMesh->GetSubMeshCount(), 1);
        }
    }
}

/**
 * Draws the shape of all instances visible in a depth shadow cascade.
 * @param pViewVolume View volume used for culling, may be nullptr.
 * @param viewIndex View index.
 * @param shadowIndex Index of the depth shadow cascade.
 */
void MeshDrawer::drawDepthShadow(const nn::g3d::ViewVolume* pViewVolume, s32 viewIndex,
                                 s32 shadowIndex) const {
    if (mMeshNum < 1) {
        return;
    }

    agl::DrawContext* drawContext = getDrawContext();

    if (mDisplayList) {
        nvnCommandBufferCallCommands(agl::driver::getNvnCommandBuffer(drawContext), 1,
                                     mDisplayList->getHandlePtr());
    } else {
        mShaderAssign->getResShaderProgram()->Load(getCommandBuffer(drawContext));
        mShaderAssign->getAttribute().activateVertexAttribute(drawContext);
        mShaderAssign->getAttribute().activateVertexBuffer(drawContext);
    }

    u32 shadowBit = 1 << shadowIndex;

    for (s32 i = 0; i < mMeshNum; i++) {
        Mesh* mesh = mMeshes[i];
        const SimpleModelG3D* model = mesh->model;
        s32 lodIndex = model->mLodIndex;

        if (lodIndex < model->getModelObj()->GetLodCount() - 1) {
            lodIndex++;
        }

        if (model->isDisableDepthShadow()) {
            continue;
        }

        if (lodIndex >= mesh->shapeObj->GetResource()->GetMeshCount()) {
            continue;
        }

        if (!mDepthShadowFlags[i].isOn(shadowBit)) {
            continue;
        }

        if (!isDrawMesh(model)) {
            continue;
        }

        const nn::g3d::MaterialObj* material = mesh->materialObj;
        const nn::g3d::SkeletonObj* skeleton = mesh->modelObj->GetSkeleton();
        s32 bufferIndex = *model->mCurrentBufferIndex;

        if (mRenderStateType == 0) {
            sead::GraphicsContext context;
        }

        agl::DrawContext* context = getDrawContext();

        if (mTextureType == 0 || model->mIsForceActivateTexture) {
            mShaderAssign->getSampler().activate(context, material);
        }

        if (mMaterialType == 0) {
            mShaderAssign->activateMaterialUniformBlock(context, material, bufferIndex);
        }

        activateUniformBlockAssignArray(*mesh->model->mUniformBlockAssignArray);

        if (mShapeBlockLocation >= 0) {
            agl::ShaderLocation location;
            location.setLocation(mShapeBlockLocation);
            agl::g3d::ShaderUtilG3D::load(getDrawContext(), location,
                                          static_cast<const nn::gfx::Buffer&>(
                                              *mesh->shapeObj->GetShapeBlock(viewIndex, bufferIndex)),
                                          0x100, bufferIndex);
        }

        if (mSkeletonBlockLocation >= 0) {
            agl::ShaderLocation location;
            location.setLocation(mSkeletonBlockLocation);
            agl::g3d::ShaderUtilG3D::load(getDrawContext(), location,
                                          *skeleton->GetMtxBlock(bufferIndex),
                                          skeleton->GetMtxBlockSize(), bufferIndex);
        }

        const nn::g3d::ShapeObj* shape = mesh->shapeObj;
        bool isExistBounding = alModelFunction::isExistBoundingNode(shape->GetResource());

        if (pViewVolume && isExistBounding) {
            nn::g3d::CullingContext cullingContext;

            while (shape->TestSubMeshIntersection(&cullingContext, *pViewVolume, lodIndex)) {
                shape->GetResource()->GetMesh(lodIndex)->DrawSubMesh(
                    getCommandBuffer(getDrawContext()), cullingContext.submeshIndex,
                    cullingContext.submeshCount, 1);
            }
        } else {
            const nn::g3d::ResMesh* resMesh = shape->GetResource()->GetMesh(lodIndex);
            resMesh->DrawSubMesh(getCommandBuffer(getDrawContext()), 0,
                                 resMesh->GetSubMeshCount(), 1);
        }
    }
}

/**
 * Checks whether any instance of the shape is visible.
 * @return Whether any instance is drawn.
 */
bool MeshDrawer::isExistDrawMesh() const {
    for (s32 i = 0; i < mMeshNum; i++) {
        if (isDrawMesh(mMeshes[i]->model)) {
            return true;
        }
    }

    return false;
}

/**
 * Adds an instance of the shape unless it was already added.
 * @param pModelObj Model object of the instance.
 * @param pShapeObj Shape object of the instance.
 * @param pModel Model of the instance.
 */
void MeshDrawer::addMesh(const nn::g3d::ModelObj* pModelObj, const nn::g3d::ShapeObj* pShapeObj,
                         const SimpleModelG3D* pModel) {
    for (s32 i = 0; i < mMeshNum; i++) {
        if (mMeshes[i]->modelObj == pModelObj && mMeshes[i]->shapeObj == pShapeObj) {
            return;
        }
    }

    Mesh* mesh = mMeshes[mMeshNum];
    mesh->modelObj = pModelObj;
    mesh->shapeObj = pShapeObj;
    mesh->materialObj = pModelObj->GetMaterial(pShapeObj->GetResource()->GetMaterialIndex());
    mesh->model = pModel;
    mMeshes[mMeshNum] = mesh;
    mMeshNum++;
}

/**
 * Removes an instance of the shape.
 * @param pModelObj Model object of the instance.
 * @param pShapeObj Shape object of the instance.
 */
void MeshDrawer::removeMesh(const nn::g3d::ModelObj* pModelObj,
                            const nn::g3d::ShapeObj* pShapeObj) {
    for (s32 i = 0; i < mMeshNum; i++) {
        Mesh* mesh = mMeshes[i];

        if (mesh->modelObj == pModelObj && mesh->shapeObj == pShapeObj) {
            for (; i < mMeshNum - 1; i++) {
                mMeshes[i] = mMeshes[i + 1];
            }

            mMeshNum--;
            mMeshes[mMeshNum] = mesh;
            return;
        }
    }
}

/**
 * Inserts a mesh drawer sorted by draw priority.
 * @param pDrawer Mesh drawer.
 */
void MeshDrawerTable::insert(MeshDrawer* pDrawer) {
    for (s32 i = 0; i < size(); i++) {
        if (unsafeAt(i)->getDrawPriority() > pDrawer->getDrawPriority()) {
            sead::PtrArray<MeshDrawer>::insert(i, pDrawer);
            return;
        }
    }

    pushBack(pDrawer);
}

}  // namespace al
