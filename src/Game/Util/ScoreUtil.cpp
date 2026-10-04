#include "Util/ScoreUtil.hpp"

#include "Library/Actor/ComboCounter.hpp"
#include "Library/Item/ActorScoreKeeper.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Scene/ISceneObj.hpp"
#include "Library/Scene/SceneObjUtil.hpp"
#include "Util/DrcUtil.hpp"
#include "Util/ProjectMsgUtil.hpp"

/**
 * @brief Scene object that spawns score / 1-UP pop-ups and keeps the players' scores.
 */
class ScoreHolder : public al::ISceneObj {
public:
    void popUpScore(const al::LiveActor* pActor, al::HitSensor* pSensor, const char* pCategory,
                    s32 combo, const sead::Vector3f& rPos);
    void popUpScore(const al::LiveActor* pActor, al::HitSensor* pSensor, const char* pCategory,
                    s32 combo, const sead::Vector3f* pPos, const sead::Vector3f& rOffset);
    void popUpPlayerUp(const al::LiveActor* pActor, s32 num, const sead::Vector3f& rPos,
                       s32 type);
    s32 getScoreMaxLevel(const char* pCategory) const;
    s32 getPlayerScore(s32 playerNo) const;
};

namespace {
    constexpr s32 cSceneObjScoreHolder = 20;

    /**
     * @brief Gets the scene's score holder.
     * @param pHolder Scene object holder user.
     * @return The score holder scene object.
     */
    inline ScoreHolder* getScoreHolder(const al::IUseSceneObjHolder* pHolder) {
        return static_cast<ScoreHolder*>(al::getSceneObj(pHolder, cSceneObjScoreHolder));
    }

    /**
     * @brief Gets the default score category of an actor.
     * @param pActor Actor whose score keeper is queried.
     * @return The category name, or nullptr if the actor has no score keeper.
     */
    inline const char* getCategoryName(const al::LiveActor* pActor) {
        al::ActorScoreKeeper* keeper = pActor->getActorScoreKeeper();
        return keeper != nullptr ? keeper->getCategoryName() : nullptr;
    }

    /**
     * @brief Gets the score category an actor gives for the given factor.
     * @param pActor Actor whose score keeper is queried.
     * @param pFactor Name of the factor (reason) the score is given for.
     * @return The category name, or nullptr if there is none.
     */
    inline const char* tryGetCategoryName(const al::LiveActor* pActor, const char* pFactor) {
        al::ActorScoreKeeper* keeper = pActor->getActorScoreKeeper();
        return keeper != nullptr ? keeper->tryGetCategoryName(pFactor) : nullptr;
    }

    /**
     * @brief Checks whether an actor gives a score by default.
     * @param pActor Actor whose score keeper is queried.
     * @return true if the actor has a default score category.
     */
    inline bool isExistScoreCategory(const al::LiveActor* pActor) {
        al::ActorScoreKeeper* keeper = pActor->getActorScoreKeeper();
        return keeper != nullptr && keeper->getCategoryName() != nullptr;
    }

    void updateComboCount(const al::LiveActor* pActor, const al::SensorMsg* pMsg,
                          const char* pCategory);

    /**
     * @brief Gets the current combo count carried by an attack message.
     * @param pMsg Attack message that may carry a combo counter.
     * @return The combo count, or 0 if the message has no combo counter.
     */
    inline s32 getComboCount(const al::SensorMsg* pMsg) {
        al::ComboCounter* counter = rc::tryGetMsgComboCount(pMsg);
        return counter != nullptr ? counter->mCounter : 0;
    }
}  // namespace

namespace rc {
    /**
     * @brief Pops up the actor's default score above its position.
     * @param pActor Actor that gives the score.
     * @param pSensor Sensor of the player that gets the score.
     * @param offsetY Height above the actor's position to pop up at.
     * @param combo Combo count of the score.
     */
    void addScore(const al::LiveActor* pActor, al::HitSensor* pSensor, f32 offsetY, s32 combo) {
        al::ActorScoreKeeper* keeper = pActor->getActorScoreKeeper();
        if (keeper == nullptr) {
            return;
        }

        const char* category = keeper->getCategoryName();
        if (category == nullptr) {
            return;
        }

        getScoreHolder(pActor)->popUpScore(pActor, pSensor, category, combo,
                                           al::getTrans(pActor) +
                                               sead::Vector3f(0.0f, offsetY, 0.0f));
    }

