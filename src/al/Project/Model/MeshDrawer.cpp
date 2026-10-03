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
#include <attributes.h>
#include <shadow/aglDepthShadow.h>

#include "Library/Draw/GraphicsFunction.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Model/ModelShaderAssign.hpp"
#include "Library/Shader/DeferredRendering/ModelLightParam.hpp"
#include "Library/Shader/DeferredRendering/PeripheryRendering.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/EnvTextureKeeper.hpp"
#include "Library/Shadow/Depth/DepthShadowDrawer.hpp"
#include "Project/Draw/GpuMemAllocator.hpp"
#include "Project/Draw/RenderState.hpp"
#include "Project/Model/ModelAdditionalInfo.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {

/**
 * Gets the draw context of the game framework.
 * @return The draw context.
 */
static agl::DrawContext* getDrawContext() {
    return GameFrameworkNx::getAglDrawContext();
}

/**
 * Gets the command buffer of a draw context.
 * @param pContext Draw context.
 * @return The command buffer.
 */
static nn::gfx::CommandBuffer* getCommandBuffer(agl::DrawContext* pContext) {
    return reinterpret_cast<nn::gfx::CommandBuffer*>(reinterpret_cast<uintptr_t>(pContext) + 8);
}

/**
 * Reinterprets a vector of nn::util as a sead vector.
 * @param rVector Vector.
 * @return The same vector as sead::Vector3f.
 */
static const sead::Vector3f& toVector3f(const nn::util::Vector3fType& rVector) {
    return reinterpret_cast<const sead::Vector3f&>(rVector);
}

/**
 * Loads the shader program and activates the vertex attributes and buffers.
 * @param pContext Draw context.
 * @param pShaderAssign Shader assign.
 */
static ALWAYS_INLINE void activateShader(agl::DrawContext* pContext,
                                         const ModelShaderAssign* pShaderAssign) {
    pShaderAssign->getResShaderProgram()->Load(getCommandBuffer(pContext));
    const agl::g3d::ModelShaderAttribute& attribute = pShaderAssign->getAttribute();
    attribute.activateVertexAttribute(pContext);
    attribute.activateVertexBuffer(pContext);
}

/**
 * Activates the textures and the uniform block of a material.
 * @param pContext Draw context.
 * @param pShaderAssign Shader assign.
 * @param pMaterial Material.
 * @param isActivateTexture Whether the textures are activated.
 * @param isActivateUniformBlock Whether the material uniform block is activated.
 * @param bufferIndex GPU buffer index.
 */
static ALWAYS_INLINE void activateMaterial(agl::DrawContext* pContext,
                                           const ModelShaderAssign* pShaderAssign,
                                           const nn::g3d::MaterialObj* pMaterial,
                                           bool isActivateTexture, bool isActivateUniformBlock,
                                           s32 bufferIndex) {
    if (isActivateTexture) {
        pShaderAssign->getSampler().activate(pContext, pMaterial);
    }

    if (isActivateUniformBlock) {
        pShaderAssign->activateMaterialUniformBlock(pContext, pMaterial, bufferIndex);
    }
}

/**
 * Checks a bit of a packed bit array.
 * @param pArray Bit array.
 * @param index Bit index.
 * @return Whether the bit is set.
 */
static bool isBitOn(const u32* pArray, s32 index) {
    return (pArray[index >> 5] & (1 << index)) != 0;
}

/**
 * Draws the sub meshes of a shape, culled by the view volume if the shape has bounding nodes.
 * @param pShape Shape object.
 * @param pViewVolume View volume, may be nullptr.
 * @param lodIndex Level of detail.
 */
