#pragma once

#include "Library/Factory/Factory.hpp"

namespace al {
class CameraPoser_RS;

typedef CameraPoser_RS* (*CameraPoserCreatorFunction)(const char*);

/// Creates camera posers by their class name.
class CameraPoserFactory_RS : public Factory<CameraPoserCreatorFunction> {
public:
    CameraPoserFactory_RS(const char* pName);

    virtual CameraPoser_RS* createEntranceCameraPoser() const;

    s32 _1C = 9;  // _1C
};
}  // namespace al