    /**
     * @brief Pops up the actor's default score at the given position.
     * @param pActor Actor that gives the score.
     * @param pSensor Sensor of the player that gets the score.
     * @param rPos Position to pop up at.
     * @param combo Combo count of the score.
     */
    void addScore(const al::LiveActor* pActor, al::HitSensor* pSensor, const sead::Vector3f& rPos,
                  s32 combo) {
        al::ActorScoreKeeper* keeper = pActor->getActorScoreKeeper();
        if (keeper == nullptr) {
            return;
        }

        const char* category = keeper->getCategoryName();
        if (category == nullptr) {
            return;
        }

        getScoreHolder(pActor)->popUpScore(pActor, pSensor, category, combo, rPos);
    }

    /**
     * @brief Pops up the actor's default score at a position that follows another one.
     * @param pActor Actor that gives the score.
     * @param pSensor Sensor of the player that gets the score.
     * @param pPos Position the pop-up follows.
     * @param rOffset Offset from the followed position.
     * @param combo Combo count of the score.
     */
    void addScore(const al::LiveActor* pActor, al::HitSensor* pSensor, const sead::Vector3f* pPos,
                  const sead::Vector3f& rOffset, s32 combo) {
        al::ActorScoreKeeper* keeper = pActor->getActorScoreKeeper();
        if (keeper == nullptr) {
            return;
        }

        const char* category = keeper->getCategoryName();
        if (category == nullptr) {
            return;
        }

        getScoreHolder(pActor)->popUpScore(pActor, pSensor, category, combo, pPos, rOffset);
    }

    /**
     * @brief Pops up the actor's score for a factor above its position.
     * @param pActor Actor that gives the score.
     * @param pSensor Sensor of the player that gets the score.
     * @param pFactor Name of the factor (reason) the score is given for.
     * @param offsetY Height above the actor's position to pop up at.
     * @param combo Combo count of the score.
     */
    void addScoreByFactor(const al::LiveActor* pActor, al::HitSensor* pSensor,
                          const char* pFactor, f32 offsetY, s32 combo) {
        al::ActorScoreKeeper* keeper = pActor->getActorScoreKeeper();
        if (keeper == nullptr) {
            return;
        }

        const char* category = keeper->tryGetCategoryName(pFactor);
        if (category == nullptr) {
            return;
        }

        getScoreHolder(pActor)->popUpScore(pActor, pSensor, category, combo,
                                           al::getTrans(pActor) +
                                               sead::Vector3f(0.0f, offsetY, 0.0f));
    }

    /**
     * @brief Pops up the actor's score for a factor at the given position.
     * @param pActor Actor that gives the score.
     * @param pSensor Sensor of the player that gets the score.
     * @param pFactor Name of the factor (reason) the score is given for.
     * @param rPos Position to pop up at.
     * @param combo Combo count of the score.
     */
    void addScoreByFactor(const al::LiveActor* pActor, al::HitSensor* pSensor,
                          const char* pFactor, const sead::Vector3f& rPos, s32 combo) {
        al::ActorScoreKeeper* keeper = pActor->getActorScoreKeeper();
        if (keeper == nullptr) {
            return;
        }

        const char* category = keeper->tryGetCategoryName(pFactor);
        if (category == nullptr) {
            return;
        }

        getScoreHolder(pActor)->popUpScore(pActor, pSensor, category, combo, rPos);
    }

