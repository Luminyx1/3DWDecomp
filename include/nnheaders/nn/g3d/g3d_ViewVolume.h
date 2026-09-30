#pragma once

#include <nn/g3d/g3d_Bounding.h>

namespace nn::g3d {
class Aabb;
class Sphere;

class ViewVolume {
public:
    // fovy and aspect define the lens; near/far bound depth and matrix places the volume.
    void SetPerspective(f32 fovy, f32 aspect, f32 near, f32 far, const nn::util::neon::MatrixRowMajor4x3fType& matrix);
    // top/bottom/left/right describe the near plane; near/far bound depth; matrix places the volume.
    void SetFrustum(f32 top, f32 bottom, f32 left, f32 right, f32 near, f32 far, const nn::util::neon::MatrixRowMajor4x3fType& matrix);
    // top/bottom/left/right and near/far bound the orthographic volume transformed by matrix.
    void SetOrtho(f32 top, f32 bottom, f32 left, f32 right, f32 near, f32 far, const nn::util::neon::MatrixRowMajor4x3fType& matrix);
    // shape is tested against this volume; Ex returns -1 outside, 0 crossing, or 1 inside.
    bool TestIntersection(const nn::g3d::Sphere& shape) const;
    s32 TestIntersectionEx(const nn::g3d::Sphere& shape) const;
    bool TestIntersection(const nn::g3d::Aabb& shape) const;
    s32 TestIntersectionEx(const nn::g3d::Aabb& shape) const;

private:
    Aabb bounds;
    Plane planes[6];
    int planeCount;
    u32 useBounds;
};
}  // namespace nn::g3d
