#pragma once

#include "Library/Factory/Factory.hpp"

namespace al {
class CameraPoser_RS;
class CameraPoserEntrance_RS;

using CameraPoserCreatorFunction_RS = CameraPoser_RS* (*)(const char* pName);

class CameraPoserFactory_RS : public Factory<CameraPoserCreatorFunction_RS> {
public:
    CameraPoserFactory_RS(const char* pName);

    virtual CameraPoserEntrance_RS* createEntranceCameraPoser() const;

private:
    s32 _1c = 9;
};

}  // namespace al
