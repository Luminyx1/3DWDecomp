#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class RabbitRouteRider;
// Layout prefix required by rabbit states; later fields are not yet reconstructed.
class Rabbit : public al::LiveActor {
public:
    bool canStartReverse(const al::LiveActor* pActor) const;
    float getMoveEndDistance() const;
    /** @brief Gets the route rider. @return The rabbit's route rider. */
    RabbitRouteRider* getRouteRider() const { return mRouteRider; }

private:
    unsigned char mUnknown148[8];
    RabbitRouteRider* mRouteRider;
};
