#pragma once
#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_ResAnimCurve.h>

namespace nn::g3d {
struct ResKeyShapeAnimInfo {
    nn::util::BinPtrToString name;
    s8 curveIndex;
    s8 bindIndex;
    u8 _a[6];
};
class ResVertexShapeAnim {
public:
    // result holds key-shape weights; frame is sample time and indices marks unbound entries as -1.
    void Evaluate(float* result, float frame, const s8* indices) const;
    // cache has one interval entry per curve, retaining work between calls.
    void Evaluate(float* result, float frame, const s8* indices, AnimFrameCache* cache) const;
    // shape supplies named key shapes to resolve and store as bind indices.
    BindResult PreBind(const ResShape* shape);
    // shape supplies named key shapes to check without changing stored bindings.
    BindResult BindCheck(const ResShape* shape) const;
    // result receives the resource's initial key-shape weights.
    void Initialize(float* result) const;

    nn::util::BinPtrToString name;
    ResAnimCurve* curves;
    float* baseValues;
    ResKeyShapeAnimInfo* keyShapes;
    u16 curveCount;
    u16 keyShapeCount;
    u8 _24[12];
};
class ResShapeAnim {
public:
    // model supplies the named shapes used to resolve and retain animation bindings.
    BindResult PreBind(const ResModel* model);
    // model supplies named shapes to check without changing resource bindings.
    BindResult BindCheck(const ResModel* model) const;
    // buffer supplies size writable bytes for replacing curves with baked samples.
    bool BakeCurve(void* buffer, size_t size);
    void* ResetCurve();
    void Reset();

    u32 signature;
    u16 flags;
    u16 _6;
    u8 _8[16];
    const ResModel* boundModel;
    u16* bindIndices;
    ResVertexShapeAnim* shapeAnims;
    u8 _30[0x14];
    u32 bakedSize;
    u16 _48;
    u16 shapeAnimCount;
    u8 _4c[4];
};
}