static ALWAYS_INLINE void drawShape(const nn::g3d::ShapeObj* pShape,
                                    const nn::g3d::ViewVolume* pViewVolume, s32 lodIndex) {
    bool isExistBounding = alModelFunction::isExistBoundingNode(pShape->GetResource());

    if (pViewVolume != nullptr && isExistBounding) {
        nn::g3d::CullingContext cullingContext;

        while (pShape->TestSubMeshIntersection(&cullingContext, *pViewVolume, lodIndex)) {
            pShape->GetResource()->GetMesh(lodIndex)->DrawSubMesh(
                getCommandBuffer(getDrawContext()), cullingContext.submeshIndex,
                cullingContext.submeshCount, 1);
        }
    } else {
        const nn::g3d::ResMesh* resMesh = pShape->GetResource()->GetMesh(lodIndex);
        resMesh->DrawSubMesh(getCommandBuffer(getDrawContext()), 0, resMesh->GetSubMeshCount(),
                             1);
    }
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
    const nn::g3d::ResShadingModel* shadingModel =
        mShaderSelector->GetShadingModel()->GetResource();
    mSkeletonBlockLocation = program->GetUniformBlockLocation(
        shadingModel->GetSkeletonBlockIndex(), nn::g3d::Stage_Vertex);
    mShapeBlockLocation = program->GetUniformBlockLocation(shadingModel->GetShapeBlockIndex(),
                                                           nn::g3d::Stage_Vertex);
}

/**
 * Checks whether the shape of a model instance is drawn.
 * @param pModel Model instance.
 * @return Whether the shape is drawn.
 */
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

        if (mesh->model->isDisableDepthShadow() || !isDrawMesh(mesh->model)) {
            continue;
        }

        const_cast<SimpleModelG3D*>(mesh->model)->calcBoundingForDepth();
        const nn::g3d::ShapeObj* shape = mesh->shapeObj;

        if (!alModelFunction::isExistBoundingNode(shape->GetResource())) {
            const nn::g3d::Sphere* bounding = shape->GetBounding();

            if (bounding == nullptr) {
                bounding = mesh->modelObj->GetBounding();
            }

            if (bounding != nullptr) {
                sead::Sphere<sead::Vector3f> sphere;
                sphere.setCenter(toVector3f(bounding->center));
                sphere.setRadius(bounding->radius);
                flag.set(depthShadow->checkSphere(sphere, sead::CoreInfo::getCurrentCoreId(), 0));
            } else {
                flag.makeAllOne();
            }

            continue;
        }

        flag.makeAllZero();
        s32 subMeshNum = shape->GetResource()->GetMesh()->GetSubMeshCount();
        const nn::g3d::Aabb* aabb = shape->GetSubMeshBoundingArray();

        for (s32 j = 0; j < subMeshNum; j++) {
            sead::BoundBox3f box(toVector3f(aabb[j].minimum), toVector3f(aabb[j].maximum));
            flag.set(depthShadow->checkBox(box, sead::CoreInfo::getCurrentCoreId(), 0));
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
                                s32 type, bool isBlend);
static void activateOptionBlock(agl::DrawContext* pContext,
                                const nn::g3d::ShaderSelector* pSelector);

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
        agl::GPUMemAddrBase buffer = pAllocator->allocMemory("DisplayList", 0x400, 4);
        mDisplayList->beginDisplayListBuffer(buffer, 0x400, true);
        activateRenderState(&context, mMaterialObj, mRenderStateType, isBlend);
        activateOptionBlock(&context, mShaderSelector);
        activateShader(&context, mShaderAssign);
        activateMaterial(&context, mShaderAssign, mMaterialObj, mTextureType == 1,
                         mMaterialType == 1, 0);

        mDisplayList->endDisplayList();
    }

    sead::Graphics::instance()->unlockDrawContext();
}

/**
 * Applies the render state of a material.
 * @param pContext Draw context.
 * @param pMaterial Material.
 * @param type 1 applies the full material render state, 2 only the depth and polygon state.
 * @param isBlend Whether blending is enabled.
 */
static void activateRenderState(agl::DrawContext* pContext, const nn::g3d::MaterialObj* pMaterial,
                                s32 type, bool isBlend) {
    switch (type) {
    case 2: {
        sead::GraphicsContext context;
        context.setColorMask(0, false, false, false, false);
        context.setBlendEnable(false);
        setDepthCtrlToContext(&context, pMaterial);
        setPolygonCtrlToContext(&context, pMaterial);
        context.setPolygonOffsetFrontEnable(true);
        context.apply(pContext);
        break;
    }
    case 1: {
        sead::GraphicsContext context;
        setBlendCtrlToContext(&context, pMaterial, isBlend);
        setDepthCtrlToContext(&context, pMaterial);
        setPolygonCtrlToContext(&context, pMaterial);
        setAlphaTestToContext(&context, pMaterial);
        setPolygonOffsetToContext(pContext, &context, pMaterial, 0.0f);
        context.apply(pContext);
        break;
    }
    }
}

