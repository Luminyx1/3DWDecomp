#include <math/seadGeometry.h>
#include <math/seadMathCalcCommon.h>

namespace sead
{
/**
 * Calculates the squared distance between a point and a line.
 * @param rPoint Point.
 * @param rLine Line.
 * @param pT Receives the parameter of the closest point on the line; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistancePointToLine(const Vector2f& rPoint, const Line<Vector2f>& rLine,
                                             f32* pT)
{
    const f32 t = (rPoint - rLine.getPos()).dot(rLine.getDir());
    const Vector2f closest = rLine.getPos() + rLine.getDir() * t;
    const f32 distance = (rPoint - closest).squaredLength();

    if (pT)
    {
        *pT = t;
    }

    return distance;
}

/**
 * Calculates the squared distance between a point and a line.
 * @param rPoint Point.
 * @param rLine Line.
 * @param pT Receives the parameter of the closest point on the line; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistancePointToLine(const Vector3f& rPoint, const Line<Vector3f>& rLine,
                                             f32* pT)
{
    Vector3f diff = rPoint;
    diff -= rLine.getPos();
    const f32 t = diff.dot(rLine.getDir());
    const Vector3f closest = rLine.getPos() + rLine.getDir() * t;
    const f32 distance = (rPoint - closest).squaredLength();

    if (pT)
    {
        *pT = t;
    }

    return distance;
}

// NON_MATCHING: the target stores pT through an integer select
f32 Geometry::calcSquaredDistancePointToRay(const Vector2f& rPoint, const Ray<Vector2f>& rRay,
                                            f32* pT)
{
    const Vector2f diff = rPoint - rRay.getPos();
    f32 t = diff.dot(rRay.getDir());
    f32 distance = (rPoint - (rRay.getPos() + rRay.getDir() * t)).squaredLength();

    if (t < 0.0f)
    {
        t = 0.0f;
        distance = diff.squaredLength();
    }

    if (pT)
    {
        *pT = t;
    }

    return distance;
}

// NON_MATCHING: the target stores pT through an integer select
f32 Geometry::calcSquaredDistancePointToRay(const Vector3f& rPoint, const Ray<Vector3f>& rRay,
                                            f32* pT)
{
    const Vector3f diff = rPoint - rRay.getPos();
    f32 t = diff.dot(rRay.getDir());
    f32 distance = (rPoint - (rRay.getPos() + rRay.getDir() * t)).squaredLength();

    if (t < 0.0f)
    {
        t = 0.0f;
        distance = diff.squaredLength();
    }

    if (pT)
    {
        *pT = t;
    }

    return distance;
}

// NON_MATCHING: register allocation
f32 Geometry::calcSquaredDistancePointToSegment(const Vector2f& rPoint,
                                                const Segment<Vector2f>& rSegment, f32* pT)
{
    const Vector2f dir = rSegment.getPos1() - rSegment.getPos0();
    f32 t = dir.dot(rPoint - rSegment.getPos0());
    Vector2f closest;

    if (t <= 0.0f)
    {
        t = 0.0f;
        closest = rSegment.getPos0();
    }
    else
    {
        const f32 lengthSq = dir.squaredLength();

        if (t >= lengthSq)
        {
            t = 1.0f;
            closest = rSegment.getPos1();
        }
        else
        {
            t /= lengthSq;
            closest = rSegment.getPos0() + dir * t;
        }
    }

    if (pT)
    {
        *pT = t;
    }

    return (closest - rPoint).squaredLength();
}

// NON_MATCHING: register allocation
f32 Geometry::calcSquaredDistancePointToSegment(const Vector3f& rPoint,
                                                const Segment<Vector3f>& rSegment, f32* pT)
{
    const Vector3f dir = rSegment.getPos1() - rSegment.getPos0();
    f32 t = dir.dot(rPoint - rSegment.getPos0());
    Vector3f closest;

    if (t <= 0.0f)
    {
        t = 0.0f;
        closest = rSegment.getPos0();
    }
    else
    {
        const f32 lengthSq = dir.squaredLength();

        if (t >= lengthSq)
        {
            t = 1.0f;
            closest = rSegment.getPos1();
        }
        else
        {
            t /= lengthSq;
            closest = rSegment.getPos0() + dir * t;
        }
    }

    if (pT)
    {
        *pT = t;
    }

    return (closest - rPoint).squaredLength();
}

/**
 * Calculates the squared distance between a point and a plane.
 * @param rPoint Point.
 * @param rPlane Plane.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistancePointToPlane(const Vector2f& rPoint, const Plane2<f32>& rPlane)
{
    const Vector2f point = rPoint;
    const Plane2<f32> plane = rPlane;
    const f32 distance = point.dot(plane.getNormal()) - plane.getD();
    return distance * distance;
}

/**
 * Calculates the squared distance between a point and a plane.
 * @param rPoint Point.
 * @param rPlane Plane.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistancePointToPlane(const Vector3f& rPoint, const Plane3<f32>& rPlane)
{
    const Vector3f point = rPoint;
    const f32 distance = point.dot(rPlane.getNormal()) - rPlane.getD();
    return distance * distance;
}

/**
 * Calculates the squared distance between a sphere and a plane.
 * @param rSphere Sphere.
 * @param rPlane Plane.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceSphereToPlane(const Sphere<Vector2f>& rSphere,
                                               const Plane2<f32>& rPlane)
{
    const Plane2<f32> plane = rPlane;
    const f32 distance = plane.getNormal().dot(rSphere.getCenter()) - plane.getD();
    const f32 radius = rSphere.getRadius();

    if (distance > radius)
    {
        return (distance - radius) * (distance - radius);
    }

    if (distance < -radius)
    {
        return (radius + distance) * (radius + distance);
    }

    return 0.0f;
}

/**
 * Calculates the squared distance between a sphere and a plane.
 * @param rSphere Sphere.
 * @param rPlane Plane.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceSphereToPlane(const Sphere<Vector3f>& rSphere,
                                               const Plane3<f32>& rPlane)
{
    const f32 distance = rPlane.getNormal().dot(rSphere.getCenter()) - rPlane.getD();
    const f32 radius = rSphere.getRadius();

    if (distance > radius)
    {
        return (distance - radius) * (distance - radius);
    }

    if (distance < -radius)
    {
        return (radius + distance) * (radius + distance);
    }

    return 0.0f;
}

/**
 * Calculates the squared distance between a point and an axis-aligned box.
 * @param rPoint Point.
 * @param rBox Axis-aligned box.
 * @param pClosest Receives the closest point inside the box; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistancePointToAABB(const Vector2f& rPoint, const BoundBox2<f32>& rBox,
                                             Vector2f* pClosest)
{
    f32 distance = 0.0f;
    {
        f32 closest = rPoint.x;

        if (rPoint.x < rBox.getMin().x)
        {
            closest = rBox.getMin().x;
            distance += (closest - rPoint.x) * (closest - rPoint.x);
        }
        else if (rPoint.x > rBox.getMax().x)
        {
            closest = rBox.getMax().x;
            distance += (closest - rPoint.x) * (closest - rPoint.x);
        }

        if (pClosest)
        {
            pClosest->x = closest;
        }
    }

    {
        f32 closest = rPoint.y;

        if (rPoint.y < rBox.getMin().y)
        {
            closest = rBox.getMin().y;
            distance += (closest - rPoint.y) * (closest - rPoint.y);
        }
        else if (rPoint.y > rBox.getMax().y)
        {
            closest = rBox.getMax().y;
            distance += (closest - rPoint.y) * (closest - rPoint.y);
        }

        if (pClosest)
        {
            pClosest->y = closest;
        }
    }

    return distance;
}

/**
 * Calculates the squared distance between a point and an axis-aligned box.
 * @param rPoint Point.
 * @param rBox Axis-aligned box.
 * @param pClosest Receives the closest point inside the box; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistancePointToAABB(const Vector3f& rPoint, const BoundBox3<f32>& rBox,
                                             Vector3f* pClosest)
{
    f32 distance = 0.0f;
    {
        f32 closest = rPoint.x;

        if (rPoint.x < rBox.getMin().x)
        {
            closest = rBox.getMin().x;
            distance += (closest - rPoint.x) * (closest - rPoint.x);
        }
        else if (rPoint.x > rBox.getMax().x)
        {
            closest = rBox.getMax().x;
            distance += (closest - rPoint.x) * (closest - rPoint.x);
        }

        if (pClosest)
        {
            pClosest->x = closest;
        }
    }

    {
        f32 closest = rPoint.y;

        if (rPoint.y < rBox.getMin().y)
        {
            closest = rBox.getMin().y;
            distance += (closest - rPoint.y) * (closest - rPoint.y);
        }
        else if (rPoint.y > rBox.getMax().y)
        {
            closest = rBox.getMax().y;
            distance += (closest - rPoint.y) * (closest - rPoint.y);
        }

        if (pClosest)
        {
            pClosest->y = closest;
        }
    }

    {
        f32 closest = rPoint.z;

        if (rPoint.z < rBox.getMin().z)
        {
            closest = rBox.getMin().z;
            distance += (closest - rPoint.z) * (closest - rPoint.z);
        }
        else if (rPoint.z > rBox.getMax().z)
        {
            closest = rBox.getMax().z;
            distance += (closest - rPoint.z) * (closest - rPoint.z);
        }

        if (pClosest)
        {
            pClosest->z = closest;
        }
    }

    return distance;
}

namespace
{
template <typename T>
bool isNearlyZero(T value)
{
    return value >= -MathCalcCommon<T>::epsilon() && value <= MathCalcCommon<T>::epsilon();
}

/**
 * Calculates the squared distance between two segments.
 * @param rSegment0 First segment.
 * @param rSegment1 Second segment.
 * @param pT0 Receives the parameter of the closest point on the first segment; may be null.
 * @param pT1 Receives the parameter of the closest point on the second segment; may be null.
 * @return Squared distance.
 */
template <typename T>
f32 calcSquaredDistanceSegmentToSegment_(const Segment<T>& rSegment0, const Segment<T>& rSegment1,
                                         f32* pT0, f32* pT1)
{
    const T r = rSegment0.getPos0() - rSegment1.getPos0();
    const T d0 = rSegment0.getPos1() - rSegment0.getPos0();
    const T d1 = rSegment1.getPos1() - rSegment1.getPos0();
    const f32 a = d0.dot(d0);
    const f32 e = d1.dot(d1);
    const f32 f = r.dot(d1);

    f32 t;
    f32 s;

    if (isNearlyZero(a))
    {
        if (isNearlyZero(e))
        {
            if (pT0)
            {
                *pT0 = 0.0f;
            }

            if (pT1)
            {
                *pT1 = 0.0f;
            }

            return (rSegment0.getPos0() - rSegment1.getPos0()).squaredLength();
        }

        s = 0.0f;
        t = f / e;

        if (t < 0.0f)
        {
            t = 0.0f;
        }
        else if (t > 1.0f)
        {
            t = 1.0f;
        }
    }
    else
    {
        if (isNearlyZero(e))
        {
            const f32 c = r.dot(d0);
            t = 0.0f;
            s = -c / a;

            if (s < 0.0f)
            {
                s = 0.0f;
            }
            else if (s > 1.0f)
            {
                s = 1.0f;
            }
        }
        else
        {
            const f32 c = r.dot(d0);
            const f32 b = d0.dot(d1);
            const f32 denom = a * e - b * b;
            s = 0.0f;

            if (!isNearlyZero(denom))
            {
                s = (b * f - c * e) / denom;

                if (s < 0.0f)
                {
                    s = 0.0f;
                }
                else if (s > 1.0f)
                {
                    s = 1.0f;
                }
            }

            const f32 tnom = f + b * s;

            if (tnom < 0.0f)
            {
                t = 0.0f;
                s = -c / a;

                if (s < 0.0f)
                {
                    s = 0.0f;
                }
                else if (s > 1.0f)
                {
                    s = 1.0f;
                }
            }
            else if (tnom > e)
            {
                t = 1.0f;
                s = (b - c) / a;

                if (s < 0.0f)
                {
                    s = 0.0f;
                }
                else if (s > 1.0f)
                {
                    s = 1.0f;
                }
            }
            else
            {
                t = tnom / e;
            }
        }
    }

    const T c0 = rSegment0.getPos0() + d0 * s;
    const T c1 = rSegment1.getPos0() + d1 * t;

    if (pT0)
    {
        *pT0 = s;
    }

    if (pT1)
    {
        *pT1 = t;
    }

    return (c0 - c1).squaredLength();
}

/**
 * Calculates the squared distance between a line and a segment.
 * @param rLine Line.
 * @param rSegment Segment.
 * @param pT0 Receives the parameter of the closest point on the line; may be null.
 * @param pT1 Receives the parameter of the closest point on the segment; may be null.
 * @return Squared distance.
 */
template <typename T>
f32 calcSquaredDistanceLineToSegment_(const Line<T>& rLine, const Segment<T>& rSegment, f32* pT0,
                                      f32* pT1)
{
    const T r = rLine.getPos() - rSegment.getPos0();
    const T d = rSegment.getPos1() - rSegment.getPos0();
    const f32 c = r.dot(rLine.getDir());
    const f32 e = d.dot(d);

    f32 s;
    f32 t;

    if (isNearlyZero(e))
    {
        s = 0.0f;
        t = -c;
    }
    else
    {
        const f32 b = d.dot(rLine.getDir());
        const f32 denom = e - b * b;
        s = 0.0f;

        if (!isNearlyZero(denom))
        {
            s = (r.dot(d) - b * c) / denom;

            if (s < 0.0f)
            {
                s = 0.0f;
            }
            else if (s > 1.0f)
            {
                s = 1.0f;
            }
        }

        t = b * s - c;
    }

    const T c0 = rLine.getPos() + rLine.getDir() * t;
    const T c1 = rSegment.getPos0() + d * s;

    if (pT0)
    {
        *pT0 = t;
    }

    if (pT1)
    {
        *pT1 = s;
    }

    return (c0 - c1).squaredLength();
}

/**
 * Calculates the squared distance between a ray and a segment.
 * @param rRay Ray.
 * @param rSegment Segment.
 * @param pT0 Receives the parameter of the closest point on the ray; may be null.
 * @param pT1 Receives the parameter of the closest point on the segment; may be null.
 * @return Squared distance.
 */
template <typename T>
f32 calcSquaredDistanceRayToSegment_(const Ray<T>& rRay, const Segment<T>& rSegment, f32* pT0,
                                     f32* pT1)
{
    const T r = rRay.getPos() - rSegment.getPos0();
    const T d = rSegment.getPos1() - rSegment.getPos0();
    const f32 c = r.dot(rRay.getDir());
    const f32 e = d.dot(d);

    f32 s;
    f32 t;

    if (isNearlyZero(e))
    {
        s = 0.0f;
        t = Mathf::max(0.0f, -c);
    }
    else
    {
        const f32 b = d.dot(rRay.getDir());
        const f32 denom = e - b * b;
        const f32 f = r.dot(d);
        s = 0.0f;

        if (!isNearlyZero(denom))
        {
            s = (f - b * c) / denom;

            if (s < 0.0f)
            {
                s = 0.0f;
            }
            else if (s > 1.0f)
            {
                s = 1.0f;
            }
        }

        t = b * s - c;

        if (t < 0.0f)
        {
            t = 0.0f;
            s = f / e;

            if (s < 0.0f)
            {
                s = 0.0f;
            }
            else if (s > 1.0f)
            {
                s = 1.0f;
            }
        }
    }

    const T c0 = rRay.getPos() + rRay.getDir() * t;
    const T c1 = rSegment.getPos0() + d * s;

    if (pT0)
    {
        *pT0 = t;
    }

    if (pT1)
    {
        *pT1 = s;
    }

    return (c0 - c1).squaredLength();
}

/**
 * Clips the parametric line pos + dir * t against an axis-aligned box, one axis at a time.
 * @param rPos Origin of the line.
 * @param rDir Direction of the line.
 * @param rMin Minimum corner of the box.
 * @param rMax Maximum corner of the box.
 * @param pT0 Receives the parameter where the line enters the box; may be null.
 * @param pT1 Receives the parameter where the line leaves the box; may be null.
 * @param tMin Lowest parameter accepted.
 * @param tMax Highest parameter accepted.
 * @return true if part of the parameter range lies inside the box.
 */
bool calcIntersectionLineToAABB_(const Vector2f& rPos, const Vector2f& rDir, const Vector2f& rMin,
                                 const Vector2f& rMax, f32* pT0, f32* pT1, f32 tMin, f32 tMax)
{
    if (isNearlyZero(rDir.x))
    {
        if (rPos.x < rMin.x || rPos.x > rMax.x)
        {
            return false;
        }
    }
    else
    {
        const f32 inv = 1.0f / rDir.x;
        const f32 t0 = inv * (rMin.x - rPos.x);
        const f32 t1 = inv * (rMax.x - rPos.x);
        const f32 tFar = t0 > t1 ? t0 : t1;
        const f32 tNear = t0 > t1 ? t1 : t0;
        tMin = Mathf::max(tNear, tMin);
        tMax = Mathf::min(tFar, tMax);

        if (tMin > tMax)
        {
            return false;
        }
    }

    if (isNearlyZero(rDir.y))
    {
        if (rPos.y < rMin.y || rPos.y > rMax.y)
        {
            return false;
        }
    }
    else
    {
        const f32 inv = 1.0f / rDir.y;
        f32 t0 = inv * (rMin.y - rPos.y);
        f32 t1 = inv * (rMax.y - rPos.y);

        if (t0 > t1)
        {
            const f32 tmp = t0;
            t0 = t1;
            t1 = tmp;
        }

        tMin = Mathf::max(t0, tMin);
        tMax = Mathf::min(t1, tMax);

        if (tMin > tMax)
        {
            return false;
        }
    }

    if (pT0)
    {
        *pT0 = tMin;
    }

    if (pT1)
    {
        *pT1 = tMax;
    }

    return true;
}

/**
 * Clips the parametric line pos + dir * t against an axis-aligned box, one axis at a time.
 * @param rPos Origin of the line.
 * @param rDir Direction of the line.
 * @param rMin Minimum corner of the box.
 * @param rMax Maximum corner of the box.
 * @param pT0 Receives the parameter where the line enters the box; may be null.
 * @param pT1 Receives the parameter where the line leaves the box; may be null.
 * @param tMin Lowest parameter accepted.
 * @param tMax Highest parameter accepted.
 * @return true if part of the parameter range lies inside the box.
 */
bool calcIntersectionLineToAABB_(const Vector3f& rPos, const Vector3f& rDir, const Vector3f& rMin,
                                 const Vector3f& rMax, f32* pT0, f32* pT1, f32 tMin, f32 tMax)
{
    if (isNearlyZero(rDir.x))
    {
        if (rPos.x < rMin.x || rPos.x > rMax.x)
        {
            return false;
        }
    }
    else
    {
        const f32 inv = 1.0f / rDir.x;
        f32 t0 = inv * (rMin.x - rPos.x);
        f32 t1 = inv * (rMax.x - rPos.x);

        if (t0 > t1)
        {
            const f32 tmp = t0;
            t0 = t1;
            t1 = tmp;
        }

        tMin = Mathf::max(t0, tMin);
        tMax = Mathf::min(t1, tMax);

        if (tMin > tMax)
        {
            return false;
        }
    }

    if (isNearlyZero(rDir.y))
    {
        if (rPos.y < rMin.y || rPos.y > rMax.y)
        {
            return false;
        }
    }
    else
    {
        const f32 inv = 1.0f / rDir.y;
        f32 t0 = inv * (rMin.y - rPos.y);
        f32 t1 = inv * (rMax.y - rPos.y);

        if (t0 > t1)
        {
            const f32 tmp = t0;
            t0 = t1;
            t1 = tmp;
        }

        tMin = Mathf::max(t0, tMin);
        tMax = Mathf::min(t1, tMax);

        if (tMin > tMax)
        {
            return false;
        }
    }

    if (isNearlyZero(rDir.z))
    {
        if (rPos.z < rMin.z || rPos.z > rMax.z)
        {
            return false;
        }
    }
    else
    {
        const f32 inv = 1.0f / rDir.z;
        f32 t0 = inv * (rMin.z - rPos.z);
        f32 t1 = inv * (rMax.z - rPos.z);

        if (t0 > t1)
        {
            const f32 tmp = t0;
            t0 = t1;
            t1 = tmp;
        }

        tMin = Mathf::max(t0, tMin);
        tMax = Mathf::min(t1, tMax);

        if (tMin > tMax)
        {
            return false;
        }
    }

    if (pT0)
    {
        *pT0 = tMin;
    }

    if (pT1)
    {
        *pT1 = tMax;
    }

    return true;
}
}  // namespace