    /**
     * @brief Pops up the actor's score for a factor at a position that follows another one.
     * @param pActor Actor that gives the score.
     * @param pSensor Sensor of the player that gets the score.
     * @param pFactor Name of the factor (reason) the score is given for.
     * @param pPos Position the pop-up follows.
     * @param rOffset Offset from the followed position.
     * @param combo Combo count of the score.
     */
    void addScoreByFactor(const al::LiveActor* pActor, al::HitSensor* pSensor,
                          const char* pFactor, const sead::Vector3f* pPos,
                          const sead::Vector3f& rOffset, s32 combo) {
        al::ActorScoreKeeper* keeper = pActor->getActorScoreKeeper();
        if (keeper == nullptr) {
            return;
        }

        const char* category = keeper->tryGetCategoryName(pFactor);
        if (category == nullptr) {
            return;
        }

        getScoreHolder(pActor)->popUpScore(pActor, pSensor, category, combo, pPos, rOffset);
    }

    /**
     * @brief Pops up the actor's default score with the message's combo count above its
     * position, then advances the combo.
     * @param pActor Actor that gives the score.
     * @param pSensor Sensor of the player that gets the score.
     * @param pMsg Attack message that may carry a combo counter.
     * @param offsetY Height above the actor's position to pop up at.
     */
    void addScoreCombo(const al::LiveActor* pActor, al::HitSensor* pSensor,
                       const al::SensorMsg* pMsg, f32 offsetY) {
        addScore(pActor, pSensor, offsetY, getComboCount(pMsg));
        if (isExistScoreCategory(pActor)) {
            updateComboCount(pActor, pMsg, getCategoryName(pActor));
        }
    }

    /**
     * @brief Pops up the actor's default score with the message's combo count at the given
     * position, then advances the combo.
     * @param pActor Actor that gives the score.
     * @param pSensor Sensor of the player that gets the score.
     * @param pMsg Attack message that may carry a combo counter.
     * @param rPos Position to pop up at.
     */
    void addScoreCombo(const al::LiveActor* pActor, al::HitSensor* pSensor,
                       const al::SensorMsg* pMsg, const sead::Vector3f& rPos) {
        addScore(pActor, pSensor, rPos, getComboCount(pMsg));
        if (isExistScoreCategory(pActor)) {
            updateComboCount(pActor, pMsg, getCategoryName(pActor));
        }
    }

    /**
     * @brief Pops up the actor's default score with the message's combo count at a position
     * that follows another one, then advances the combo.
     * @param pActor Actor that gives the score.
     * @param pSensor Sensor of the player that gets the score.
     * @param pMsg Attack message that may carry a combo counter.
     * @param pPos Position the pop-up follows.
     * @param rOffset Offset from the followed position.
     */
    void addScoreCombo(const al::LiveActor* pActor, al::HitSensor* pSensor,
                       const al::SensorMsg* pMsg, const sead::Vector3f* pPos,
                       const sead::Vector3f& rOffset) {
        addScore(pActor, pSensor, pPos, rOffset, getComboCount(pMsg));
        if (isExistScoreCategory(pActor)) {
            updateComboCount(pActor, pMsg, getCategoryName(pActor));
        }
    }

    /**
     * @brief Pops up the actor's score for a factor with the message's combo count above its
     * position, then advances the combo.
     * @param pActor Actor that gives the score.
     * @param pSensor Sensor of the player that gets the score.
     * @param pFactor Name of the factor (reason) the score is given for.
     * @param pMsg Attack message that may carry a combo counter.
     * @param offsetY Height above the actor's position to pop up at.
     */
    void addScoreComboByFactor(const al::LiveActor* pActor, al::HitSensor* pSensor,
                               const char* pFactor, const al::SensorMsg* pMsg, f32 offsetY) {
        addScoreByFactor(pActor, pSensor, pFactor, offsetY, getComboCount(pMsg));
        updateComboCount(pActor, pMsg, tryGetCategoryName(pActor, pFactor));
    }
}  // namespace rc

