#pragma once

#include "Library/Factory/Factory.hpp"

namespace al {
    class AreaObj;

    typedef AreaObj* (*AreaCreatorFunction)(const char*);

    /// Factory that creates area objects from their class name.
    class AreaObjFactory : public Factory<AreaCreatorFunction> {
    public:
        AreaObjFactory(const char* pName);
    };
};