/**
 * Calculates the squared distance between a line and a line.
 * @param rLine0 First line.
 * @param rLine1 Second line.
 * @param pT0 Receives the parameter of the closest point on the first line; may be null.
 * @param pT1 Receives the parameter of the closest point on the second line; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceLineToLine(const Line<Vector2f>& rLine0,
                                            const Line<Vector2f>& rLine1, f32* pT0, f32* pT1)
{
    const f32 denom = rLine0.getDir().x * rLine1.getDir().y - rLine0.getDir().y * rLine1.getDir().x;

    if (isNearlyZero(denom))
    {
        if (pT0)
        {
            *pT0 = 0.0f;
        }

        return calcSquaredDistancePointToLine(rLine0.getPos(), rLine1, pT1);
    }

    if (pT0)
    {
        const Vector2f diff = rLine1.getPos() - rLine0.getPos();
        *pT0 = (rLine1.getDir().x * diff.y - rLine1.getDir().y * diff.x) / -denom;
    }

    if (pT1)
    {
        const Vector2f diff = rLine1.getPos() - rLine0.getPos();
        *pT1 = (diff.x * rLine0.getDir().y - diff.y * rLine0.getDir().x) / denom;
    }

    return 0.0f;
}

/**
 * Calculates the squared distance between a line and a line.
 * @param rLine0 First line.
 * @param rLine1 Second line.
 * @param pT0 Receives the parameter of the closest point on the first line; may be null.
 * @param pT1 Receives the parameter of the closest point on the second line; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceLineToLine(const Line<Vector3f>& rLine0,
                                            const Line<Vector3f>& rLine1, f32* pT0, f32* pT1)
{
    const Vector3f r = rLine0.getPos() - rLine1.getPos();
    const f32 a = rLine0.getDir().dot(rLine1.getDir());
    const f32 denom = 1.0f - a * a;

    if (isNearlyZero(denom))
    {
        if (pT0)
        {
            *pT0 = 0.0f;
        }

        return calcSquaredDistancePointToLine(rLine0.getPos(), rLine1, pT1);
    }

    const f32 b = r.dot(rLine0.getDir());
    const f32 c = r.dot(rLine1.getDir());
    const f32 t0 = (a * c - b) / denom;
    const f32 t1 = (c - b * a) / denom;
    const Vector3f c0 = rLine0.getPos() + rLine0.getDir() * t0;
    const Vector3f c1 = rLine1.getPos() + rLine1.getDir() * t1;

    if (pT0)
    {
        *pT0 = t0;
    }

    if (pT1)
    {
        *pT1 = t1;
    }

    return (c0 - c1).squaredLength();
}

/**
 * Calculates the squared distance between a ray and a ray.
 * @param rRay0 First ray.
 * @param rRay1 Second ray.
 * @param pT0 Receives the parameter of the closest point on the first ray; may be null.
 * @param pT1 Receives the parameter of the closest point on the second ray; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceRayToRay(const Ray<Vector2f>& rRay0, const Ray<Vector2f>& rRay1,
                                          f32* pT0, f32* pT1)
{
    const Vector2f r = rRay0.getPos() - rRay1.getPos();
    const f32 a = rRay0.getDir().dot(rRay1.getDir());
    const f32 b = r.dot(rRay0.getDir());
    const f32 denom = 1.0f - a * a;
    const f32 c = r.dot(rRay1.getDir());

    f32 t0 = 0.0f;

    if (!isNearlyZero(denom))
    {
        t0 = Mathf::max(0.0f, (a * c - b) / denom);
    }

    f32 t1 = c + a * t0;

    if (t1 < 0.0f)
    {
        t1 = 0.0f;
        t0 = Mathf::max(0.0f, -b);
    }

    const Vector2f c0 = rRay0.getPos() + rRay0.getDir() * t0;
    const Vector2f c1 = rRay1.getPos() + rRay1.getDir() * t1;

    if (pT0)
    {
        *pT0 = t0;
    }

    if (pT1)
    {
        *pT1 = t1;
    }

    return (c0 - c1).squaredLength();
}

/**
 * Calculates the squared distance between a ray and a ray.
 * @param rRay0 First ray.
 * @param rRay1 Second ray.
 * @param pT0 Receives the parameter of the closest point on the first ray; may be null.
 * @param pT1 Receives the parameter of the closest point on the second ray; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceRayToRay(const Ray<Vector3f>& rRay0, const Ray<Vector3f>& rRay1,
                                          f32* pT0, f32* pT1)
{
    const Vector3f r = rRay0.getPos() - rRay1.getPos();
    const f32 a = rRay0.getDir().dot(rRay1.getDir());
    const f32 b = r.dot(rRay0.getDir());
    const f32 denom = 1.0f - a * a;
    const f32 c = r.dot(rRay1.getDir());

    f32 t0 = 0.0f;

    if (!isNearlyZero(denom))
    {
        t0 = Mathf::max(0.0f, (a * c - b) / denom);
    }

    f32 t1 = c + a * t0;

    if (t1 < 0.0f)
    {
        t1 = 0.0f;
        t0 = Mathf::max(0.0f, -b);
    }

    const Vector3f c0 = rRay0.getPos() + rRay0.getDir() * t0;
    const Vector3f c1 = rRay1.getPos() + rRay1.getDir() * t1;

    if (pT0)
    {
        *pT0 = t0;
    }

    if (pT1)
    {
        *pT1 = t1;
    }

    return (c0 - c1).squaredLength();
}

/**
 * Calculates the squared distance between a segment and a segment.
 * @param rSegment0 First segment.
 * @param rSegment1 Second segment.
 * @param pT0 Receives the parameter of the closest point on the first segment; may be null.
 * @param pT1 Receives the parameter of the closest point on the second segment; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceSegmentToSegment(const Segment<Vector2f>& rSegment0,
                                                  const Segment<Vector2f>& rSegment1, f32* pT0,
                                                  f32* pT1)
{
    return calcSquaredDistanceSegmentToSegment_(rSegment0, rSegment1, pT0, pT1);
}

/**
 * Calculates the squared distance between a segment and a segment.
 * @param rSegment0 First segment.
 * @param rSegment1 Second segment.
 * @param pT0 Receives the parameter of the closest point on the first segment; may be null.
 * @param pT1 Receives the parameter of the closest point on the second segment; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceSegmentToSegment(const Segment<Vector3f>& rSegment0,
                                                  const Segment<Vector3f>& rSegment1, f32* pT0,
                                                  f32* pT1)
{
    return calcSquaredDistanceSegmentToSegment_(rSegment0, rSegment1, pT0, pT1);
}

/**
 * Calculates the squared distance between a line and a segment.
 * @param rLine Line.
 * @param rSegment Segment.
 * @param pT0 Receives the parameter of the closest point on the line; may be null.
 * @param pT1 Receives the parameter of the closest point on the segment; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceLineToSegment(const Line<Vector2f>& rLine,
                                               const Segment<Vector2f>& rSegment, f32* pT0,
                                               f32* pT1)
{
    return calcSquaredDistanceLineToSegment_(rLine, rSegment, pT0, pT1);
}

/**
 * Calculates the squared distance between a line and a segment.
 * @param rLine Line.
 * @param rSegment Segment.
 * @param pT0 Receives the parameter of the closest point on the line; may be null.
 * @param pT1 Receives the parameter of the closest point on the segment; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceLineToSegment(const Line<Vector3f>& rLine,
                                               const Segment<Vector3f>& rSegment, f32* pT0,
                                               f32* pT1)
{
    return calcSquaredDistanceLineToSegment_(rLine, rSegment, pT0, pT1);
}

/**
 * Calculates the squared distance between a ray and a segment.
 * @param rRay Ray.
 * @param rSegment Segment.
 * @param pT0 Receives the parameter of the closest point on the ray; may be null.
 * @param pT1 Receives the parameter of the closest point on the segment; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceRayToSegment(const Ray<Vector2f>& rRay,
                                              const Segment<Vector2f>& rSegment, f32* pT0, f32* pT1)
{
    return calcSquaredDistanceRayToSegment_(rRay, rSegment, pT0, pT1);
}

/**
 * Calculates the squared distance between a ray and a segment.
 * @param rRay Ray.
 * @param rSegment Segment.
 * @param pT0 Receives the parameter of the closest point on the ray; may be null.
 * @param pT1 Receives the parameter of the closest point on the segment; may be null.
 * @return Squared distance.
 */
