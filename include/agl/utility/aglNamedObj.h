#pragma once

#include <basis/seadTypes.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadSafeString.h>

namespace agl::utl {

class INamedObj : public sead::hostio::Node {
public:
    INamedObj();
    virtual ~INamedObj();

    virtual const sead::SafeString& getObjName() const;
    virtual const sead::SafeString& getGroupName() const;
    virtual s32 getObjType() const;
    virtual bool isHostIOEnabled() const;

    static const sead::SafeString& getDefaultGroupName();
};
static_assert(sizeof(INamedObj) == 0x8);

}  // namespace agl::utl