/**
 * Loads the option uniform block of a shader selector.
 * @param pContext Draw context.
 * @param pSelector Shader selector.
 */
static void activateOptionBlock(agl::DrawContext* pContext,
                                const nn::g3d::ShaderSelector* pSelector) {
    if (!pSelector->GetShadingModel()->IsBlockBufferValid()) {
        return;
    }

    const nn::g3d::ResShadingModel* shadingModel = pSelector->GetShadingModel()->GetResource();
    s32 blockIndex = shadingModel->GetOptionBlockIndex();
    size_t size = blockIndex >= 0 ? shadingModel->GetUniformBlockSize(blockIndex) : 0;
    const s32* locations = &pSelector->GetProgram()->ToData().pUniformBlockTable.Get()[
        blockIndex * nn::g3d::Stage_Num];
    s32 vertexLocation = locations[nn::g3d::Stage_Vertex];
    s32 pixelLocation = locations[nn::g3d::Stage_Pixel];

    if (vertexLocation >= 0) {
        agl::ShaderLocation location(vertexLocation);
        agl::g3d::ShaderUtilG3D::load(pContext, location,
                                      *pSelector->GetShadingModel()->GetOptionBlock(), size, 0);
    }

    if (pixelLocation >= 0) {
        agl::ShaderLocation location(pixelLocation);
        agl::g3d::ShaderUtilG3D::load(pContext, location,
                                      *pSelector->GetShadingModel()->GetOptionBlock(), size, 0);
    }
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
        tryChangeShaderMode(getDrawContext(), agl::cShaderMode_GeometryShader);
    }

    if (mDisplayList != nullptr) {
        nvnCommandBufferCallCommands(agl::driver::getNvnCommandBuffer(getDrawContext()), 1,
                                     mDisplayList->getHandlePtr());
    } else {
        activateRenderState(getDrawContext(), mMaterialObj, mRenderStateType, false);
        activateOptionBlock(getDrawContext(), mShaderSelector);
        activateShader(getDrawContext(), mShaderAssign);
        activateMaterial(getDrawContext(), mShaderAssign, mMaterialObj, mTextureType == 1,
                         mMaterialType == 1, 0);
    }

    const EnvTexInfo* prevEnvTexInfo = nullptr;

    for (s32 i = 0; i < mMeshNum; i++) {
        Mesh* mesh = mMeshes[i];

        if (mesh->model->isDisableDraw()) {
            continue;
        }

        const nn::g3d::ShapeObj* shape = mesh->shapeObj;
        s32 lodIndex = mesh->model->getLodIndex();

        if (lodIndex >= shape->GetResource()->GetMeshCount()) {
            continue;
        }

        if (!isDrawMesh(mesh->model)) {
            continue;
        }

        nn::g3d::MaterialObj* material = const_cast<nn::g3d::MaterialObj*>(mesh->materialObj);
        const nn::g3d::SkeletonObj* skeleton = mesh->modelObj->GetSkeleton();
        s32 bufferIndex = mesh->model->getCurrentBufferIndex();

        if (mesh->model->getResRenderState(mShapeIndex)->isEnable()) {
            mesh->model->useCustomRenderState(getDrawContext(), material, mShapeIndex);
        }

        activateMaterial(getDrawContext(), mShaderAssign, material,
                         mTextureType == 0 || mesh->model->isForceActivateTexture(),
                         mMaterialType == 0, bufferIndex);

        if (mShapeBlockLocation >= 0) {
            agl::g3d::ShaderUtilG3D::load(
                getDrawContext(), agl::ShaderLocation(mShapeBlockLocation),
                static_cast<const nn::gfx::Buffer&>(*shape->GetShapeBlock(viewIndex, bufferIndex)),
                0x100, bufferIndex);
        }

        if (mSkeletonBlockLocation >= 0) {
            agl::g3d::ShaderUtilG3D::load(getDrawContext(),
                                          agl::ShaderLocation(mSkeletonBlockLocation),
                                          *skeleton->GetMtxBlock(bufferIndex),
                                          skeleton->GetMtxBlockSize(), bufferIndex);
        }

        if (pAdditionalInfo != nullptr) {
            pAdditionalInfo->activateEnvTexture(mShapeIndex, mesh->model);

            if (mIsUsingModelLight) {
                if (prevEnvTexInfo == nullptr ||
                    EnvTexId::isEnableTexId(prevEnvTexInfo->getCubeMapId()) !=
                        EnvTexId::isEnableTexId(
                            mesh->model->getShape(mShapeIndex).mEnvTexInfo->getCubeMapId())) {
                    pAdditionalInfo->activateModelLightTexture(mShapeIndex, mesh->model);
                    prevEnvTexInfo = mesh->model->getShape(mShapeIndex).mEnvTexInfo;
                }
            }
        }

        activateUniformBlockAssignArray(*mesh->model->getUniformBlockAssignArray());
        drawShape(shape, pViewVolume, lodIndex);
    }

    if (mUniformRegisterBuffer || mUniformRegisterBuffer2) {
        tryChangeShaderMode(getDrawContext(), agl::cShaderMode_UniformBlock);
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
        tryChangeShaderMode(getDrawContext(), agl::cShaderMode_GeometryShader);
    }

    if (mDisplayList != nullptr) {
        nvnCommandBufferCallCommands(agl::driver::getNvnCommandBuffer(getDrawContext()), 1,
                                     mDisplayList->getHandlePtr());
    } else {
        activateShader(getDrawContext(), mShaderAssign);
    }

    for (s32 i = 0; i < mMeshNum; i++) {
        Mesh* mesh = mMeshes[i];
        const nn::g3d::ShapeObj* shape = mesh->shapeObj;
        const nn::g3d::SkeletonObj* skeleton = mesh->modelObj->GetSkeleton();
        const nn::g3d::MaterialObj* material = mesh->materialObj;
        // The render state is fetched but not used here.
        mesh->model->getResRenderState(i);
        s32 lodIndex = mesh->model->getLodIndex();

        if (lodIndex >= shape->GetResource()->GetMeshCount()) {
            continue;
        }

        const nn::g3d::ResShape* resShape = shape->GetResource();

        if (!isBitOn(mesh->modelObj->GetMaterialVisibilityArray(), resShape->GetMaterialIndex()) ||
            !isBitOn(mesh->modelObj->GetBoneVisibilityArray(), resShape->GetBoneIndex())) {
            continue;
        }

        s32 bufferIndex = mesh->model != nullptr ? mesh->model->getCurrentBufferIndex() : 0;
        activateMaterial(getDrawContext(), mShaderAssign, material, true, mMaterialType == 0,
                         bufferIndex);

        if (mShapeBlockLocation >= 0) {
            agl::g3d::ShaderUtilG3D::load(
                getDrawContext(), agl::ShaderLocation(mShapeBlockLocation),
                static_cast<const nn::gfx::Buffer&>(*shape->GetShapeBlock(viewIndex, bufferIndex)),
                0x100, bufferIndex);
        }

        if (mSkeletonBlockLocation >= 0) {
            agl::g3d::ShaderUtilG3D::load(getDrawContext(),
                                          agl::ShaderLocation(mSkeletonBlockLocation),
                                          *skeleton->GetMtxBlock(bufferIndex),
                                          skeleton->GetMtxBlockSize(), bufferIndex);
        }

        drawShape(shape, pViewVolume, lodIndex);
    }

    if (mUniformRegisterBuffer || mUniformRegisterBuffer2) {
        tryChangeShaderMode(getDrawContext(), agl::cShaderMode_UniformBlock);
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

        activateUniformBlockAssignArray(*mesh->model->getUniformBlockAssignArray());
        alModelFunction::drawModelShape(
            mesh->modelObj->GetSkeleton(), mesh->materialObj, mesh->shapeObj,
            mShaderSelector->GetShadingModel(), mShaderAssign, pViewVolume, 0,
            mesh->model->getCurrentBufferIndex(), mesh->model->getLodIndex());
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

    activateShader(getDrawContext(), mShaderAssign);

    for (s32 i = 0; i < mMeshNum; i++) {
        Mesh* mesh = mMeshes[i];

        if (mesh->model->isDisableDraw()) {
            continue;
        }

        const nn::g3d::ShapeObj* shape = mesh->shapeObj;
        s32 lodIndex = mesh->model->getLodIndex();

        if (lodIndex >= shape->GetResource()->GetMeshCount()) {
            continue;
        }

        if (!isDrawMesh(mesh->model)) {
            continue;
        }

        const nn::g3d::MaterialObj* material = mesh->materialObj;
        const nn::g3d::SkeletonObj* skeleton = mesh->modelObj->GetSkeleton();
        s32 bufferIndex = mesh->model->getCurrentBufferIndex();
        activateMaterial(getDrawContext(), mShaderAssign, material, mIsAlphaTest, true,
                         bufferIndex);
        activateUniformBlockAssignArray(*mesh->model->getUniformBlockAssignArray());

        if (mShapeBlockLocation >= 0) {
            agl::g3d::ShaderUtilG3D::load(
                getDrawContext(), agl::ShaderLocation(mShapeBlockLocation),
                static_cast<const nn::gfx::Buffer&>(*shape->GetShapeBlock(viewIndex, bufferIndex)),
                0x100, bufferIndex);
        }

        if (mSkeletonBlockLocation >= 0) {
            agl::g3d::ShaderUtilG3D::load(getDrawContext(),
                                          agl::ShaderLocation(mSkeletonBlockLocation),
                                          *skeleton->GetMtxBlock(bufferIndex),
                                          skeleton->GetMtxBlockSize(), bufferIndex);
        }

        drawShape(shape, pViewVolume, lodIndex);
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

    if (mDisplayList != nullptr) {
        nvnCommandBufferCallCommands(agl::driver::getNvnCommandBuffer(getDrawContext()), 1,
                                     mDisplayList->getHandlePtr());
    } else {
        activateShader(getDrawContext(), mShaderAssign);
    }

    u32 shadowBit = 1 << shadowIndex;

    for (s32 i = 0; i < mMeshNum; i++) {
        Mesh* mesh = mMeshes[i];
        s32 lodIndex = mesh->model->getLodIndex();

        if (lodIndex < mesh->model->getModelObj()->GetLodCount() - 1) {
            lodIndex++;
        }

        if (mesh->model->isDisableDepthShadow()) {
            continue;
        }

        const nn::g3d::ShapeObj* shape = mesh->shapeObj;

        if (lodIndex >= shape->GetResource()->GetMeshCount()) {
            continue;
        }

        if (!mDepthShadowFlags[i].isOn(shadowBit)) {
            continue;
        }

        if (!isDrawMesh(mesh->model)) {
            continue;
        }

        const nn::g3d::SkeletonObj* skeleton = mesh->modelObj->GetSkeleton();
        const nn::g3d::MaterialObj* material = mesh->materialObj;
        s32 bufferIndex = mesh->model->getCurrentBufferIndex();

        if (mRenderStateType == 0) {
            sead::GraphicsContext context;
        }

        activateMaterial(getDrawContext(), mShaderAssign, material,
                         mTextureType == 0 || mesh->model->isForceActivateTexture(),
                         mMaterialType == 0, bufferIndex);
        activateUniformBlockAssignArray(*mesh->model->getUniformBlockAssignArray());

        if (mShapeBlockLocation >= 0) {
            agl::g3d::ShaderUtilG3D::load(
                getDrawContext(), agl::ShaderLocation(mShapeBlockLocation),
                static_cast<const nn::gfx::Buffer&>(*shape->GetShapeBlock(viewIndex, bufferIndex)),
                0x100, bufferIndex);
        }

        if (mSkeletonBlockLocation >= 0) {
            agl::g3d::ShaderUtilG3D::load(getDrawContext(),
                                          agl::ShaderLocation(mSkeletonBlockLocation),
                                          *skeleton->GetMtxBlock(bufferIndex),
                                          skeleton->GetMtxBlockSize(), bufferIndex);
        }

        drawShape(shape, pViewVolume, lodIndex);
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
    s32 drawPriority = pDrawer->getDrawPriority();

    for (s32 i = 0; i < size(); i++) {
        if (unsafeAt(i)->getDrawPriority() > drawPriority) {
            sead::PtrArray<MeshDrawer>::insert(i, pDrawer);
            return;
        }
    }

    pushBack(pDrawer);
}

}  // namespace al
