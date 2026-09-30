#pragma once
#include <nn/types.h>
#include <nn/util/util_MathTypes.h>

namespace nn::g3d {
class Sphere {
public:
    // source is the input sphere; matrix transforms its center and scales its radius.
    void Transform(const Sphere& source, const nn::util::Matrix4x3fType& matrix);
    // first and second are the spheres to enclose.
    void Merge(const Sphere& first, const Sphere& second);
    nn::util::Vector3fType center;
    float radius;
};
struct Bounding {
    nn::util::Float3 center;
    nn::util::Float3 extent;
};
class Aabb {
public:
    // points contains count positions to enclose; at least one position must be readable.
    void Set(const nn::util::Vector3fType* points, int count);
    // source is the local box; matrix transforms it into an axis-aligned box.
    void Transform(const Bounding& source, const nn::util::Matrix4x3fType& matrix);
    // first and second are the axis-aligned boxes to enclose.
    void Merge(const Aabb& first, const Aabb& second);
    nn::util::Vector3fType minimum;
    nn::util::Vector3fType maximum;
};
class Plane {
public:
    // first, second and third define the plane and its outward winding.
    void Set(const nn::util::Vector3fType& first, const nn::util::Vector3fType& second, const nn::util::Vector3fType& third);
    nn::util::Vector3fType normal;
    float distance;
};
struct SubMeshRange {
    // output receives intersections of the count-zero-terminated first and second lists.
    static int And(SubMeshRange* output, const SubMeshRange* first, const SubMeshRange* second);
    u16 start;
    u16 count;
    u16 detail;
};
static_assert(sizeof(Sphere) == 0x20, "sphere size");
static_assert(sizeof(Aabb) == 0x20, "axis-aligned box size");
static_assert(sizeof(Plane) == 0x20, "plane size");
static_assert(sizeof(Bounding) == 0x18, "local bounding box size");
static_assert(sizeof(SubMeshRange) == 6, "submesh range size");
}