namespace {
    /**
     * @brief Advances the combo counter carried by an attack message, resetting it once an
     * area attack (explosion, invincibility, ball) reaches the category's highest score level.
     * @param pActor Actor that receives the score.
     * @param pMsg Attack message that may carry a combo counter.
     * @param pCategory Score category of the actor.
     */
    void updateComboCount(const al::LiveActor* pActor, const al::SensorMsg* pMsg,
                          const char* pCategory) {
        al::ComboCounter* counter = rc::tryGetMsgComboCount(pMsg);
        if (counter == nullptr) {
            return;
        }

        counter->increment();
        if (al::isMsgExplosion(pMsg) || al::isMsgExplosionCollide(pMsg) ||
            al::isMsgPlayerInvincibleAttack(pMsg) || al::isMsgPlayerInvincibleTouch(pMsg) ||
            al::isMsgBallAttack(pMsg) || al::isMsgBallTrample(pMsg) ||
            al::isMsgBallRouteDokanAttack(pMsg)) {
            s32 maxLevel = getScoreHolder(pActor)->getScoreMaxLevel(pCategory);
            if (counter->mCounter >= maxLevel) {
                counter->mCounter = 0;
            }
        }
    }
}  // namespace

namespace rc {
    /**
     * @brief Pops up the actor's default score above its position for the player assisted
     * through a screen pointer.
     * @param pActor Actor that gives the score.
     * @param pPointer Screen pointer that hit the actor.
     * @param offsetY Height above the actor's position to pop up at.
     * @param combo Combo count of the score.
     */
    void addScore(const al::LiveActor* pActor, al::ScreenPointer* pPointer, f32 offsetY,
                  s32 combo) {
        addScore(pActor, DrcFunction::tryFindDrcPlayerSensor(pActor, pPointer), offsetY, combo);
    }

    /**
     * @brief Pops up the actor's default score at the given position for the player assisted
     * through a screen pointer.
     * @param pActor Actor that gives the score.
     * @param pPointer Screen pointer that hit the actor.
     * @param rPos Position to pop up at.
     * @param combo Combo count of the score.
     */
    void addScore(const al::LiveActor* pActor, al::ScreenPointer* pPointer,
                  const sead::Vector3f& rPos, s32 combo) {
        addScore(pActor, DrcFunction::tryFindDrcPlayerSensor(pActor, pPointer), rPos, combo);
    }

    /**
     * @brief Pops up the actor's score for a factor above its position for the player
     * assisted through a screen pointer.
     * @param pActor Actor that gives the score.
     * @param pPointer Screen pointer that hit the actor.
     * @param pFactor Name of the factor (reason) the score is given for.
     * @param offsetY Height above the actor's position to pop up at.
     * @param combo Combo count of the score.
     */
    void addScoreByFactor(const al::LiveActor* pActor, al::ScreenPointer* pPointer,
                          const char* pFactor, f32 offsetY, s32 combo) {
        addScoreByFactor(pActor, DrcFunction::tryFindDrcPlayerSensor(pActor, pPointer), pFactor,
                         offsetY, combo);
    }

    /**
     * @brief Pops up the actor's score for a factor at the given position for the player
     * assisted through a screen pointer.
     * @param pActor Actor that gives the score.
     * @param pPointer Screen pointer that hit the actor.
     * @param pFactor Name of the factor (reason) the score is given for.
     * @param rPos Position to pop up at.
     * @param combo Combo count of the score.
     */
    void addScoreByFactor(const al::LiveActor* pActor, al::ScreenPointer* pPointer,
                          const char* pFactor, const sead::Vector3f& rPos, s32 combo) {
        addScoreByFactor(pActor, DrcFunction::tryFindDrcPlayerSensor(pActor, pPointer), pFactor,
                         rPos, combo);
    }

    /**
     * @brief Pops up the actor's default combo score above its position for the player
     * assisted through a screen pointer.
     * @param pActor Actor that gives the score.
     * @param pPointer Screen pointer that hit the actor.
     * @param pMsg Attack message that may carry a combo counter.
     * @param offsetY Height above the actor's position to pop up at.
     */
    void addScoreCombo(const al::LiveActor* pActor, al::ScreenPointer* pPointer,
                       const al::SensorMsg* pMsg, f32 offsetY) {
        addScoreCombo(pActor, DrcFunction::tryFindDrcPlayerSensor(pActor, pPointer), pMsg,
                      offsetY);
    }

