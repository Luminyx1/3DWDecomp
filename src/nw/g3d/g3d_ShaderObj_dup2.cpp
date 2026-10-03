#include <nn/g3d/g3d_ViewVolume.h>
#include <nn/util/util_VectorApi.h>
#include <nn/util/util_Arithmetic.h>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace nn::g3d {
namespace {
/**
 * @brief Sum all four vector lanes and replicate the result.
 * @param value Vector product whose four lanes contribute to the sum.
 * @return The sum replicated in every lane.
 */
inline float32x4_t Sum(float32x4_t value) {
    float32x2_t sum = vadd_f32(vget_high_f32(value), vget_low_f32(value));
    sum = vpadd_f32(sum, sum);
    return vcombine_f32(sum, sum);
}

/**
 * @brief Calculate the dot product of padded three-component vectors.
 * @param first First vector, with a zero padding lane.
 * @param second Second vector, with a zero padding lane.
 * @return Scalar dot product.
 */
inline float Dot(float32x4_t first, float32x4_t second) {
    return vgetq_lane_f32(Sum(vmulq_f32(first, second)), 0);
}

/**
 * @brief Construct a three-component vector with zero padding.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param z Z coordinate.
 * @return Vector containing x, y, z and a zero fourth lane.
 */
inline float32x4_t Vector(float x, float y, float z) {
    nn::util::Vector3fType result;
    nn::util::VectorSet(&result, x, y, z);
    return result._v;
}

/**
 * @brief Apply an affine transformation to a position.
 * @param value Position whose first three lanes supply coordinates.
 * @param matrix Affine matrix containing basis vectors and translation.
 * @return Transformed position.
 */
inline float32x4_t TransformPosition(float32x4_t value, const nn::util::Matrix4x3fType& matrix) {
    float32x4_t x = matrix._m.val[0], y = matrix._m.val[1], z = matrix._m.val[2],
                translation = matrix._m.val[3];
    float32x4_t result = vmulq_laneq_f32(x, value, 0);
    result = vfmaq_laneq_f32(result, y, value, 1);
    result = vfmaq_laneq_f32(result, z, value, 2);
    return vaddq_f32(translation, result);
}

/**
 * @brief Take absolute coordinate values while clearing vector padding.
 * @param value Vector whose first three coordinates are used.
 * @return Absolute coordinate values with a zero padding lane.
 */
inline float32x4_t Absolute(float32x4_t value) {
    return Vector(fabsf(value[0]), fabsf(value[1]), fabsf(value[2]));
}

/**
 * @brief Select the smaller value in each coordinate and clear padding.
 * @param first First coordinate vector.
 * @param second Second coordinate vector.
 * @return Componentwise minimum with a zero padding lane.
 */
inline float32x4_t Minimum(float32x4_t first, float32x4_t second) {
    return Vector(std::min(first[0], second[0]), std::min(first[1], second[1]),
                  std::min(first[2], second[2]));
}

/**
 * @brief Select the larger value in each coordinate and clear padding.
 * @param first First coordinate vector.
 * @param second Second coordinate vector.
 * @return Componentwise maximum with a zero padding lane.
 */
inline float32x4_t Maximum(float32x4_t first, float32x4_t second) {
    return Vector(std::max(first[0], second[0]), std::max(first[1], second[1]),
                  std::max(first[2], second[2]));
}

/**
 * @brief Calculate the cross product of two padded vectors.
 * @param first First vector with zero padding.
 * @param second Second vector with zero padding.
 * @return Cross product with zero padding.
 */
inline float32x4_t Cross(float32x4_t first, float32x4_t second) {
    uint8x8_t yz = {4, 5, 6, 7, 8, 9, 10, 11};
    uint8x8_t xw = {0, 1, 2, 3, 12, 13, 14, 15};
    uint8x8_t zx = {8, 9, 10, 11, 0, 1, 2, 3};
    uint8x8_t yw = {4, 5, 6, 7, 12, 13, 14, 15};
    auto a = vreinterpretq_u8_f32(first);
    auto b = vreinterpretq_u8_f32(second);
    float32x2_t ayz = vreinterpret_f32_u8(vqtbl1_u8(a, yz));
    float32x2_t axw = vreinterpret_f32_u8(vqtbl1_u8(a, xw));
    float32x2_t bzx = vreinterpret_f32_u8(vqtbl1_u8(b, zx));
    float32x2_t byw = vreinterpret_f32_u8(vqtbl1_u8(b, yw));
    float32x2_t low = vmul_f32(ayz, bzx);
    float32x2_t high = vmul_f32(axw, byw);
    float32x4_t left = vcombine_f32(low, high);
    float32x4_t rightA = vreinterpretq_f32_u8(vcombine_u8(vqtbl1_u8(a, zx), vqtbl1_u8(a, yw)));
    float32x4_t rightB = vreinterpretq_f32_u8(vcombine_u8(vqtbl1_u8(b, yz), vqtbl1_u8(b, xw)));
    return vfmsq_f32(left, rightA, rightB);
}

/**
 * @brief Normalize a vector using two reciprocal-square-root refinements.
 * @param value Vector to normalize; zero remains zero.
 * @return Unit-length vector, or zero for a zero-length input.
 */
inline float32x4_t Normalize(float32x4_t value) {
    float32x4_t length = Sum(vmulq_f32(value, value));
    float32x4_t inverse = vrsqrteq_f32(length);
    inverse = vmulq_f32(inverse, vrsqrtsq_f32(inverse, vmulq_f32(inverse, length)));
    inverse = vmulq_f32(inverse, vrsqrtsq_f32(inverse, vmulq_f32(length, inverse)));
    return vreinterpretq_f32_u32(
        vandq_u32(vreinterpretq_u32_f32(vmulq_f32(value, inverse)), vmvnq_u32(vceqzq_f32(length))));
}
} // namespace

