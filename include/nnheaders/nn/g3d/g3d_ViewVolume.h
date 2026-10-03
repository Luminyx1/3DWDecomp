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
    /** @brief Access the volume's optional axis-aligned bounds.
     * @return Writable bounding box used when bounds testing is enabled. */
    Aabb& GetAabb() { return bounds; }
    /** @brief Access one clipping plane.
     * @param index Plane index in the range [0, 6).
     * @return Writable plane at the specified index. */
    Plane& GetPlane(int index) { return planes[index]; }
    /** @brief Select how many clipping planes participate in intersection tests.
     * @param count Active plane count in the range [0, 6]. */
    void SetPlaneCount(int count) { planeCount = count; }
    /** @brief Enable or disable the preliminary bounding-box test.
     * @param use Zero disables testing; any nonzero value enables it. */
    void SetUseBounds(u32 use) { useBounds = use; }

    bool TestIntersection(const nn::g3d::Sphere& shape) const;
    s32 TestIntersectionEx(const nn::g3d::Sphere& shape) const;
    bool TestIntersection(const nn::g3d::Aabb& shape) const;
    s32 TestIntersectionEx(const nn::g3d::Aabb& shape) const;

    /** @brief Disable clipping-plane tests while preserving the optional bounding-box test. */
    void ClearPlanes() { planeCount = 0; }

private:
    Aabb bounds;
    Plane planes[6];
    int planeCount;
    u32 useBounds;
};
}  // namespace nn::g3d
