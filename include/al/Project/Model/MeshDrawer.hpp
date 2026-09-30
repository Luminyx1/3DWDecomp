#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <prim/seadBitFlag.h>

namespace nn::g3d {
class MaterialObj;
class ModelObj;
class ShaderSelector;
class ShapeObj;
class ViewVolume;
}  // namespace nn::g3d

namespace agl {
class DisplayList;
class DrawContext;
}  // namespace agl

namespace al {
class DepthShadowDrawer;
class GpuMemAllocator;
class ModelAdditionalInfo;
class ModelShaderAssign;
class SimpleModelG3D;

class MeshDrawer {
public:
    enum RENDER_STATE_ACTIVATE_TYPE : s32 {};
    enum TEXTURE_ACTIVATE_TYPE : s32 {};
    enum MATERIAL_ACTIVATE_TYPE : s32 {};

    struct Mesh {
        const nn::g3d::ModelObj* modelObj = nullptr;
        const nn::g3d::ShapeObj* shapeObj = nullptr;
        const nn::g3d::MaterialObj* materialObj = nullptr;
        const SimpleModelG3D* model = nullptr;
    };

    MeshDrawer(const char* pName, const nn::g3d::ModelObj* pModelObj,
               const nn::g3d::ShapeObj* pShapeObj, const nn::g3d::ShaderSelector* pSelector,
               s32 meshNum);

    void initForDepthShadow();
    void clearDepthShadowFlag();
    void preDrawToDepthShadow(DepthShadowDrawer* pDrawer);
    bool operator<(const MeshDrawer& rOther) const;
    bool operator>(const MeshDrawer& rOther) const;
    void createDisplayList(GpuMemAllocator* pAllocator, RENDER_STATE_ACTIVATE_TYPE renderStateType,
                           TEXTURE_ACTIVATE_TYPE textureType, MATERIAL_ACTIVATE_TYPE materialType,
                           bool);
    void draw(const nn::g3d::ViewVolume* pViewVolume, s32 viewIndex,
              ModelAdditionalInfo* pAdditionalInfo) const;
    void drawTest(const nn::g3d::ViewVolume* pViewVolume, s32 viewIndex) const;
    void drawSimple(const nn::g3d::ViewVolume* pViewVolume) const;
    void drawDepthOnly(const nn::g3d::ViewVolume* pViewVolume, s32 viewIndex) const;
    void drawDepthShadow(const nn::g3d::ViewVolume* pViewVolume, s32 viewIndex, s32) const;
    bool isExistDrawMesh() const;
    void addMesh(const nn::g3d::ModelObj* pModelObj, const nn::g3d::ShapeObj* pShapeObj,
                 const SimpleModelG3D* pModel);
    void removeMesh(const nn::g3d::ModelObj* pModelObj, const nn::g3d::ShapeObj* pShapeObj);

    s32 getShapeIndex() const { return mShapeIndex; }

    void setForceDraw() { mIsForceDraw = true; }

    void setAlphaTest() { mIsAlphaTest = true; }

    const nn::g3d::MaterialObj* getMaterialObj() const { return mMaterialObj; }

    s32 getDrawPriority() const { return mDrawPriority; }

private:
    bool isDrawMesh(const SimpleModelG3D* pModel) const;

    const char* mName;
    const nn::g3d::ModelObj* mModelObj;
    const nn::g3d::ShapeObj* mShapeObj;
    const nn::g3d::MaterialObj* mMaterialObj = nullptr;
    ModelShaderAssign* mShaderAssign = nullptr;
    const nn::g3d::ShaderSelector* mShaderSelector;
    s32 mShapeIndex = 0;
    RENDER_STATE_ACTIVATE_TYPE mRenderStateType = RENDER_STATE_ACTIVATE_TYPE(0);
    TEXTURE_ACTIVATE_TYPE mTextureType = TEXTURE_ACTIVATE_TYPE(0);
    MATERIAL_ACTIVATE_TYPE mMaterialType = MATERIAL_ACTIVATE_TYPE(0);
    bool mIsForceDraw = false;
    bool mIsUsingModelLight = false;
    bool mIsAlphaTest = false;
    s32 mMeshNumMax = 0;
    s32 mMeshNum = 0;
    Mesh** mMeshes = nullptr;
    s32 mShapeBlockLocation = -1;
    s32 mSkeletonBlockLocation = -1;
    u32 mUniformRegisterSize = 0;
    u8* mUniformRegisterBuffer = nullptr;
    u32 mUniformRegisterSize2 = 0;
    u8* mUniformRegisterBuffer2 = nullptr;
    agl::DisplayList* mDisplayList = nullptr;
    s32 mDrawPriority = 0;
    sead::Buffer<sead::BitFlag32> mDepthShadowFlags;
};

static_assert(sizeof(MeshDrawer) == 0xa0);

class MeshDrawerTable : public sead::PtrArray<MeshDrawer> {
public:
    void insert(MeshDrawer* pDrawer);
};

}  // namespace al