f32 Geometry::calcSquaredDistanceRayToSegment(const Ray<Vector3f>& rRay,
                                              const Segment<Vector3f>& rSegment, f32* pT0, f32* pT1)
{
    return calcSquaredDistanceRayToSegment_(rRay, rSegment, pT0, pT1);
}

/**
 * Calculates where a line crosses a plane.
 * @param rLine Line.
 * @param rPlane Plane.
 * @param pT Receives the parameter of the intersection on the line; may be null.
 * @return 1 for a single intersection, 3 if the line lies in the plane, 0 otherwise.
 */
s32 Geometry::calcIntersectionLineToPlane(const Line<Vector2f>& rLine, const Plane2<f32>& rPlane,
                                          f32* pT)
{
    const f32 denom = rLine.getDir().dot(rPlane.getNormal());

    if (isNearlyZero(denom))
    {
        const f32 distance = rPlane.getNormal().dot(rLine.getPos()) - rPlane.getD();
        return isNearlyZero(distance) ? 3 : 0;
    }

    if (pT)
    {
        *pT = -(rPlane.getNormal().dot(rLine.getPos()) - rPlane.getD()) / denom;
    }

    return 1;
}

/**
 * Calculates where a line crosses a plane.
 * @param rLine Line.
 * @param rPlane Plane.
 * @param pT Receives the parameter of the intersection on the line; may be null.
 * @return 1 for a single intersection, 3 if the line lies in the plane, 0 otherwise.
 */
