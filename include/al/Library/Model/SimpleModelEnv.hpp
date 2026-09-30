#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace sead {
class Heap;
}

namespace al {
class GraphicsSystemInfo;
class LightInfo;
class UniformBlock;

class SimpleModelEnv {
public:
    SimpleModelEnv();
    ~SimpleModelEnv();

    void initialize(s32 bufferNum, const GraphicsSystemInfo* pInfo, sead::Heap* pHeap);
    void swapBuffer();
    void prepareModelDraw(s32 index) const;
    void updateEnv(s32 index, const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                   const sead::Vector2f& rProjOffset, const sead::Vector2f& rScreenSize, f32 near,
                   f32 far, f32 fovy, f32 aspect, sead::Vector2f screenSizeValue,
                   const LightInfo* pLightA, const LightInfo* pLightB, const LightInfo* pLightC,
                   f32 rate, const char* pName);

private:
    sead::PtrArray<UniformBlock> mUniformBlocks;
    const GraphicsSystemInfo* mGraphicsSystemInfo = nullptr;
};

static_assert(sizeof(SimpleModelEnv) == 0x18);

}  // namespace al
