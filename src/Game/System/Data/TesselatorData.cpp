#include "System/Data/TesselatorData.hpp"
#include <cstring>

/**
 * @brief Clear tessellation scratch records without changing the X bounds.
 */
TesselatorData::TesselatorData() { std::memset(&mScratch, 0, sizeof(mScratch)); }

/**
 * @brief Clear tessellation scratch records without changing the X bounds.
 */
void TesselatorData::reset() { std::memset(&mScratch, 0, sizeof(mScratch)); }

/**
 * @brief Advance to the next tessellation line.
 */
void TesselatorData::incrementLine() { ++mScratch.mLine; }

/**
 * @brief Set the current tessellation X position.
 * @param x Current X coordinate.
 */
void TesselatorData::setX(float x) { mX = x; }

/**
 * @brief Set the tessellation X interval.
 * @param minX Lower X bound.
 * @param maxX Upper X bound.
 * @param step X sampling step.
 */
void TesselatorData::setXBounds(float minX, float maxX, float step) {
    mMinX = minX;
    mMaxX = maxX;
    mStep = step;
}

/**
 * @brief Read a tessellation X coordinate.
 * @return The stored X coordinate.
 */
float TesselatorData::getX() const { return mX; }

/**
 * @brief Read a tessellation X coordinate.
 * @return The stored X coordinate.
 */
float TesselatorData::getMinX() const { return mMinX; }

/**
 * @brief Read a tessellation X coordinate.
 * @return The stored X coordinate.
 */
float TesselatorData::getMaxX() const { return mMaxX; }