/**
 * @brief Transform a sphere using the largest basis scale for its radius.
 * @param source Sphere to transform; may alias the destination.
 * @param matrix Affine transformation applied to the center and radius.
 */
void Sphere::Transform(const Sphere& source, const nn::util::Matrix4x3fType& matrix) {
    center._v = TransformPosition(source.center._v, matrix);
    float x = vgetq_lane_f32(vsqrtq_f32(Sum(vmulq_f32(matrix._m.val[0], matrix._m.val[0]))), 0);
    float y = vgetq_lane_f32(vsqrtq_f32(Sum(vmulq_f32(matrix._m.val[1], matrix._m.val[1]))), 0);
    float z = vgetq_lane_f32(vsqrtq_f32(Sum(vmulq_f32(matrix._m.val[2], matrix._m.val[2]))), 0);
    float scale = std::max(std::max(x, y), z);
    radius = source.radius * scale;
}

/**
 * @brief Construct a sphere enclosing two input spheres.
 * @param first First sphere, with a nonnegative radius.
 * @param second Second sphere, with a nonnegative radius.
 */
void Sphere::Merge(const Sphere& first, const Sphere& second) {
    float32x4_t difference = vsubq_f32(second.center._v, first.center._v);
    float distanceSquared = Dot(difference, difference);
    float radiusDifference = second.radius - first.radius;

    if (distanceSquared <
        vget_lane_f32(vmax_f32(vdup_n_f32(radiusDifference * radiusDifference), vdup_n_f32(0.0001f)), 0)) {
        *this = first.radius > second.radius ? first : second;
    } else {
        float distance = distanceSquared * (1.0f / sqrtf(distanceSquared));
        float combinedRadius = (distance + (first.radius + second.radius)) * 0.5f;
        float fraction = (combinedRadius - first.radius) / distance;
        float32x4_t position = vaddq_f32(first.center._v, vmulq_n_f32(difference, fraction));
        radius = combinedRadius;
        center._v = position;
    }
}

/**
 * @brief Compute bounds for an array of positions.
 * @param points Non-null array containing at least one position, even when count is nonpositive.
 * @param count Number of positions; a nonpositive count uses only the first position.
 */