    /**
     * @brief Pops up the actor's default combo score at the given position for the player
     * assisted through a screen pointer.
     * @param pActor Actor that gives the score.
     * @param pPointer Screen pointer that hit the actor.
     * @param pMsg Attack message that may carry a combo counter.
     * @param rPos Position to pop up at.
     */
    void addScoreCombo(const al::LiveActor* pActor, al::ScreenPointer* pPointer,
                       const al::SensorMsg* pMsg, const sead::Vector3f& rPos) {
        addScoreCombo(pActor, DrcFunction::tryFindDrcPlayerSensor(pActor, pPointer), pMsg, rPos);
    }

    /**
     * @brief Pops up a score of a system category (not tied to the actor's score keeper) at
     * the actor's position.
     * @param pActor Actor at which the score pops up.
     * @param pSensor Sensor of the player that gets the score.
     * @param pCategory Score category name.
     * @param combo Combo count of the score.
     */
    void addScoreBySystem(const al::LiveActor* pActor, al::HitSensor* pSensor,
                          const char* pCategory, s32 combo) {
        getScoreHolder(pActor)->popUpScore(pActor, pSensor, pCategory, combo,
                                           al::getTrans(pActor));
    }

    /**
     * @brief Pops up a score of a system category (not tied to the actor's score keeper) at
     * the given position.
     * @param pActor Actor that gives the score.
     * @param pSensor Sensor of the player that gets the score.
     * @param pCategory Score category name.
     * @param rPos Position to pop up at.
     * @param combo Combo count of the score.
     */
    void addScoreBySystem(const al::LiveActor* pActor, al::HitSensor* pSensor,
                          const char* pCategory, const sead::Vector3f& rPos, s32 combo) {
        getScoreHolder(pActor)->popUpScore(pActor, pSensor, pCategory, combo, rPos);
    }
}  // namespace rc

namespace ScoreFunction {
    /**
     * @brief Pops up a 1-UP above the actor's position.
     * @param pActor Actor that gives the 1-UP.
     * @param offsetY Height above the actor's position to pop up at.
     */
    void popUpPlayerOneUp(const al::LiveActor* pActor, f32 offsetY) {
        getScoreHolder(pActor)->popUpPlayerUp(
            pActor, 1, al::getTrans(pActor) + sead::Vector3f(0.0f, offsetY, 0.0f), 0);
    }

    /**
     * @brief Pops up a number of extra lives above the actor's position.
     * @param pActor Actor that gives the lives.
     * @param num Number of lives.
     * @param offsetY Height above the actor's position to pop up at.
     */
    void popUpPlayerUp(const al::LiveActor* pActor, s32 num, f32 offsetY) {
        getScoreHolder(pActor)->popUpPlayerUp(
            pActor, num, al::getTrans(pActor) + sead::Vector3f(0.0f, offsetY, 0.0f), 0);
    }

    /**
     * @brief Pops up a 1-UP at the given position.
     * @param pActor Actor that gives the 1-UP.
     * @param rPos Position to pop up at.
     */
    void popUpPlayerOneUp(const al::LiveActor* pActor, const sead::Vector3f& rPos) {
        getScoreHolder(pActor)->popUpPlayerUp(pActor, 1, rPos, 0);
    }

    /**
     * @brief Pops up a number of extra lives at the given position.
     * @param pActor Actor that gives the lives.
     * @param num Number of lives.
     * @param rPos Position to pop up at.
     */
    void popUpPlayerUp(const al::LiveActor* pActor, s32 num, const sead::Vector3f& rPos) {
        getScoreHolder(pActor)->popUpPlayerUp(pActor, num, rPos, 0);
    }

    /**
     * @brief Gets a player's score.
     * @param pHolder Scene object holder user.
     * @param playerNo Index of the player.
     * @return The player's score.
     */
    s32 getPlayerScore(const al::IUseSceneObjHolder* pHolder, s32 playerNo) {
        return getScoreHolder(pHolder)->getPlayerScore(playerNo);
    }
}  // namespace ScoreFunction
