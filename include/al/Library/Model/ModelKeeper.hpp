#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

class alModelCafe;

namespace al {
class GpuMemAllocator;

class ModelKeeper {
public:
    ModelKeeper();
    ~ModelKeeper();

    void setGlobalAlpha(f32* pAlpha);
    void setGlobalYOffset(f32* pYOffset);
    void initResource(const char* pModelArcName, const char* pAnimArcName, const char* pSuffix);
    void setModel(const char* pName, alModelCafe* pModel);
    void initModel(s32 bufferNum, GpuMemAllocator* pAllocator);
    void show();
    void hide();
    void update();
    void updatePaused();
    void calc(const sead::Matrix34f& rMtx, const sead::Vector3f& rScale);
    void setLodParams(const f32* pLodDistances, s32 lodNum, const sead::Vector3f* pPos, bool isFlag);
    void updateLod(const sead::Vector3f& rPos, bool isFlag);
    s32 getLodNum() const;
    bool setDisableDraw(bool isDisable);
    bool setDisableDepthShadow(bool isDisable);
    const sead::Matrix34f* getWorldMtxPtrByIndex(s32 index) const;
    void initLightCtrl(s32 num);

    alModelCafe* getModelCafe() const { return mModelCafe; }
    const char* getModelName() const { return mModelName; }
    bool isFixedModel() const { return mIsFixedModel; }
    bool isIgnoreUpdateDrawClipping() const { return mIsIgnoreUpdateDrawClipping; }
    bool isNeedSetBaseMtxAndCalcAnim() const { return mIsNeedSetBaseMtxAndCalcAnim; }
    void setFixedModel(bool isFixed) { mIsFixedModel = isFixed; }
    void setIgnoreUpdateDrawClipping(bool isIgnore) { mIsIgnoreUpdateDrawClipping = isIgnore; }
    void setNeedSetBaseMtxAndCalcAnim(bool isNeed) { mIsNeedSetBaseMtxAndCalcAnim = isNeed; }
    void setLodDisabled(bool isDisable) { mIsLodDisabled = isDisable; }

    alModelCafe* mModelCafe = nullptr;
    const char* mModelName = nullptr;
    void* _10 = nullptr;
    bool mIsFixedModel = false;
    bool mIsIgnoreUpdateDrawClipping = false;
    bool mIsNeedSetBaseMtxAndCalcAnim = true;
    bool mIsLodDisabled = false;
    bool _1c = false;
};
}  // namespace al
