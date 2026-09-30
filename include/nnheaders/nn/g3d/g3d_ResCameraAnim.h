#pragma once
#include <nn/g3d/g3d_ResAnimCurve.h>
#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_BindFuncTable.h>

namespace nn::g3d {
struct CameraAnimResult { float values[11]; };
class ResCameraAnim {
public:
    // result receives the resource's constant animation values.
    void Initialize(CameraAnimResult* result) const;
    // result receives samples at frame; cache, when supplied, has one entry per curve.
    void Evaluate(CameraAnimResult* result, float frame) const;
    void Evaluate(CameraAnimResult* result, float frame, AnimFrameCache* cache) const;
    // buffer supplies size writable bytes for replacing curves with baked samples.
    bool BakeCurve(void* buffer, size_t size);
    void* ResetCurve();
    void Reset();

    u32 signature;
    u16 flags;
    u16 _6;
    const char* name;
    ResAnimCurve* curves; // 0x10
    void* baseValues;
    void* userData;
    void* userDataDictionary;
    u32 frameCount; // 0x30
    u32 bakedSize;
    u16 userDataCount;
    u8 curveCount;
    u8 _3b[5];
};
}
