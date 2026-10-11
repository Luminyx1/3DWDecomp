#include "NPC/NpcStateParam.hpp"

/** @brief Initializes the default gravity, velocity damping, and ground-check parameters. */
NpcStateParam::NpcStateParam()
    : mGravity(1.0f), mAirVelocityScale(0.99f), mGroundVelocityScale(0.93f),
      mParamC(150.0f), mGroundCheckFlags(0), mGroundCheckDistance(0.0f) {}

/**
 * @brief Stores the movement parameters retained by this game version.
 * @param gravity Acceleration applied along gravity while airborne.
 * @param airVelocityScale Velocity multiplier while airborne.
 * @param groundVelocityScale Velocity multiplier while grounded.
 * @param unused3 Unused constructor argument.
 * @param unused4 Unused constructor argument.
 * @param unused5 Unused constructor argument.
 * @param unused6 Unused constructor argument.
 * @param paramC Parameter stored at offset 0x0c; its purpose is not yet identified.
 * @param groundCheckFlags Flags passed to the ground-contact check.
 * @param groundCheckDistance Distance passed to the ground-contact check.
 */
NpcStateParam::NpcStateParam(float gravity, float airVelocityScale, float groundVelocityScale,
                           float unused3, float unused4, float unused5, float unused6,
                           float paramC, int groundCheckFlags, float groundCheckDistance)
    : mGravity(gravity), mAirVelocityScale(airVelocityScale),
      mGroundVelocityScale(groundVelocityScale), mParamC(paramC),
      mGroundCheckFlags(groundCheckFlags), mGroundCheckDistance(groundCheckDistance) {}