void Aabb::Set(const nn::util::Vector3fType* points, int count) {
    float32x4_t high = points[0]._v;
    float32x4_t low = high;

    for (int i = 0; i < count; ++i) {
        low = Minimum(low, points[i]._v);
        high = Maximum(high, points[i]._v);
    }

    maximum._v = high;
    minimum._v = low;
}

/**
 * @brief Transform a center-and-extents box into an enclosing axis-aligned box.
 * @param source Box center and nonnegative half-extents.
 * @param matrix Affine transformation into the destination coordinate system.
 */
void Aabb::Transform(const Bounding& source, const nn::util::Matrix4x3fType& matrix) {
    nn::util::Vector3fType position;
    nn::util::VectorLoad(&position, source.center);
    float32x4_t x = Absolute(matrix._m.val[0]);
    float32x4_t y = Absolute(matrix._m.val[1]);
    float32x4_t z = Absolute(matrix._m.val[2]);
    float32x4_t center = TransformPosition(position._v, matrix);
    float32x4_t extent = vmulq_n_f32(x, source.extent.x);
    extent = vaddq_f32(extent, vmulq_n_f32(y, source.extent.y));
    extent = vaddq_f32(extent, vmulq_n_f32(z, source.extent.z));
    minimum._v = vsubq_f32(center, extent);
    maximum._v = vaddq_f32(center, extent);
}

/**
 * @brief Compute the componentwise union of two axis-aligned boxes.
 * @param first First box; may alias the destination.
 * @param second Second box; may alias the destination.
 */
void Aabb::Merge(const Aabb& first, const Aabb& second) {
    minimum._v = Minimum(first.minimum._v, second.minimum._v);
    maximum._v = Maximum(first.maximum._v, second.maximum._v);
}

/**
 * @brief Construct a plane from three positions and their winding.
 * @param first Point through which the plane passes.
 * @param second Second point defining the plane orientation.
 * @param third Third point; noncollinear inputs produce a unit normal.
 */
void Plane::Set(const nn::util::Vector3fType& first, const nn::util::Vector3fType& second,
                const nn::util::Vector3fType& third) {
    normal._v = Normalize(Cross(vsubq_f32(third._v, first._v), vsubq_f32(second._v, first._v)));
    distance = -Dot(first._v, normal._v);
}

/**
 * @brief Construct a perspective viewing volume.
 * @param fovy Vertical field of view in radians.
 * @param aspect Width-to-height ratio.
 * @param near Positive near-plane distance.
 * @param far Far-plane distance greater than near.
 * @param matrix Affine transform placing the viewing volume.
 */
void ViewVolume::SetPerspective(float fovy, float aspect, float near, float far,
                                const nn::util::Matrix4x3fType& matrix) {
    auto angle = nn::util::RadianToAngleIndex(fovy * 0.5f);
    float tangent = nn::util::SinTable(angle) / nn::util::CosTable(angle);
    float top = tangent * near;
    SetFrustum(top, -top, -(top * aspect), top * aspect, near, far, matrix);
}

/**
 * @brief Construct the clipping planes and bounds of a viewing volume.
 * @param top Upper vertical bound of the near plane.
 * @param bottom Lower vertical bound of the near plane.
 * @param left Left horizontal bound of the near plane.
 * @param right Right horizontal bound of the near plane.
 * @param near Positive near-plane distance.
 * @param far Far-plane distance greater than near.
 * @param matrix Affine transform placing the volume.
 */
void ViewVolume::SetFrustum(float top, float bottom, float left, float right, float near, float far,
                            const nn::util::Matrix4x3fType& matrix) {
    nn::util::Vector3fType points[8];
    float ratio = (1.0f / near) * far;
    points[0]._v = Vector(left, top, -near);
    points[1]._v = Vector(right, top, -near);
    points[2]._v = Vector(right, bottom, -near);
    points[3]._v = Vector(left, bottom, -near);
    points[4]._v = Vector(ratio * left, ratio * top, -far);
    points[5]._v = Vector(ratio * right, ratio * top, -far);
    points[6]._v = Vector(ratio * right, ratio * bottom, -far);
    points[7]._v = Vector(ratio * left, ratio * bottom, -far);

    for (int i = 0; i < 8; ++i) {
        points[i]._v = TransformPosition(points[i]._v, matrix);
    }
    bounds.Set(points, 8);
    nn::util::Vector3fType origin = {matrix._m.val[3]};
    planes[2].Set(points[0], points[1], points[2]);
    planes[0].Set(origin, points[3], points[0]);
    planes[1].Set(origin, points[1], points[2]);
    planes[4].Set(origin, points[0], points[1]);
    planes[5].Set(origin, points[2], points[3]);
    planes[3].Set(points[4], points[7], points[6]);
    planeCount = 6;
    useBounds = 0;
}

