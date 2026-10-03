#pragma once
#include <basis/seadTypes.h>
class TesselatorData {
  public:
    TesselatorData();
    void reset();
    void incrementLine();
    void setX(float x);
    void setXBounds(float minX, float maxX, float step);
    float getX() const;
    float getMinX() const;
    float getMaxX() const;

  private:
    struct Scratch {
        u8 mUnreconstructed[0x60d0]; // Tessellation records; element layout not yet reconstructed.
        int mLine;
    } mScratch;
    float mX;
    float mMinX;
    float mMaxX;
    float mStep;
};
static_assert(sizeof(TesselatorData) == 0x60e4);
