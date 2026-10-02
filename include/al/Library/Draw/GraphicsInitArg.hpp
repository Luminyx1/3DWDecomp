#pragma once

#include <basis/seadTypes.h>

#include "shadow/aglPrimitiveOcclusion.h"

namespace al {
class GraphicsSystemInfo;
class ViewRenderer;

/**
 * Creates the view renderer used by a GraphicsSystemInfo. Kept here (instead of including
 * ViewRendererCreator.hpp) because GraphicsSystemInfo needs the virtual interface.
 */
class ViewRendererCreator {
public:
    ViewRendererCreator() = default;

    virtual ViewRenderer* createViewRenderer(GraphicsSystemInfo* pInfo);
    virtual void deleteViewRenderer(ViewRenderer* pRenderer);
};

/**
 * Atmos scatter type (0 = none). It has its own copy assignment, which makes GraphicsInitArg
 * copy it separately from the rest of the struct.
 */
class AtmosScatterType {
public:
    AtmosScatterType(s32 type = 0) : mType(type) {}

    AtmosScatterType& operator=(const AtmosScatterType& rOther) {
        mType = rOther.mType;
        return *this;
    }

    operator s32() const { return mType; }

private:
    s32 mType;
};

struct GraphicsInitArg {
    /**
     * Gets whether the atmos scatter is also rendered into a cube map.
     * @return true if atmos scatter is enabled and uses a cube map
     */
    bool isUsingCubeMapAtmosScatter() const;

    /**
     * Gets the number of views the atmos scatter renders (the cube map faces are added on top).
     * @return the atmos scatter view count
     */
    s32 getAtmosScatterViewNum() const;

    /**
     * Gets the number of views including the stereo doubling.
     * @return the view count
     */
    s32 getViewNumWithStereo() const { return _20 << mIsStereo; }

    /**
     * Sets the view count (also used as the primitive occlusion context count).
     * @param viewNum the view count
     */
    void setViewNum(s32 viewNum) {
        _20 = viewNum;
        _14 = viewNum;
    }

    /**
     * Gets the primitive occlusion create arg stored at _14.
     * @return the primitive occlusion create arg
     */
    const agl::sdw::PrimitiveOcclusion::CreateArg& getPrimitiveOcclusionArg() const {
        return *reinterpret_cast<const agl::sdw::PrimitiveOcclusion::CreateArg*>(&_14);
    }

    AtmosScatterType mAtmosScatterType = 0;
    f32 mFar = 1000.0f;
    f32 mNear = 100.0f;
    bool mIsUsingCubeMapAtmosScatter = false;
    bool mIsStereo = false;
    bool mIsUsingViewRenderer = false;
    bool _f = false;
    bool _10 = false;
    // Primitive occlusion initialize arg (0x14..0x20)
    s32 _14 = 1;
    bool _18 = false;
    bool _19 = true;
    bool _1a = false;
    bool _1b = false;
    bool _1c = false;
    bool _1d = true;
    bool _1e = false;
    bool _1f = false;
    s32 _20 = 1;
    ViewRendererCreator* mViewRendererCreator = nullptr;
};

static_assert(sizeof(GraphicsInitArg) == 0x30);
}  // namespace al
