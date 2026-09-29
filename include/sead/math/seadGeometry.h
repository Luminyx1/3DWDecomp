#pragma once

#include <math/seadVector.h>

namespace sead
{
template <typename T>
class Ray
{
public:
    Ray() = default;
    Ray(const T& rPos, const T& rDir) : mPos(rPos), mDir(rDir) {}

    const T& getPos() const { return mPos; }
    const T& getDir() const { return mDir; }

    void setPos(const T& rPos) { mPos = rPos; }
    void setDir(const T& rDir) { mDir = rDir; }
    void setPosDir(const T& rPos, const T& rDir)
    {
        mPos = rPos;
        mDir = rDir;
    }

private:
    T mPos;
    T mDir;
};

}  // namespace sead
