#pragma once

#include <basis/seadTypes.h>
#include <heap/seadDisposer.h>
#include <prim/seadDelegate.h>
#include <prim/seadSafeString.h>

namespace sead::hostio {
class Context;
}

namespace agl::lyr {

class RenderInfo;

class DrawMethod : public sead::IDisposer {
public:
    enum BindType {
        cBindType_None = 0,
        cBindType_Const = 1,
        cBindType_NonConst = 2,
    };

    using Delegate = sead::IDelegate1<const RenderInfo&>;

    ~DrawMethod() override;

    static s32 compare(const DrawMethod* pA, const DrawMethod* pB);
    void genMessage(sead::hostio::Context* pContext);

    bool isBound() const { return mFlag & 1; }
    const Delegate* getDelegate() const { return reinterpret_cast<const Delegate*>(mDelegate); }
    const void* getObject() const { return *reinterpret_cast<void* const*>(&mDelegate[2]); }
    s32 getPriority() const { return mPriority; }

    void invoke(const RenderInfo& rInfo) const
    {
        if (isBound())
        {
            const_cast<Delegate*>(getDelegate())->invoke(rInfo);
        }
    }

private:
    sead::SafeString mName;
    BindType mBindType;
    s32 mPriority;
    u8 mFlag;
    u32 mDelegate[8];
};
static_assert(sizeof(DrawMethod) == 0x60);

}  // namespace agl::lyr
