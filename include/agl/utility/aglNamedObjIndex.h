#pragma once

#include <basis/seadTypes.h>
#include <container/seadListImpl.h>
#include <prim/seadSafeString.h>

#include "utility/aglParameter.h"

namespace sead::hostio {
class Context;
class PropertyEvent;
}  // namespace sead::hostio

namespace agl::utl {

class INamedObjIndex;
class INamedObjMgr;

class INamedObjIndexCallback {
public:
    virtual void callbackSyncNameToIndex(INamedObjIndex* pIndex) {}
    virtual void callbackSyncIndexToName(INamedObjIndex* pIndex) {}
};

class INamedObjIndex : public Parameter<sead::FixedSafeString<32>> {
public:
    static constexpr s32 cIndexNone = -1;
    static constexpr s32 cIndexNotFound = -2;

    INamedObjIndex();
    INamedObjIndex(const sead::FixedSafeString<32>& rValue, const sead::SafeString& rName,
                   const sead::SafeString& rLabel, IParameterObj* pObj);
    ~INamedObjIndex() override;

    virtual const sead::SafeString& getNamedObjName(s32 index) const = 0;
    virtual s32 getNamedObjNum() const = 0;

    void bind(INamedObjMgr* pMgr);
    void syncIndexToName();
    void syncNameToIndex();
    void genComboBoxSelect(sead::hostio::Context* pContext, bool isEnable);
    bool listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    s32 getIndex() const { return mIndex; }
    void setIndex(s32 index) { mIndex = index; }
    void setCallback(INamedObjIndexCallback* pCallback) { mCallback = pCallback; }
    INamedObjMgr* getMgr() const { return mMgr; }

protected:
    friend class INamedObjMgr;

    s32 mIndex;
    sead::ListNode mListNode;
    INamedObjIndexCallback* mCallback = nullptr;
    INamedObjMgr* mMgr = nullptr;
};
static_assert(sizeof(INamedObjIndex) == 0x78);

}  // namespace agl::utl