/**
 * @brief Construct the clipping planes and bounds of a viewing volume.
 * @param top Upper vertical bound of the near plane.
 * @param bottom Lower vertical bound of the near plane.
 * @param left Left horizontal bound of the near plane.
 * @param right Right horizontal bound of the near plane.
 * @param near Positive near-plane distance.
 * @param far Far-plane distance greater than near.
 * @param matrix Affine transform placing the volume.
 */
void ViewVolume::SetOrtho(float top, float bottom, float left, float right, float near, float far,
                          const nn::util::Matrix4x3fType& matrix) {
    nn::util::Vector3fType points[8];
    points[0]._v = Vector(left, top, -near);
    points[1]._v = Vector(right, top, -near);
    points[2]._v = Vector(right, bottom, -near);
    points[3]._v = Vector(left, bottom, -near);
    points[4]._v = Vector(left, top, -far);
    points[5]._v = Vector(right, top, -far);
    points[6]._v = Vector(right, bottom, -far);
    points[7]._v = Vector(left, bottom, -far);

    for (int i = 0; i < 8; ++i) {
        points[i]._v = TransformPosition(points[i]._v, matrix);
    }
    bounds.Set(points, 8);
    planes[0].Set(points[0], points[7], points[4]);
    planes[1].Set(points[1], points[5], points[6]);
    planes[3].Set(points[4], points[7], points[6]);
    planes[4].Set(points[0], points[4], points[5]);
    planes[2].Set(points[0], points[1], points[2]);
    planes[5].Set(points[2], points[6], points[7]);
    planeCount = 6;
    useBounds = 0;
}

/**
 * @brief Test whether a sphere intersects the viewing volume.
 * @param shape Sphere with a nonnegative radius in the volume coordinate system.
 * @return True unless the sphere lies wholly outside a clipping plane.
 */