s32 Geometry::calcIntersectionLineToPlane(const Line<Vector3f>& rLine, const Plane3<f32>& rPlane,
                                          f32* pT)
{
    const f32 denom = rLine.getDir().dot(rPlane.getNormal());

    if (isNearlyZero(denom))
    {
        const f32 distance = rPlane.getNormal().dot(rLine.getPos()) - rPlane.getD();
        return isNearlyZero(distance) ? 3 : 0;
    }

    if (pT)
    {
        *pT = -(rPlane.getNormal().dot(rLine.getPos()) - rPlane.getD()) / denom;
    }

    return 1;
}

/**
 * Calculates where a ray crosses a plane.
 * @param rRay Ray.
 * @param rPlane Plane.
 * @param pT Receives the parameter of the intersection on the ray; may be null.
 * @return 1 for a single intersection, 3 if the ray lies in the plane, 0 otherwise.
 */
s32 Geometry::calcIntersectionRayToPlane(const Ray<Vector2f>& rRay, const Plane2<f32>& rPlane,
                                         f32* pT)
{
    const Plane2<f32> plane = rPlane;
    const Ray<Vector2f> ray = rRay;
    const f32 denom = ray.getDir().dot(plane.getNormal());
    const f32 distance = ray.getPos().dot(plane.getNormal()) - plane.getD();

    if (isNearlyZero(denom))
    {
        return isNearlyZero(distance) ? 3 : 0;
    }

    const f32 t = -distance / denom;

    if (!(t >= 0.0f))
    {
        return 0;
    }

    if (pT)
    {
        *pT = t;
    }

    return 1;
}

