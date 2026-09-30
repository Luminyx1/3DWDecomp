#include <nn/g3d/g3d_ViewVolume.h>
#include <nn/util/util_VectorApi.h>
#include <nn/util/util_Arithmetic.h>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace nn::g3d {
namespace {
// value contains a vector product; return the replicated sum of its lanes.
inline float32x4_t Sum(float32x4_t value) {
    float32x2_t sum = vadd_f32(vget_high_f32(value), vget_low_f32(value));
    sum = vpadd_f32(sum, sum);
    return vcombine_f32(sum, sum);
}
// first and second are padded three-component vectors whose dot product is returned.
inline float Dot(float32x4_t first, float32x4_t second) {
    return vgetq_lane_f32(Sum(vmulq_f32(first, second)), 0);
}
// x/y/z supply vector components; its padding component is zero.
inline float32x4_t Vector(float x, float y, float z) {
    nn::util::Vector3fType result;
    nn::util::VectorSet(&result, x, y, z);
    return result._v;
}
// value is transformed as a position by matrix, including translation.
inline float32x4_t TransformPosition(float32x4_t value, const nn::util::Matrix4x3fType& matrix) {
    float32x4_t x = matrix._m.val[0], y = matrix._m.val[1], z = matrix._m.val[2], translation = matrix._m.val[3];
    float32x4_t result = vmulq_laneq_f32(x, value, 0);
    result = vfmaq_laneq_f32(result, y, value, 1);
    result = vfmaq_laneq_f32(result, z, value, 2);
    return vaddq_f32(translation, result);
}
// first and second are vectors; select componentwise bounds while clearing padding.
inline float32x4_t Minimum(float32x4_t first, float32x4_t second) {
    return Vector(std::min(first[0], second[0]), std::min(first[1], second[1]), std::min(first[2], second[2]));
}
inline float32x4_t Maximum(float32x4_t first, float32x4_t second) {
    return Vector(std::max(first[0], second[0]), std::max(first[1], second[1]), std::max(first[2], second[2]));
}
// first and second are vectors whose cross product is returned.
inline float32x4_t Cross(float32x4_t first, float32x4_t second) {
    uint8x8_t yz = {4,5,6,7,8,9,10,11};
    uint8x8_t xw = {0,1,2,3,12,13,14,15};
    uint8x8_t zx = {8,9,10,11,0,1,2,3};
    uint8x8_t yw = {4,5,6,7,12,13,14,15};
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
// value is normalized with two reciprocal-square-root refinements; zero remains zero.
inline float32x4_t Normalize(float32x4_t value) {
    float32x4_t length = Sum(vmulq_f32(value, value));
    float32x4_t inverse = vrsqrteq_f32(length);
    inverse = vmulq_f32(inverse, vrsqrtsq_f32(inverse, vmulq_f32(inverse, length)));
    inverse = vmulq_f32(inverse, vrsqrtsq_f32(inverse, vmulq_f32(length, inverse)));
    return vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(vmulq_f32(value, inverse)), vmvnq_u32(vceqzq_f32(length))));
}
}
// source is transformed by matrix; the largest basis scale expands its radius.
void Sphere::Transform(const Sphere& source, const nn::util::Matrix4x3fType& matrix) {
    center._v = TransformPosition(source.center._v, matrix);
    float x = vgetq_lane_f32(vsqrtq_f32(Sum(vmulq_f32(matrix._m.val[0], matrix._m.val[0]))), 0);
    float y = vgetq_lane_f32(vsqrtq_f32(Sum(vmulq_f32(matrix._m.val[1], matrix._m.val[1]))), 0);
    float z = vgetq_lane_f32(vsqrtq_f32(Sum(vmulq_f32(matrix._m.val[2], matrix._m.val[2]))), 0);
    float scale = std::max(std::max(x, y), z);
    radius = source.radius * scale;
}
// first and second are enclosed by the resulting sphere, including containment cases.
void Sphere::Merge(const Sphere& first, const Sphere& second) {
    float32x4_t difference = vsubq_f32(second.center._v, first.center._v);
    float distanceSquared = Dot(difference, difference);
    float radiusDifference = second.radius - first.radius;
    if (distanceSquared < vget_lane_f32(vmax_f32(vdup_n_f32(radiusDifference * radiusDifference), vdup_n_f32(0.0001f)), 0)) {
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
// points supplies count positions; the first remains the result when count is nonpositive.
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
// source provides center and extents; matrix produces the enclosing world-space box.
void Aabb::Transform(const Bounding& source, const nn::util::Matrix4x3fType& matrix) {
    float32x4_t x = Vector(fabsf(matrix._m.val[0][0]), fabsf(matrix._m.val[0][1]), fabsf(matrix._m.val[0][2]));
    float32x4_t y = Vector(fabsf(matrix._m.val[1][0]), fabsf(matrix._m.val[1][1]), fabsf(matrix._m.val[1][2]));
    float32x4_t z = Vector(fabsf(matrix._m.val[2][0]), fabsf(matrix._m.val[2][1]), fabsf(matrix._m.val[2][2]));
    float32x4_t center = TransformPosition(vcombine_f32(vld1_f32(&source.center.x), vreinterpret_f32_u64(vcreate_u64(*reinterpret_cast<const u32*>(&source.center.z)))), matrix);
    float32x4_t extent = vmulq_n_f32(x, source.extent.x);
    extent = vaddq_f32(extent, vmulq_n_f32(y, source.extent.y));
    extent = vaddq_f32(extent, vmulq_n_f32(z, source.extent.z));
    minimum._v = vsubq_f32(center, extent);
    maximum._v = vaddq_f32(center, extent);
}
// first and second provide the coordinate bounds to combine.
void Aabb::Merge(const Aabb& first, const Aabb& second) {
    minimum._v = Minimum(first.minimum._v, second.minimum._v);
    maximum._v = Maximum(first.maximum._v, second.maximum._v);
}
// first, second and third define a plane; winding controls the normal direction.
void Plane::Set(const nn::util::Vector3fType& first, const nn::util::Vector3fType& second, const nn::util::Vector3fType& third) {
    normal._v = Normalize(Cross(vsubq_f32(third._v, first._v), vsubq_f32(second._v, first._v)));
    distance = -Dot(first._v, normal._v);
}
// fovy/aspect describe the lens; near/far give depths; matrix places the frustum.
void ViewVolume::SetPerspective(float fovy, float aspect, float near, float far, const nn::util::Matrix4x3fType& matrix) {
    auto angle = nn::util::RadianToAngleIndex(fovy * 0.5f);
    float tangent = nn::util::SinTable(angle) / nn::util::CosTable(angle);
    float top = tangent * near;
    SetFrustum(top, -top, -(top * aspect), top * aspect, near, far, matrix);
}
// top/bottom/left/right define the lens bounds; near/far give depths; matrix places the volume.
void ViewVolume::SetFrustum(float top, float bottom, float left, float right, float near, float far, const nn::util::Matrix4x3fType& matrix) {
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
    for (int i = 0; i < 8; ++i) points[i]._v = TransformPosition(points[i]._v, matrix);
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
// top/bottom/left/right define the lens bounds; near/far give depths; matrix places the volume.
void ViewVolume::SetOrtho(float top, float bottom, float left, float right, float near, float far, const nn::util::Matrix4x3fType& matrix) {
    nn::util::Vector3fType points[8];
    points[0]._v = Vector(left, top, -near);
    points[1]._v = Vector(right, top, -near);
    points[2]._v = Vector(right, bottom, -near);
    points[3]._v = Vector(left, bottom, -near);
    points[4]._v = Vector(left, top, -far);
    points[5]._v = Vector(right, top, -far);
    points[6]._v = Vector(right, bottom, -far);
    points[7]._v = Vector(left, bottom, -far);
    for (int i = 0; i < 8; ++i) points[i]._v = TransformPosition(points[i]._v, matrix);
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
// shape is accepted unless its sphere lies wholly outside one of the volume's planes.
bool ViewVolume::TestIntersection(const Sphere& shape) const {
    for (int i = 0; i < planeCount; ++i) {
        if (Dot(planes[i].normal._v, shape.center._v) + planes[i].distance > shape.radius) return false;
    }
    return true;
}
// shape is classified as outside (-1), intersecting (0), or fully inside (1).
int ViewVolume::TestIntersectionEx(const Sphere& shape) const {
    int result = 1;
    for (int i = 0; i < planeCount; ++i) {
        float distance = Dot(planes[i].normal._v, shape.center._v) + planes[i].distance;
        if (distance > shape.radius) return -1;
        if (distance >= -shape.radius) result = 0;
    }
    return result;
}
// shape supplies the axis-aligned box to test against the volume.
bool ViewVolume::TestIntersection(const Aabb& shape) const {
    if (useBounds) {
        if (bounds.minimum._v[0] > shape.maximum._v[0]) return false;
        if (shape.minimum._v[0] > bounds.maximum._v[0]) return false;
        if (bounds.minimum._v[1] > shape.maximum._v[1]) return false;
        if (shape.minimum._v[1] > bounds.maximum._v[1]) return false;
        if (bounds.minimum._v[2] > shape.maximum._v[2]) return false;
        if (shape.minimum._v[2] > bounds.maximum._v[2]) return false;
    }
    for (int i = 0; i < planeCount; ++i) {
        float32x4_t normal = planes[i].normal._v;
        float32x4_t low = vdupq_n_f32(0);
        low = vsetq_lane_f32(normal[0] >= 0 ? shape.minimum._v[0] : shape.maximum._v[0], low, 0);
        low = vsetq_lane_f32(normal[1] >= 0 ? shape.minimum._v[1] : shape.maximum._v[1], low, 1);
        low = vsetq_lane_f32(normal[2] >= 0 ? shape.minimum._v[2] : shape.maximum._v[2], low, 2);
        float projected = Dot(normal, low);
        float distance = planes[i].distance;
        if (projected + distance > 0) return false;
    }
    return true;
}
// shape supplies the axis-aligned box to test against the volume.
int ViewVolume::TestIntersectionEx(const Aabb& shape) const {
    if (useBounds) {
        if (bounds.minimum._v[0] > shape.maximum._v[0]) return -1;
        if (shape.minimum._v[0] > bounds.maximum._v[0]) return -1;
        if (bounds.minimum._v[1] > shape.maximum._v[1]) return -1;
        if (shape.minimum._v[1] > bounds.maximum._v[1]) return -1;
        if (bounds.minimum._v[2] > shape.maximum._v[2]) return -1;
        if (shape.minimum._v[2] > bounds.maximum._v[2]) return -1;
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
        if (projected + distance > 0) return -1;
        if (result && distance + Dot(normal, Vector(high0, high1, high2)) >= 0) result = 0;
    }
    return result;
}
// output receives overlaps from sorted, count-zero-terminated first and second range lists.
int SubMeshRange::And(SubMeshRange* output, const SubMeshRange* first, const SubMeshRange* second) {
    const SubMeshRange* early = first;
    const SubMeshRange* late = second;
    if (early->start > late->start) std::swap(early, late);
    int count = 0;
    while (early->count) {
        if (!late->count) break;
        unsigned end = early->start + early->count;
        if (end <= late->start) {
            ++early;
            if (early->start > late->start) std::swap(early, late);
            continue;
        }
        if (end < unsigned(late->start + late->count)) {
            output->start = late->start;
            output->count = end - late->start;
            output->detail = std::max(early->detail, late->detail);
            const SubMeshRange* next = early + 1;
            ++count;
            ++output;
            early = late;
            late = next;
            continue;
        }
        *output = *late;
        output->detail = std::max(early->detail, late->detail);
        ++late;
        ++output;
        ++count;
    }
    output->detail = 0;
    output->start = 0;
    output->count = 0;
    return count;
}
}
