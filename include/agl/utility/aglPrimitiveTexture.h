#pragma once

#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include "utility/aglDebugTexturePage.h"

namespace sead::hostio {
class Context;
class PropertyEvent;
}  // namespace sead::hostio

namespace agl {
class TextureSampler;
}

namespace agl::utl {

class PrimitiveTexture : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(PrimitiveTexture)

    PrimitiveTexture();
    virtual ~PrimitiveTexture();

public:
    enum Type
    {
        cType_Black1D,
        cType_Black1DArray,
        cType_Black2D,
        cType_Black2DArray,
        cType_Black3D,
        cType_BlackCube,
        cType_BlackCubeArray,
        cType_White2D,
        cType_WhiteCubeArray,
        cType_Zero2D,
        cType_Zero1D,
        cType_Zero1DArray,
        cType_Zero2DArray,
        cType_Zero3D,
        cType_Red2D,
        cType_Green2D,
        cType_Blue2D,
        cType_Gray2D,
        cType_DarkRed2D,
        cType_DarkGreen2D,
        cType_DarkBlue2D,
        cType_Depth32_0,
        cType_Depth32_1,
        cType_DepthShadow,
        cType_DepthShadowArray,
        cType_MipLevel,
        cType_Num
    };

    void initialize(sead::Heap* pHeap);
    void entryDebugPage();

    const TextureSampler* getTextureSampler(Type type) const { return mSamplers[type]; }

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    void destroy_();

    TextureSampler* mSamplers[cType_Num];
    DebugTexturePage mDebugTexturePage;
};
static_assert(sizeof(PrimitiveTexture) == 0x338);

}  // namespace agl::utl
