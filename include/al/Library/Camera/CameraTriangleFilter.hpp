#pragma once

#include "Project/Collision/TriangleFilterBase.hpp"

namespace al {

class CameraTriangleFilter : public TriangleFilterBase {
public:
    CameraTriangleFilter() {}

    bool isInvalidTriangle(const Triangle& rTriangle) const override;
};

class CameraTriangleFilterOnlyCeiling : public CameraTriangleFilter {
public:
    CameraTriangleFilterOnlyCeiling() {}

    bool isInvalidTriangle(const Triangle& rTriangle) const override;
};

class SubjectiveCameraTriangleFilter : public TriangleFilterBase {
public:
    SubjectiveCameraTriangleFilter() {}

    bool isInvalidTriangle(const Triangle& rTriangle) const override;

private:
    bool mIsIgnoreThrough = false;
};

}  // namespace al
