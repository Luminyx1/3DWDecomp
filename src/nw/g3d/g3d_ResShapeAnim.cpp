#include <nn/g3d/g3d_ResShapeAnim.h>

namespace nn::g3d {
static_assert(sizeof(ResVertexShapeAnim) == 0x30, "Vertex shape animation size");
static_assert(sizeof(ResShapeAnim) == 0x50, "Shape animation resource size");
// result receives weights at frame; indices skips unbound key shapes.
void ResVertexShapeAnim::Evaluate(float* result, float frame, const s8* indices) const {
    int count = keyShapeCount;
    const ResKeyShapeAnimInfo* info = keyShapes;
    for (int i = 0; i < count; ++i, ++info) {
        if (indices[i] == -1) continue;
        int index = info->curveIndex;
        if (index == -1) continue;
        const ResAnimCurve* curve = &curves[index];
        AnimFrameCache temporary;
        result[i] = curve->EvaluateFloat(frame, &temporary);
    }
}
// result receives weights at frame; indices skips unbound key shapes; cache stores curve intervals.
void ResVertexShapeAnim::Evaluate(float* result, float frame, const s8* indices, AnimFrameCache* cache) const {
    int count = keyShapeCount;
    const ResKeyShapeAnimInfo* info = keyShapes;
    for (int i = 0; i < count; ++i, ++info) {
        if (indices[i] == -1) continue;
        int index = info->curveIndex;
        if (index == -1) continue;

        result[i] = curves[index].EvaluateFloat(frame, &cache[index]);
    }
}
// shape supplies the named key shapes to resolve and retain.
BindResult ResVertexShapeAnim::PreBind(const ResShape* shape) {
    BindResult result;
    int count = keyShapeCount;
    ResKeyShapeAnimInfo* info = keyShapes;
    for (int i = 0; i < count; ++i, ++info) {
        int index = shape->FindKeyShapeIndex(info->name.Get()->GetData());
        info->bindIndex = index;
        if (index == -1) result.Merge(BindResult(BindResult::Flag_Failure));
        else result.Merge(BindResult(BindResult::Flag_Success));
    }
    return result;
}
// shape supplies the named key shapes to check without changing bindings.
BindResult ResVertexShapeAnim::BindCheck(const ResShape* shape) const {
    BindResult result;
    int count = keyShapeCount;
    const ResKeyShapeAnimInfo* info = keyShapes;
    for (int i = 0; i < count; ++i, ++info) {
        int index = shape->FindKeyShapeIndex(info->name.Get()->GetData());
        if (index == -1) result.Merge(BindResult(BindResult::Flag_Failure));
        else result.Merge(BindResult(BindResult::Flag_Success));
    }
    return result;
}
// result receives zero for the base shape and stored values for the remaining keys.
void ResVertexShapeAnim::Initialize(float* result) const {
    result[0] = 0.0f;
    int count = keyShapeCount;
    const float* values = baseValues;
    for (int i = 1; i < count; ++i) result[i] = values[i - 1];
}
// model supplies named shapes; retain the model and each successful shape/key-shape binding.
BindResult ResShapeAnim::PreBind(const ResModel* model) {
    boundModel = model;
    BindResult result;
    int count = shapeAnimCount;
    u16* indices = bindIndices;
    for (int i = 0; i < count; ++i) {
        ResVertexShapeAnim* anim = &shapeAnims[i];
        const ResShape* shape = model->FindShape(anim->name.Get()->GetData());
        if (shape != nullptr) {
            indices[i] = shape->GetIndex();
            result.Merge(anim->PreBind(shape));
        } else {
            indices[i] = 0xffff;
            result.Merge(BindResult(BindResult::Flag_Failure));
        }
    }
    return result;
}
// model supplies named shapes to check without updating the stored model or indices.
BindResult ResShapeAnim::BindCheck(const ResModel* model) const {
    BindResult result;
    int count = shapeAnimCount;
    for (int i = 0; i < count; ++i) {
        const ResVertexShapeAnim* anim = &shapeAnims[i];
        const ResShape* shape = model->FindShape(anim->name.Get()->GetData());
        if (shape != nullptr) result.Merge(anim->BindCheck(shape));
        else result.Merge(BindResult(BindResult::Flag_Failure));
    }
    return result;
}
// buffer supplies size writable bytes; an empty request already succeeds.
bool ResShapeAnim::BakeCurve(void* buffer, size_t size) {
    if (!size) return true;
    if ((buffer == nullptr) || bakedSize > size) return false;
    u8* output = static_cast<u8*>(buffer);
    int count = shapeAnimCount;
    for (int i = 0; i < count; ++i) {
        ResVertexShapeAnim* anim = &shapeAnims[i];
        int curves = anim->curveCount;
        for (int j = 0; j < curves; ++j) {
            ResAnimCurve* curve = &anim->curves[j];
            size_t bytes = curve->CalculateBakedFloatSize();
            curve->BakeFloat(output, bytes);
            output += bytes;
        }
    }
    flags |= 1;
    return true;
}
void* ResShapeAnim::ResetCurve() {
    if (!(flags & 1)) return nullptr;
    void* buffer = nullptr;
    bool found = false;
    int count = shapeAnimCount;
    for (int i = 0; i < count; ++i) {
        ResVertexShapeAnim* anim = &shapeAnims[i];
        int curves = anim->curveCount;
        for (int j = 0; j < curves; ++j) {
            ResAnimCurve* curve = &anim->curves[j];
            if (((curve->flags & 0x70) == 0x20) & !found) {
                buffer = curve->keys;
                found = true;
            }
            curve->Reset();
        }
    }
    flags ^= 1;
    return buffer;
}
void ResShapeAnim::Reset() {
    boundModel = nullptr;
    int count = shapeAnimCount;
    u16* indices = bindIndices;
    for (int i = 0; i < count; ++i) {
        ResVertexShapeAnim* anim = &shapeAnims[i];
        int keys = anim->keyShapeCount;
        ResKeyShapeAnimInfo* info = anim->keyShapes;
        for (int j = 0; j < keys; ++j, ++info) info->bindIndex = -1;
        indices[i] = 0xffff;
    }
    if (!(flags & 1)) return;
    count = shapeAnimCount;
    for (int i = 0; i < count; ++i) {
        ResVertexShapeAnim* anim = &shapeAnims[i];
        int curves = anim->curveCount;
        for (int j = 0; j < curves; ++j) anim->curves[j].Reset();
    }
    flags ^= 1;
}
}
