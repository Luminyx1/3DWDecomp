#pragma once

class IUsePlayerHorizontalSpeedAverage {
public:
    virtual void resetHorizontalSpeedAverage() = 0;
    virtual float getHorizontalSpeedAverage() const = 0;
};