/**
 * Calculates where a ray crosses a plane.
 * @param rRay Ray.
 * @param rPlane Plane.
 * @param pT Receives the parameter of the intersection on the ray; may be null.
 * @return 1 for a single intersection, 3 if the ray lies in the plane, 0 otherwise.
 */
s32 Geometry::calcIntersectionRayToPlane(const Ray<Vector3f>& rRay, const Plane3<f32>& rPlane,
                                         f32* pT)
{
    const Ray<Vector3f> ray = rRay;
    const f32 denom = ray.getDir().dot(rPlane.getNormal());
    const f32 distance = ray.getPos().dot(rPlane.getNormal()) - rPlane.getD();

    if (isNearlyZero(denom))
    {
        return isNearlyZero(distance) ? 3 : 0;
    }

    const f32 t = -distance / denom;

    if (!(t >= 0.0f))
    {
        return 0;
    }

    if (pT)
    {
        *pT = t;
    }

    return 1;
}

/**
 * Calculates where a segment crosses a plane.
 * @param rSegment Segment.
 * @param rPlane Plane.
 * @param pT Receives the parameter of the intersection on the segment; may be null.
 * @return 1 for a single intersection, 3 if the segment lies in the plane, 0 otherwise.
 */
s32 Geometry::calcIntersectionSegmentToPlane(const Segment<Vector2f>& rSegment,
                                             const Plane2<f32>& rPlane, f32* pT)
{
    const Vector2f dir = rSegment.getPos1() - rSegment.getPos0();
    const f32 distance = rSegment.getPos0().dot(rPlane.getNormal()) - rPlane.getD();
    const f32 denom = dir.dot(rPlane.getNormal());

    if (denom == 0.0f)
    {
        if (distance == 0.0f)
        {
            if (pT)
            {
                *pT = 0.0f;
            }

            return 3;
        }

        return 0;
    }

    const f32 t = -distance / denom;

    if (t >= 0.0f && t <= 1.0f)
    {
        if (pT)
        {
            *pT = t;
        }

        return 1;
    }

    return 0;
}

/**
 * Calculates where a segment crosses a plane.
 * @param rSegment Segment.
 * @param rPlane Plane.
 * @param pT Receives the parameter of the intersection on the segment; may be null.
 * @return 1 for a single intersection, 3 if the segment lies in the plane, 0 otherwise.
 */
s32 Geometry::calcIntersectionSegmentToPlane(const Segment<Vector3f>& rSegment,
                                             const Plane3<f32>& rPlane, f32* pT)
{
    const Vector3f dir = rSegment.getPos1() - rSegment.getPos0();
    const f32 distance = rSegment.getPos0().dot(rPlane.getNormal()) - rPlane.getD();
    const f32 denom = dir.dot(rPlane.getNormal());

    if (denom == 0.0f)
    {
        if (distance == 0.0f)
        {
            if (pT)
            {
                *pT = 0.0f;
            }

            return 3;
        }

        return 0;
    }

    const f32 t = -distance / denom;

    if (t >= 0.0f && t <= 1.0f)
    {
        if (pT)
        {
            *pT = t;
        }

        return 1;
    }

    return 0;
}

/**
 * Calculates where a line crosses the surface of a sphere.
 * @param rLine Line.
 * @param rSphere Sphere.
 * @param pT0 Receives the parameter of the first intersection on the line; may be null.
 * @param pT1 Receives the parameter of the second intersection on the line; may be null.
 * @return Number of intersection points.
 */
