#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class PipePackun;

/** @brief One segment of a Pipe Piranha Plant stem. Partial layout. */
class PipePackunBodyParts : public al::LiveActor {
public:
    PipePackunBodyParts(PipePackun* pHost, const char* pName);
    void killSwitch();
    f32 getLength() const;
    void setTotalLength(f32 length);
    void setTurnPattern(const PipePackunBodyParts* pPrevious);
    void calcPos(sead::Vector3f* pPos, f32 rate) const;
    void calcDir(sead::Vector3f* pDir, f32 rate) const;
    void setCoord(f32 coord);
    f32 calcNearCoord(f32* pDistance, const sead::Vector3f& rPos);
    /** @brief Gets the cumulative endpoint coordinate. @return Coordinate at the segment end. */
    f32 getTotalLength() const { return mTotalLength; }
private:
    u8 _144[0x20];
    f32 mTotalLength;
    u8 _168[0x10];
};
static_assert(sizeof(PipePackunBodyParts) == 0x178);
