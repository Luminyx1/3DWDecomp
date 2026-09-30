#pragma once

#include <math/seadVector.h>

#include "Library/HostIO/IUseHioNode.hpp"

namespace al {
class ByamlIter;
class CameraOffsetPreset;

class CameraOffsetCtrl : public HioNode {
public:
    virtual void load(const ByamlIter& rIter) = 0;
    virtual const sead::Vector3f& getOffset() const = 0;
};

class CameraOffsetCtrlPreset : public CameraOffsetCtrl {
public:
    CameraOffsetCtrlPreset();

    void load(const ByamlIter& rIter) override;
    const sead::Vector3f& getOffset() const override;

private:
    CameraOffsetPreset* mPreset = nullptr;
};

class CameraOffsetCtrlY : public CameraOffsetCtrl {
public:
    void load(const ByamlIter& rIter) override;

    const sead::Vector3f& getOffset() const override { return mOffset; }

private:
    sead::Vector3f mOffset = sead::Vector3f::zero;
};

}  // namespace al