s32 Geometry::calcIntersectionLineToSphere(const Line<Vector2f>& rLine,
                                           const Sphere<Vector2f>& rSphere, f32* pT0, f32* pT1)
{
    const Vector2f diff = rLine.getPos() - rSphere.getCenter();
    const f32 b = 2.0f * diff.dot(rLine.getDir());
    const f32 c = diff.squaredLength() - rSphere.getRadius() * rSphere.getRadius();
    const f32 discriminant = b * b + -4.0f * c;

    if (discriminant > 0.0f)
    {
        if (pT0 || pT1)
        {
            const f32 root = Mathf::sqrt(discriminant);

            if (pT0)
            {
                *pT0 = (-b - root) * 0.5f;
            }

            if (pT1)
            {
                *pT1 = (root - b) * 0.5f;
            }
        }

        return 2;
    }

    if (!isNearlyZero(discriminant))
    {
        return 0;
    }

    if (pT0)
    {
        *pT0 = b * -0.5f;
    }

    return 1;
}

/**
 * Calculates where a line crosses the surface of a sphere.
 * @param rLine Line.
 * @param rSphere Sphere.
 * @param pT0 Receives the parameter of the first intersection on the line; may be null.
 * @param pT1 Receives the parameter of the second intersection on the line; may be null.
 * @return Number of intersection points.
 */
s32 Geometry::calcIntersectionLineToSphere(const Line<Vector3f>& rLine,
                                           const Sphere<Vector3f>& rSphere, f32* pT0, f32* pT1)
{
    const Vector3f diff = rLine.getPos() - rSphere.getCenter();
    const f32 b = 2.0f * diff.dot(rLine.getDir());
    const f32 c = diff.squaredLength() - rSphere.getRadius() * rSphere.getRadius();
    const f32 discriminant = b * b + -4.0f * c;

    if (discriminant > 0.0f)
    {
        if (pT0 || pT1)
        {
            const f32 root = Mathf::sqrt(discriminant);

            if (pT0)
            {
                *pT0 = (-b - root) * 0.5f;
            }

            if (pT1)
            {
                *pT1 = (root - b) * 0.5f;
            }
        }

        return 2;
    }

    if (!isNearlyZero(discriminant))
    {
        return 0;
    }

    if (pT0)
    {
        *pT0 = b * -0.5f;
    }

    return 1;
}

/**
 * Calculates where a ray crosses the surface of a sphere.
 * @param rRay Ray.
 * @param rSphere Sphere.
 * @param pT0 Receives the parameter of the first intersection on the ray; may be null.
 * @param pT1 Receives the parameter of the second intersection on the ray; may be null.
 * @return Number of intersection points.
 */
s32 Geometry::calcIntersectionRayToSphere(const Ray<Vector2f>& rRay,
                                          const Sphere<Vector2f>& rSphere, f32* pT0, f32* pT1)
{
    const Vector2f diff = rRay.getPos() - rSphere.getCenter();
    const f32 b = 2.0f * diff.dot(rRay.getDir());
    const f32 c = diff.squaredLength() - rSphere.getRadius() * rSphere.getRadius();
    const f32 discriminant = b * b + -4.0f * c;

    if (discriminant > 0.0f)
    {
        if (b > 0.0f && b * b > discriminant)
        {
            return 0;
        }

        if (!(b > 0.0f) && !(b * b < discriminant))
        {
            if (pT0 || pT1)
            {
                const f32 root = Mathf::sqrt(discriminant);

                if (pT0)
                {
                    *pT0 = (-b - root) * 0.5f;
                }

                if (pT1)
                {
                    *pT1 = (root - b) * 0.5f;
                }
            }

            return 2;
        }

        if (pT0)
        {
            *pT0 = (Mathf::sqrt(discriminant) - b) * 0.5f;
        }

        return 1;
    }

    if (isNearlyZero(discriminant) && !(b > 0.0f))
    {
        if (pT0)
        {
            *pT0 = b * -0.5f;
        }

        return 1;
    }

    return 0;
}

/**
 * Calculates where a ray crosses the surface of a sphere.
 * @param rRay Ray.
 * @param rSphere Sphere.
 * @param pT0 Receives the parameter of the first intersection on the ray; may be null.
 * @param pT1 Receives the parameter of the second intersection on the ray; may be null.
 * @return Number of intersection points.
 */
s32 Geometry::calcIntersectionRayToSphere(const Ray<Vector3f>& rRay,
                                          const Sphere<Vector3f>& rSphere, f32* pT0, f32* pT1)
{
    const Vector3f diff = rRay.getPos() - rSphere.getCenter();
    const f32 b = 2.0f * diff.dot(rRay.getDir());
    const f32 c = diff.squaredLength() - rSphere.getRadius() * rSphere.getRadius();
    const f32 discriminant = b * b + -4.0f * c;

    if (discriminant > 0.0f)
    {
        if (b > 0.0f && b * b > discriminant)
        {
            return 0;
        }

        if (!(b > 0.0f) && !(b * b < discriminant))
        {
            if (pT0 || pT1)
            {
                const f32 root = Mathf::sqrt(discriminant);

                if (pT0)
                {
                    *pT0 = (-b - root) * 0.5f;
                }

                if (pT1)
                {
                    *pT1 = (root - b) * 0.5f;
                }
            }

            return 2;
        }

        if (pT0)
        {
            *pT0 = (Mathf::sqrt(discriminant) - b) * 0.5f;
        }

        return 1;
    }

    if (isNearlyZero(discriminant) && !(b > 0.0f))
    {
        if (pT0)
        {
            *pT0 = b * -0.5f;
        }

        return 1;
    }

    return 0;
}

// NON_MATCHING: bEnd * bEnd is hoisted and one fccmp condition differs
template <typename T>
static s32 calcIntersectionSegmentToSphere_(const Segment<T>& rSegment, const Sphere<T>& rSphere,
                                            f32* pT0, f32* pT1)
{
    T dir = rSegment.getPos1() - rSegment.getPos0();
    const f32 length = Mathf::sqrt(dir.squaredLength());

    if (length > 0.0f)
    {
        dir *= 1.0f / length;
    }

    const T diff = rSegment.getPos0() - rSphere.getCenter();

    if (length <= 0.0f)
    {
        const f32 c = diff.squaredLength() - rSphere.getRadius() * rSphere.getRadius();

        if (!isNearlyZero(c))
        {
            return 0;
        }

        if (pT0)
        {
            *pT0 = 0.0f;
        }

        if (pT1)
        {
            *pT1 = 1.0f;
        }

        return 2;
    }

    const f32 b = 2.0f * dir.dot(diff);
    const f32 c = diff.squaredLength() - rSphere.getRadius() * rSphere.getRadius();
    const f32 discriminant = b * b + -4.0f * c;

    if (discriminant > 0.0f)
    {
        if (b > 0.0f && b * b > discriminant)
        {
            return 0;
        }

        const f32 bEnd = length + length + b;

        if (!(b > 0.0f) && !(b * b < discriminant))
        {
            if (bEnd >= 0.0f && bEnd * bEnd >= discriminant)
            {
                if (pT0 || pT1)
                {
                    const f32 root = Mathf::sqrt(discriminant);

                    if (pT0)
                    {
                        *pT0 = (-b - root) * 0.5f / length;
                    }

                    if (pT1)
                    {
                        *pT1 = (root - b) * 0.5f / length;
                    }
                }

                return 2;
            }

            if (bEnd < 0.0f && discriminant < bEnd * bEnd)
            {
                return 0;
            }

            if (pT0)
            {
                *pT0 = (-b - Mathf::sqrt(discriminant)) * 0.5f / length;
            }

            return 1;
        }

        if (bEnd < 0.0f || bEnd * bEnd < discriminant)
        {
            return 0;
        }

        if (pT0)
        {
            *pT0 = (Mathf::sqrt(discriminant) - b) * 0.5f / length;
        }

        return 1;
    }

    if (!isNearlyZero(discriminant))
    {
        return 0;
    }

    if (b > 0.0f && b * b > discriminant)
    {
        return 0;
    }

    const f32 bEnd = length + length + b;

    if (bEnd < 0.0f && discriminant < bEnd * bEnd)
    {
        return 0;
    }

    if (pT0)
    {
        *pT0 = b * -0.5f / length;
    }

    return 1;
}

