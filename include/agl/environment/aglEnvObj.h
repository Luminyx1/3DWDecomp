#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <prim/seadSafeString.h>

#include "utility/aglNamedObj.h"
#include "utility/aglNamedObjIndex.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterObj.h"

namespace sead {
class Heap;
namespace hostio {
class Context;
class NodeEvent;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
}

namespace agl::env {

class EnvObjMgr;

struct TypeInfo {
    s16 id;
};

class EnvObj : public utl::IParameterObj, public utl::INamedObj {
    SEAD_RTTI_BASE(EnvObj)

public:
    static constexpr s32 cTypeMax = 128;

    enum MetaInfo {
        cMetaInfo_Light = 0,
        cMetaInfo_Fog = 1,
        cMetaInfo_Projector = 2,
        cMetaInfo_Other = 3,
        cMetaInfo_Num = 4,
    };

    using CreateFunc = EnvObj* (*)(sead::Heap*);

    struct TypeData {
        const char* mName;
        const char* mLabel;
        CreateFunc mCreateFunc;
        MetaInfo mMetaInfo;
        s32 mIndex;
        u32 mPriority;
    };
    static_assert(sizeof(TypeData) == 0x28);

    struct ViewData;

    class Index : public utl::INamedObjIndex {
    public:
        ~Index() override;

        const sead::SafeString& getNamedObjName(s32 index) const override;
        s32 getNamedObjNum() const override;

    protected:
        s32 mType;
    };
    static_assert(sizeof(Index) == 0x80);

    EnvObj();
    ~EnvObj() override;

protected:
    void postRead_() override { callbackLoadData(); }

public:
    virtual void initialize(s32 viewNum, sead::Heap* pHeap);
    virtual void update() {}
    virtual void updateView(const ViewData& rViewData, s32 viewIndex) {}
    virtual void drawDebug(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                           const sead::Matrix44f& rProjMtx, s32 viewIndex) const
    {
    }
    virtual void callbackLoadData() {}
    virtual const sead::SafeString& getEnvObjName() const { return *mEnvObjName; }
    const sead::SafeString& getGroupName() const override { return *mGroupName; }
    const sead::SafeString& getObjName() const override { return getEnvObjName(); }
    bool isHostIOEnabled() const override { return mFlag.isOn(1); }
    s32 getObjType() const override { return getTypeId(); }

protected:
    virtual void copyFromImpl_(const EnvObj& rOther);

public:
    virtual s32 getTypeId() const = 0;

    void initialize_(s32 index, s32 viewNum, EnvObjMgr* pMgr, sead::Heap* pHeap);
    void clear_();
    void becomeDefaultName_();
    void setGroupNameCopy(const sead::SafeString& rName);
    void setEnable(bool enable);
    void setEnvObjNameCopy(const sead::SafeString& rName);
    void setEditable(bool editable);
    void copyFrom(const EnvObj& rOther);
    void update_();
    void drawDebug_(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                    const sead::Matrix44f& rProjMtx, s32 viewIndex) const;
    void drawDirectionalLight(DrawContext* pDrawContext, s32 viewIndex,
                              const sead::Vector3f& rDir, const sead::Color4f& rColor0,
                              const sead::Color4f& rColor1, const sead::Color4f& rColor2) const;
    void drawPointLight(DrawContext* pDrawContext, s32 viewIndex, const sead::Vector3f& rPos,
                        f32 radius, const sead::Color4f& rColor0,
                        const sead::Color4f& rColor1) const;
    void drawFog(DrawContext* pDrawContext, s32 viewIndex, f32 start, f32 end,
                 const sead::Vector3f& rDir, const sead::Color4f& rColor) const;

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
    void listenNodeEvent(const sead::hostio::NodeEvent* pEvent);

    static const TypeInfo* registClass(const sead::SafeString& rName,
                                       const sead::SafeString& rLabel, CreateFunc createFunc,
                                       MetaInfo metaInfo, u32 priority);
    static s32 searchTypeIndex(const sead::SafeString& rName);
    static sead::SafeString getMetaInfoName(MetaInfo metaInfo);

    bool isEnable() const { return *mEnable; }
    u16 getIndex() const { return mIndex; }
    EnvObjMgr* getMgr() const { return mMgr; }
    static const TypeData& getTypeData(s32 type) { return sTypeInfoTable[type]; }
    static s32 getTypeNum() { return sTypeNum; }

    static s32 sTypeNum;
    static TypeData sTypeInfoTable[cTypeMax];

protected:
    friend class EnvObjBuffer;
    friend class EnvObjMgr;
    friend class EnvObjSet;

    EnvObjMgr* mMgr = nullptr;
    utl::Parameter<bool> mEnable;
    utl::Parameter<sead::FixedSafeString<32>> mEnvObjName;
    u16 mIndex = 0;
    mutable sead::BitFlag8 mFlag = 1;
    utl::Parameter<sead::FixedSafeString<32>> mGroupName;
    s16 mCopySrcIndex = -1;
};
static_assert(sizeof(EnvObj) == 0x110);

}  // namespace agl::env
