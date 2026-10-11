#pragma once

class NpcStateParam {
public:
    NpcStateParam();
    NpcStateParam(float gravity, float airVelocityScale, float groundVelocityScale,
                  float unused3, float unused4, float unused5, float unused6,
                  float paramC, int groundCheckFlags, float groundCheckDistance);

    float mGravity;
    float mAirVelocityScale;
    float mGroundVelocityScale;
    float mParamC;
    int mGroundCheckFlags;
    float mGroundCheckDistance;
};
