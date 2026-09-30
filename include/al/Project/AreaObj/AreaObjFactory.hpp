#pragma once

#include "Library/Factory/Factory.hpp"

namespace al {
class AreaObj;

using AreaCreatorFunction = AreaObj* (*)(const char*);

class AreaObjFactory : public Factory<AreaCreatorFunction> {
public:
    AreaObjFactory(const char* pName);
};
}  // namespace al
