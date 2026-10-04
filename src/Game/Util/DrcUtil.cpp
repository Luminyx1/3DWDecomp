#include "Util/DrcUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "MapObj/DrcAssistDirectorUtil.hpp"

namespace DrcFunction {
    /**
     * @brief Finds the body sensor of the player assisted through the given screen pointer.
     * @param pActor Actor used to access the scene object holder.
     * @param pPointer Screen pointer (touch / gyro cursor) to look up.
     * @return The player's "Body" sensor, or nullptr if no player is assisted by the pointer.
     */
    al::HitSensor* tryFindDrcPlayerSensor(const al::LiveActor* pActor,
                                          const al::ScreenPointer* pPointer) {
        al::LiveActor* player = rc::findDrcAssistDirectorPlayer(pActor, pPointer);
        if (player == nullptr) {
            return nullptr;
        }

        return al::getHitSensor(player, "Body");
    }

    /**
     * @brief Finds the body sensor of the player assisted through the given hit sensor.
     * @param pActor Actor used to access the scene object holder.
     * @param pSensor Hit sensor of the assist director to look up.
     * @return The player's "Body" sensor, or nullptr if no player is found.
     */
    al::HitSensor* tryFindDrcPlayerSensor(const al::LiveActor* pActor,
                                          const al::HitSensor* pSensor) {
        al::LiveActor* player = rc::findDrcAssistDirectorPlayer(pActor, pSensor);
        if (player == nullptr) {
            return nullptr;
        }

        return al::getHitSensor(player, "Body");
    }

    /**
     * @brief Finds the actor currently touched by the given screen pointer.
     * @param pActor Actor used to access the scene object holder.
     * @param pPointer Screen pointer to look up.
     * @return The touched actor, or nullptr if none.
     */
    al::LiveActor* tryFindDrcTouchActor(const al::LiveActor* pActor,
                                        const al::ScreenPointer* pPointer) {
        return rc::findDrcAssistDirectorTouchPointer(pActor, pPointer);
    }
};  // namespace DrcFunction
