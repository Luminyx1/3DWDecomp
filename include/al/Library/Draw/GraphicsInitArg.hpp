#pragma once

#include <basis/seadTypes.h>

namespace al {
class GraphicsSystemInfo;
class ViewRenderer;

class ViewRendererCreator {
public:
    ViewRendererCreator() = default;

    virtual ViewRenderer* createViewRenderer(GraphicsSystemInfo* pInfo);
    virtual void deleteViewRenderer(ViewRenderer* pRenderer);
};

struct GraphicsInitArg {
    s32 _0 = 0;
    f32 mFar = 1000.0f;
    f32 mNear = 100.0f;
    bool _c = false;
    bool _d = false;
    bool _e = false;
    bool _f = false;
    bool _10 = false;
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
