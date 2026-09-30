#pragma once

#include <nn/types.h>
#include <nn/util/util_MathTypes.h>

namespace nn::g3d {

class WorldMtxManip {
public:
    // Non-const access invalidates the cached identity/scale flags of the bone.
    nn::util::Matrix4x3fType* GetMtx() {
        *m_pFlag &= ~0xCC000000u;
        return m_pMtx;
    }

private:
    nn::util::Matrix4x3fType* m_pMtx;
    void* m_pScale;
    u32* m_pFlag;
};

class ICalculateWorldCallback {
public:
    class CallbackArg {
    public:
        int GetBoneIndex() const { return *m_pBoneIndex; }
        void SetCallbackBoneIndex(int index) { m_CallbackBoneIndex = index; }

    private:
        const int* m_pBoneIndex;
        int m_CallbackBoneIndex;
    };

    virtual ~ICalculateWorldCallback() {}
    virtual void Exec(CallbackArg& arg, WorldMtxManip& manip) = 0;
};

}  // namespace nn::g3d
