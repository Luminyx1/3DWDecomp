#pragma once

namespace al {
class Triangle;

class TriangleFilterBase {
public:
    virtual bool isInvalidTriangle(const Triangle& rTriangle) const = 0;
};

}  // namespace al
