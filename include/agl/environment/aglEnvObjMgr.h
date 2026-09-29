#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>
#include <prim/seadSafeString.h>
#include <thread/seadCriticalSection.h>

#include "environment/aglEnvObj.h"
#include "environment/aglEnvObjBuffer.h"
#include "environment/aglEnvObjSet.h"
#include "utility/aglNamedObjMgr.h"
#include "utility/aglParameterIO.h"
#include "utility/aglParameterList.h"

namespace sead {
class Heap;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
class RenderBuffer;
}  // namespace agl

namespace agl::env {

struct EnvObj::ViewData {
    ViewData(const sead::Matrix34f& rViewMtx, const sead::Matrix34f& rInvViewMtx,
             const sead::Matrix44f& rProjMtx)
    {
        mViewMtx = rViewMtx;
        mInvViewMtx = rInvViewMtx;
        mProjMtx = rProjMtx;
    }

    sead::Matrix34f mViewMtx;
    sead::Matrix34f mInvViewMtx;
    sead::Matrix44f mProjMtx;
};
static_assert(sizeof(EnvObj::ViewData) == 0xa0);

class EnvObjMgr : public EnvObjBuffer, public utl::INamedObjMgr, public utl::IParameterIO {
public:
    class InitArg : public EnvObjBuffer::AllocateArg {
    public:
        InitArg();

        s32 getGroupNum() const { return mGroupNum; }
        s32 getViewNum() const { return mViewNum; }

    private:
        s32 mGroupNum = 0x100;
        s32 mViewNum = 1;
    };

    class TypeNode : public utl::IParameterList, public sead::hostio::Node {
    public:
        TypeNode();

        void initialize(s32 type, EnvObjMgr* pMgr, sead::Heap* pHeap);
        void genMessage(sead::hostio::Context* pContext);
        void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    private:
        s32 mType = 0;
        EnvObjMgr* mMgr = nullptr;
    };
    static_assert(sizeof(TypeNode) == 0xb0);

    struct View {
        View() = default;

        sead::Matrix34f mViewMtx;
        sead::Matrix34f mInvViewMtx;
        sead::Matrix44f mProjMtx;
        f32 mNear;
        f32 mFar;
        s32 mDirectionalLightNum;
        s32 _ac = 0;
        const RenderBuffer* mRenderBuffer;
        f32 mDebugScale;
    };
    static_assert(sizeof(View) == 0xc0);

    EnvObjMgr();
    ~EnvObjMgr() override;

    bool save(const sead::SafeString& rPath, u32 flag) const override
    {
        return saveImpl_(rPath, 1, -1);
    }
    void applyResParameterArchive(utl::ResParameterArchive arc) override
    {
        applyResource_(arc, arc, 1.0f, -1);
    }
    void listenPropertyEventFromGroup(GroupEventType type, Group* pGroup) override;
    const sead::SafeString& getSaveFilePath() const override { return mPath; }
    const sead::SafeString& getNamedObjName(s32 index, s32 type) const override
    {
        return getObj(type, index)->getEnvObjName();
    }
    s32 getNamedObjNum(s32 type) const override { return getObjNum(type); }
    void constructList() override;

    void initialize(const InitArg& rArg, sead::Heap* pHeap);
    void removeObj(EnvObj* pObj);
    void clear(s32 group);
    void reconstruct();
    void update();
    void updateView(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                    s32 viewIndex);
    void drawDebug(DrawContext* pDrawContext, s32 viewIndex, const RenderBuffer* pRenderBuffer,
                   f32 scale) const;
    void drawDirectionalLight_(DrawContext* pDrawContext, s32 viewIndex, const EnvObj& rObj,
                               const sead::Vector3f& rDir, const sead::Color4f& rColor0,
                               const sead::Color4f& rColor1, const sead::Color4f& rColor2) const;
    void drawPointLight_(DrawContext* pDrawContext, s32 viewIndex, const EnvObj& rObj,
                         const sead::Vector3f& rPos, f32 radius, const sead::Color4f& rColor0,
                         const sead::Color4f& rColor1) const;
    void drawFog_(DrawContext* pDrawContext, s32 viewIndex, const EnvObj& rObj, f32 start,
                  f32 end, const sead::Vector3f& rDir, const sead::Color4f& rColor) const;
    bool saveToGroupFilePath(const sead::SafeString& rPath) const;

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    void setDirty() { mFlag.set(1); }
    EnvObjSet& getEnvObjSet() { return mEnvObjSet; }

protected:
    bool saveImpl_(const sead::SafeString& rPath, u32 flag, s32 group) const;
    void applyResource_(utl::ResParameterArchive arc0, utl::ResParameterArchive arc1, f32 t,
                        s32 group);

    friend class EnvObj;

    sead::BitFlag32 mFlag;
    sead::PtrArray<EnvObj> mUpdateObj;
    mutable sead::Buffer<View> mView;
    f32 mDebugDrawScale = 1.0f;
    sead::CriticalSection mCS;
    EnvObjSet mEnvObjSet;
    s32 mListMode = 2;
    sead::Buffer<TypeNode> mTypeNode;
    void* _568 = nullptr;
    void* _570 = nullptr;
    s32 mSelectedType = 0;
    s32 mSelectedDirectionalLight = 0;
};
static_assert(sizeof(EnvObjMgr) == 0x580);

}  // namespace agl::env