s32 Geometry::calcIntersectionSegmentToSphere(const Segment<Vector2f>& rSegment,
                                              const Sphere<Vector2f>& rSphere, f32* pT0, f32* pT1)
{
    return calcIntersectionSegmentToSphere_(rSegment, rSphere, pT0, pT1);
}

s32 Geometry::calcIntersectionSegmentToSphere(const Segment<Vector3f>& rSegment,
                                              const Sphere<Vector3f>& rSphere, f32* pT0, f32* pT1)
{
    return calcIntersectionSegmentToSphere_(rSegment, rSphere, pT0, pT1);
}

/**
 * Checks whether a line intersects an axis-aligned box.
 * @param rLine Line.
 * @param rBox Axis-aligned box.
 * @param pT0 Receives the parameter where the line enters the box; may be null.
 * @param pT1 Receives the parameter where the line leaves the box; may be null.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionLineToAABB(const Line<Vector2f>& rLine, const BoundBox2<f32>& rBox,
                                          f32* pT0, f32* pT1)
{
    return calcIntersectionLineToAABB_(rLine.getPos(), rLine.getDir(), rBox.getMin(), rBox.getMax(),
                                       pT0, pT1, -MathCalcCommon<f32>::maxNumber(),
                                       MathCalcCommon<f32>::maxNumber());
}

/**
 * Checks whether a line intersects an axis-aligned box.
 * @param rLine Line.
 * @param rBox Axis-aligned box.
 * @param pT0 Receives the parameter where the line enters the box; may be null.
 * @param pT1 Receives the parameter where the line leaves the box; may be null.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionLineToAABB(const Line<Vector3f>& rLine, const BoundBox3<f32>& rBox,
                                          f32* pT0, f32* pT1)
{
    return calcIntersectionLineToAABB_(rLine.getPos(), rLine.getDir(), rBox.getMin(), rBox.getMax(),
                                       pT0, pT1, -MathCalcCommon<f32>::maxNumber(),
                                       MathCalcCommon<f32>::maxNumber());
}

/**
 * Checks whether a ray intersects an axis-aligned box.
 * @param rRay Ray.
 * @param rBox Axis-aligned box.
 * @param pT0 Receives the parameter where the ray enters the box; may be null.
 * @param pT1 Receives the parameter where the ray leaves the box; may be null.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionRayToAABB(const Ray<Vector2f>& rRay, const BoundBox2<f32>& rBox,
                                         f32* pT0, f32* pT1)
{
    return calcIntersectionLineToAABB_(rRay.getPos(), rRay.getDir(), rBox.getMin(), rBox.getMax(),
                                       pT0, pT1, 0.0f, MathCalcCommon<f32>::maxNumber());
}

/**
 * Checks whether a ray intersects an axis-aligned box.
 * @param rRay Ray.
 * @param rBox Axis-aligned box.
 * @param pT0 Receives the parameter where the ray enters the box; may be null.
 * @param pT1 Receives the parameter where the ray leaves the box; may be null.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionRayToAABB(const Ray<Vector3f>& rRay, const BoundBox3<f32>& rBox,
                                         f32* pT0, f32* pT1)
{
    return calcIntersectionLineToAABB_(rRay.getPos(), rRay.getDir(), rBox.getMin(), rBox.getMax(),
                                       pT0, pT1, 0.0f, MathCalcCommon<f32>::maxNumber());
}

/**
 * Checks whether a segment intersects an axis-aligned box.
 * @param rSegment Segment.
 * @param rBox Axis-aligned box.
 * @param pT0 Receives the parameter where the segment enters the box; may be null.
 * @param pT1 Receives the parameter where the segment leaves the box; may be null.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionSegmentToAABB(const Segment<Vector2f>& rSegment,
                                             const BoundBox2<f32>& rBox, f32* pT0, f32* pT1)
{
    const Vector2f dir = rSegment.getPos1() - rSegment.getPos0();
    return calcIntersectionLineToAABB_(rSegment.getPos0(), dir, rBox.getMin(), rBox.getMax(), pT0,
                                       pT1, 0.0f, 1.0f);
}

/**
 * Checks whether a segment intersects an axis-aligned box.
 * @param rSegment Segment.
 * @param rBox Axis-aligned box.
 * @param pT0 Receives the parameter where the segment enters the box; may be null.
 * @param pT1 Receives the parameter where the segment leaves the box; may be null.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionSegmentToAABB(const Segment<Vector3f>& rSegment,
                                             const BoundBox3<f32>& rBox, f32* pT0, f32* pT1)
{
    const Vector3f dir = rSegment.getPos1() - rSegment.getPos0();
    return calcIntersectionLineToAABB_(rSegment.getPos0(), dir, rBox.getMin(), rBox.getMax(), pT0,
                                       pT1, 0.0f, 1.0f);
}

/**
 * Checks whether a sphere intersects an axis-aligned box.
 * @param rSphere Sphere.
 * @param rBox Axis-aligned box.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionSphereToAABB(const Sphere<Vector2f>& rSphere,
                                            const BoundBox2<f32>& rBox)
{
    const BoundBox2<f32> box = rBox;
    const f32 distance = calcSquaredDistancePointToAABB(rSphere.getCenter(), box, nullptr);
    return distance <= rSphere.getRadius() * rSphere.getRadius();
}

/**
 * Checks whether a sphere intersects an axis-aligned box.
 * @param rSphere Sphere.
 * @param rBox Axis-aligned box.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionSphereToAABB(const Sphere<Vector3f>& rSphere,
                                            const BoundBox3<f32>& rBox)
{
    const BoundBox3<f32> box = rBox;
    const f32 distance = calcSquaredDistancePointToAABB(rSphere.getCenter(), box, nullptr);
    return distance <= rSphere.getRadius() * rSphere.getRadius();
}

/**
 * Checks whether two spheres intersect.
 * @param rSphere0 First sphere.
 * @param rSphere1 Second sphere.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionSphereToSphere(const Sphere<Vector2f>& rSphere0,
                                              const Sphere<Vector2f>& rSphere1)
{
    const f32 squaredDistance = (rSphere0.getCenter() - rSphere1.getCenter()).squaredLength();
    const f32 radius = rSphere0.getRadius() + rSphere1.getRadius();
    return squaredDistance <= radius * radius;
}

/**
 * Checks whether two spheres intersect.
 * @param rSphere0 First sphere.
 * @param rSphere1 Second sphere.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionSphereToSphere(const Sphere<Vector3f>& rSphere0,
                                              const Sphere<Vector3f>& rSphere1)
{
    const f32 squaredDistance = (rSphere0.getCenter() - rSphere1.getCenter()).squaredLength();
    const f32 radius = rSphere0.getRadius() + rSphere1.getRadius();
    return squaredDistance <= radius * radius;
}

// NON_MATCHING: operand order of one fmul
bool Geometry::calcIntersectionPlaneToAABB(const Plane2<f32>& rPlane, const BoundBox2<f32>& rBox)
{
    const Vector2f center(rBox.getMin().x + rBox.getHalfSizeX(),
                          rBox.getMin().y + rBox.getHalfSizeY());
    const Vector2f extent = rBox.getMax() - center;
    f32 radius = 0.0f;

    for (s32 i = 0; i < 2; i++)
    {
        radius += extent.e[i] * Mathf::abs(rPlane.getNormal().e[i]);
    }

    const f32 distance = rPlane.getNormal().dot(center) - rPlane.getD();
    return Mathf::abs(distance) <= radius;
}

// NON_MATCHING: scheduling
bool Geometry::calcIntersectionPlaneToAABB(const Plane3<f32>& rPlane, const BoundBox3<f32>& rBox)
{
    Vector3f center = rBox.getMin();
    center.x += rBox.getHalfSizeX();
    center.y += rBox.getHalfSizeY();
    center.z += rBox.getHalfSizeZ();
    const Vector3f extent = rBox.getMax() - center;
    f32 radius = 0.0f;

    for (s32 i = 0; i < 3; i++)
    {
        radius += extent.e[i] * Mathf::abs(rPlane.getNormal().e[i]);
    }

    const f32 distance = rPlane.getNormal().dot(center) - rPlane.getD();
    return Mathf::abs(distance) <= radius;
}

/**
 * Checks whether two capsules intersect.
 * @param rCapsule0 First capsule.
 * @param rCapsule1 Second capsule.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionCapsuleToCapsule(const Capsule<Vector2f>& rCapsule0,
                                                const Capsule<Vector2f>& rCapsule1)
{
    const f32 distance = calcSquaredDistanceSegmentToSegment_(
        rCapsule0.getSegment(), rCapsule1.getSegment(), nullptr, nullptr);
    const f32 radius = rCapsule0.getRadius() + rCapsule1.getRadius();
    return distance <= radius * radius;
}

/**
 * Checks whether two capsules intersect.
 * @param rCapsule0 First capsule.
 * @param rCapsule1 Second capsule.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionCapsuleToCapsule(const Capsule<Vector3f>& rCapsule0,
                                                const Capsule<Vector3f>& rCapsule1)
{
    const f32 distance = calcSquaredDistanceSegmentToSegment_(
        rCapsule0.getSegment(), rCapsule1.getSegment(), nullptr, nullptr);
    const f32 radius = rCapsule0.getRadius() + rCapsule1.getRadius();
    return distance <= radius * radius;
}

/**
 * Checks whether a line intersects a capsule.
 * @param rLine Line.
 * @param rCapsule Capsule.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionLineToCapsule(const Line<Vector2f>& rLine,
                                             const Capsule<Vector2f>& rCapsule)
{
    const f32 distance =
        calcSquaredDistanceLineToSegment_(rLine, rCapsule.getSegment(), nullptr, nullptr);
    return distance <= rCapsule.getRadius() * rCapsule.getRadius();
}

/**
 * Checks whether a line intersects a capsule.
 * @param rLine Line.
 * @param rCapsule Capsule.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionLineToCapsule(const Line<Vector3f>& rLine,
                                             const Capsule<Vector3f>& rCapsule)
{
    const f32 distance =
        calcSquaredDistanceLineToSegment_(rLine, rCapsule.getSegment(), nullptr, nullptr);
    return distance <= rCapsule.getRadius() * rCapsule.getRadius();
}

/**
 * Checks whether a ray intersects a capsule.
 * @param rRay Ray.
 * @param rCapsule Capsule.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionRayToCapsule(const Ray<Vector2f>& rRay,
                                            const Capsule<Vector2f>& rCapsule)
{
    const f32 distance =
        calcSquaredDistanceRayToSegment_(rRay, rCapsule.getSegment(), nullptr, nullptr);
    return distance <= rCapsule.getRadius() * rCapsule.getRadius();
}

/**
 * Checks whether a ray intersects a capsule.
 * @param rRay Ray.
 * @param rCapsule Capsule.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionRayToCapsule(const Ray<Vector3f>& rRay,
                                            const Capsule<Vector3f>& rCapsule)
{
    const f32 distance =
        calcSquaredDistanceRayToSegment_(rRay, rCapsule.getSegment(), nullptr, nullptr);
    return distance <= rCapsule.getRadius() * rCapsule.getRadius();
}

/**
 * Checks whether a segment intersects a capsule.
 * @param rSegment Segment.
 * @param rCapsule Capsule.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionSegmentToCapsule(const Segment<Vector2f>& rSegment,
                                                const Capsule<Vector2f>& rCapsule)
{
    const f32 distance =
        calcSquaredDistanceSegmentToSegment_(rSegment, rCapsule.getSegment(), nullptr, nullptr);
    return distance <= rCapsule.getRadius() * rCapsule.getRadius();
}

/**
 * Checks whether a segment intersects a capsule.
 * @param rSegment Segment.
 * @param rCapsule Capsule.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionSegmentToCapsule(const Segment<Vector3f>& rSegment,
                                                const Capsule<Vector3f>& rCapsule)
{
    const f32 distance =
        calcSquaredDistanceSegmentToSegment_(rSegment, rCapsule.getSegment(), nullptr, nullptr);
    return distance <= rCapsule.getRadius() * rCapsule.getRadius();
}

/**
 * Checks whether a plane intersects a capsule.
 * @param rPlane Plane.
 * @param rCapsule Capsule.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionPlaneToCapsule(const Plane2<f32>& rPlane,
                                              const Capsule<Vector2f>& rCapsule)
{
    const f32 distance0 = rPlane.getNormal().dot(rCapsule.getSegment().getPos0()) - rPlane.getD();
    const f32 distance1 = rPlane.getNormal().dot(rCapsule.getSegment().getPos1()) - rPlane.getD();

    if (distance0 * distance1 < 0.0f)
    {
        return true;
    }

    return distance0 <= rCapsule.getRadius() || distance1 <= rCapsule.getRadius();
}

/**
 * Checks whether a plane intersects a capsule.
 * @param rPlane Plane.
 * @param rCapsule Capsule.
 * @return true if they intersect.
 */
bool Geometry::calcIntersectionPlaneToCapsule(const Plane3<f32>& rPlane,
                                              const Capsule<Vector3f>& rCapsule)
{
    const f32 distance0 = rPlane.getNormal().dot(rCapsule.getSegment().getPos0()) - rPlane.getD();
    const f32 distance1 = rPlane.getNormal().dot(rCapsule.getSegment().getPos1()) - rPlane.getD();

    if (distance0 * distance1 < 0.0f)
    {
        return true;
    }

    return distance0 <= rCapsule.getRadius() || distance1 <= rCapsule.getRadius();
}
}  // namespace sead