bool ViewVolume::TestIntersection(const Sphere& shape) const {
    for (int i = 0; i < planeCount; ++i) {
        if (Dot(planes[i].normal._v, shape.center._v) + planes[i].distance > shape.radius) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Classify a sphere against the viewing volume.
 * @param shape Sphere with a nonnegative radius in the volume coordinate system.
 * @return Minus one outside, zero intersecting, or one fully inside.
 */
int ViewVolume::TestIntersectionEx(const Sphere& shape) const {
    int result = 1;

    for (int i = 0; i < planeCount; ++i) {
        float distance = Dot(planes[i].normal._v, shape.center._v) + planes[i].distance;

        if (distance > shape.radius) {

            return -1;
        }

        if (distance >= -shape.radius) {

            result = 0;
        }
    }

    return result;
}

/**
 * @brief Test a box against the optional bounds and active clipping planes.
 * @param shape Axis-aligned box in the volume coordinate system.
 * @return True unless the box lies wholly outside the volume.
 */
bool ViewVolume::TestIntersection(const Aabb& shape) const {
    if (useBounds) {
        if (bounds.minimum._v[0] > shape.maximum._v[0]) {
            return false;
        }

        if (shape.minimum._v[0] > bounds.maximum._v[0]) {

            return false;
        }

        if (bounds.minimum._v[1] > shape.maximum._v[1]) {

            return false;
        }

        if (shape.minimum._v[1] > bounds.maximum._v[1]) {

            return false;
        }

        if (bounds.minimum._v[2] > shape.maximum._v[2]) {

            return false;
        }

        if (shape.minimum._v[2] > bounds.maximum._v[2]) {

            return false;
        }
    }

    for (int i = 0; i < planeCount; ++i) {
        float32x4_t normal = planes[i].normal._v;
        float32x4_t low = vdupq_n_f32(0);
        low = vsetq_lane_f32(normal[0] >= 0 ? shape.minimum._v[0] : shape.maximum._v[0], low, 0);
        low = vsetq_lane_f32(normal[1] >= 0 ? shape.minimum._v[1] : shape.maximum._v[1], low, 1);
        low = vsetq_lane_f32(normal[2] >= 0 ? shape.minimum._v[2] : shape.maximum._v[2], low, 2);
        float projected = Dot(normal, low);
        float distance = planes[i].distance;

        if (projected + distance > 0) {

            return false;
        }
    }

    return true;
}

/**
 * @brief Classify a box against the optional bounds and active clipping planes.
 * @param shape Axis-aligned box in the volume coordinate system.
 * @return Minus one outside, zero intersecting, or one fully inside.
 */
int ViewVolume::TestIntersectionEx(const Aabb& shape) const {
    if (useBounds) {
        if (bounds.minimum._v[0] > shape.maximum._v[0]) {
            return -1;
        }

        if (shape.minimum._v[0] > bounds.maximum._v[0]) {

            return -1;
        }

        if (bounds.minimum._v[1] > shape.maximum._v[1]) {

            return -1;
        }

        if (shape.minimum._v[1] > bounds.maximum._v[1]) {

            return -1;
        }

        if (bounds.minimum._v[2] > shape.maximum._v[2]) {

            return -1;
        }

        if (shape.minimum._v[2] > bounds.maximum._v[2]) {

            return -1;
        }
    }

    int result = 1;

    for (int i = 0; i < planeCount; ++i) {
        float32x4_t normal = planes[i].normal._v;
        float low0 = normal[0] >= 0 ? shape.minimum._v[0] : shape.maximum._v[0];
        float high0 = normal[0] >= 0 ? shape.maximum._v[0] : shape.minimum._v[0];
        float low1 = normal[1] >= 0 ? shape.minimum._v[1] : shape.maximum._v[1];
        float high1 = normal[1] >= 0 ? shape.maximum._v[1] : shape.minimum._v[1];
        float low2 = normal[2] >= 0 ? shape.minimum._v[2] : shape.maximum._v[2];
        float high2 = normal[2] >= 0 ? shape.maximum._v[2] : shape.minimum._v[2];
        float32x4_t low = Vector(low0, low1, low2);
        float projected = Dot(normal, low);
        float distance = planes[i].distance;

        if (projected + distance > 0) {

            return -1;
        }

        if (result && distance + Dot(normal, Vector(high0, high1, high2)) >= 0) {

            result = 0;
        }
    }

    return result;
}

/**
 * @brief Intersect two sorted submesh-range lists.
 * @param output Writable array large enough for every overlap and a zero-count terminator.
 * @param first Sorted input ranges terminated by a range whose count is zero.
 * @param second Second sorted input list with the same termination convention.
 * @return Number of overlap ranges written, excluding the terminator.
 */
int SubMeshRange::And(SubMeshRange* output, const SubMeshRange* first, const SubMeshRange* second) {
    const SubMeshRange* early = first;
    const SubMeshRange* late = second;

    if (early->start > late->start) {

        std::swap(early, late);
    }
    int count = 0;

    while (early->count != 0 && late->count != 0) {
        int end = early->start + early->count;
        if (end <= late->start) {
            ++early;
            if (early->start > late->start) {
                std::swap(early, late);
            }
        } else if (end < late->start + late->count) {
            output->start = late->start;
            output->count = end - late->start;
            output->detail = std::max(early->detail, late->detail);
            ++early;
            ++count;
            ++output;
            std::swap(early, late);
        } else {
            *output = *late;
            output->detail = std::max(early->detail, late->detail);
            ++late;
            ++output;
            ++count;
        }
    }

    output->detail = 0;
    output->start = 0;
    output->count = 0;
    return count;
}
} // namespace nn::g3d
