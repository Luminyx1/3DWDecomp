#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadSafeArray.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadSafeString.h>

#include "environment/aglEnvObj.h"
#include "environment/aglEnvObjBuffer.h"
#include "utility/aglNamedObjIndex.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterList.h"
#include "utility/aglParameterObj.h"

namespace sead {
class Heap;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl::env {

class EnvObjMgr;

class EnvObjSet : public EnvObjBuffer,
                  public utl::INamedObjIndexCallback,
                  public utl::IParameterList,
                  public sead::hostio::Node {
public:
    class Ref : public utl::IParameterObj {
    public:
        ~Ref() override;

    protected:
        bool isApply_(utl::ResParameterObj obj) const override;

    public:
        EnvObj::Index mIndex;
        utl::Parameter<sead::FixedSafeString<32>> mName;
    };
    static_assert(sizeof(Ref) == 0x100);

    EnvObjSet();
    ~EnvObjSet() override;

    void allocBuffer(const AllocateArg& rArg, sead::Heap* pHeap) override;

protected:
    bool preRead_() override;
    void postRead_() override;

public:
    void callbackSyncNameToIndex(utl::INamedObjIndex* pIndex) override;
    void callbackSyncIndexToName(utl::INamedObjIndex* pIndex) override;
    virtual void genMessageEachObj(sead::hostio::Context* pContext, s32 type, const EnvObj* pObj);

    void bind(EnvObjMgr* pMgr);
    void pushBack(EnvObj* pObj);
    void erase(EnvObj* pObj);

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    void setSelectedObj(EnvObj::MetaInfo metaInfo, EnvObj* pObj) { mSelectedObj[metaInfo] = pObj; }
    EnvObj* getSelectedObj(EnvObj::MetaInfo metaInfo) const { return mSelectedObj[metaInfo]; }

protected:
    utl::Parameter<sead::FixedSafeString<32>> mSetName;
    utl::ParameterList mRefList;
    EnvObjMgr* mMgr = nullptr;
    utl::ParameterObj mRefObj;
    sead::Buffer<Ref> mRef;
    sead::SafeArray<EnvObj*, EnvObj::cMetaInfo_Num> mSelectedObj;
};
static_assert(sizeof(EnvObjSet) == 0x230);

}  // namespace agl::env
