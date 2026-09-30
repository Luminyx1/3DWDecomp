#pragma once

#include "Library/Factory/Factory.hpp"

namespace al {
class AreaShape;

using AreaShapeCreatorFunction = AreaShape* (*)();

template <typename T>
AreaShape* createAreaShapeFunction() {
    return new T();
}

class AreaShapeFactory : public Factory<AreaShapeCreatorFunction> {
public:
    AreaShapeFactory(const char* pName);
};
}  // namespace al
