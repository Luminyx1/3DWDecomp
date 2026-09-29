#include "Project/Actor/ActorParamHolder.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
    /**
     * @brief Finds an integer parameter by name.
     * @param pName The name of the parameter.
     * @return A pointer to the parameter's value, or to a zero value if it doesn't exist.
     */
    const ActorParamS32* ActorParamHolder::findParamS32(const char* pName) const {
        ActorParamInfo* info = tryFindParamInfoByName(pName);
        return info != nullptr ? &info->mS32 : reinterpret_cast<const ActorParamS32*>("");
    }

    /**
     * @brief Searches for a parameter by name.
     * @param pName The name of the parameter.
     * @return The parameter, or nullptr if it doesn't exist.
     */
    ActorParamInfo* ActorParamHolder::tryFindParamInfoByName(const char* pName) const {
        for (s32 i = 0; i < mNumParams; i++) {
            ActorParamInfo* info = &mParams[i];
            if (isEqualString(info->mName, pName)) {
                return info;
            }
        }

        return nullptr;
    }

    /**
     * @brief Finds a float parameter by name.
     * @param pName The name of the parameter.
     * @return A pointer to the parameter's value, or to a zero value if it doesn't exist.
     */
    const ActorParamF32* ActorParamHolder::findParamF32(const char* pName) const {
        ActorParamInfo* info = tryFindParamInfoByName(pName);
        return info != nullptr ? &info->mF32 : reinterpret_cast<const ActorParamF32*>("");
    }

    /**
     * @brief Finds a movement parameter by name.
     * @param pName The name of the parameter.
     * @return The movement parameters, or zeroed parameters if it doesn't exist.
     */
    const ActorParamMove* ActorParamHolder::findParamMove(const char* pName) const {
        ActorParamInfo* info = tryFindParamInfoByName(pName);
        if (info != nullptr) {
            return info->mMove;
        }

        return reinterpret_cast<const ActorParamMove*>("");
    }

    /**
     * @brief Finds a jump parameter by name.
     * @param pName The name of the parameter.
     * @return The jump parameters, or zeroed parameters if it doesn't exist.
     */
    const ActorParamJump* ActorParamHolder::findParamJump(const char* pName) const {
        ActorParamInfo* info = tryFindParamInfoByName(pName);
        if (info != nullptr) {
            return info->mJump;
        }

        return reinterpret_cast<const ActorParamJump*>("");
    }

    /**
     * @brief Finds a sight parameter by name.
     * @param pName The name of the parameter.
     * @return The sight parameters, or zeroed parameters if it doesn't exist.
     */
    const ActorParamSight* ActorParamHolder::findParamSight(const char* pName) const {
        ActorParamInfo* info = tryFindParamInfoByName(pName);
        if (info != nullptr) {
            return info->mSight;
        }

        return reinterpret_cast<const ActorParamSight*>("");
    }

    /**
     * @brief Finds a rebound parameter by name.
     * @param pName The name of the parameter.
     * @return The rebound parameters, or zeroed parameters if it doesn't exist.
     */
    const ActorParamRebound* ActorParamHolder::findParamRebound(const char* pName) const {
        ActorParamInfo* info = tryFindParamInfoByName(pName);
        if (info != nullptr) {
            return info->mRebound;
        }

        return reinterpret_cast<const ActorParamRebound*>("");
    }

    /**
     * @brief Reads all parameters from the actor's ActorParam model resource.
     * @param pActor The actor to read the parameters of.
     */
    ActorParamHolder::ActorParamHolder(LiveActor* pActor) : mNumParams(0), mParams(nullptr) {
        getModelResource(pActor);
        ByamlIter iter(getModelResourceYaml(pActor, "ActorParam", nullptr));
        mNumParams = iter.getSize();
        mParams = new ActorParamInfo[mNumParams];

        for (s32 i = 0; i < mNumParams; i++) {
            ActorParamInfo* info = &mParams[i];
            ByamlIter paramIter;
            iter.tryGetIterByIndex(&paramIter, i);
            paramIter.tryGetStringByKey(&info->mName, "ParamName");

            ByamlIter valueIter;
            if (paramIter.tryGetIterByKey(&valueIter, "S32")) {
                info->mType = ActorParamInfo::Type_S32;
                valueIter.tryGetIntByKey(&info->mS32.mValue, "ParamS32");
            } else if (paramIter.tryGetIterByKey(&valueIter, "F32")) {
                info->mType = ActorParamInfo::Type_F32;
                valueIter.tryGetFloatByKey(&info->mF32.mValue, "ParamF32");
            } else if (paramIter.tryGetIterByKey(&valueIter, "ActorParamMove")) {
                info->mType = ActorParamInfo::Type_Move;
                ActorParamMove* move = new ActorParamMove;
                valueIter.tryGetFloatByKey(&move->mMoveAccel, "MoveAccel");
                valueIter.tryGetFloatByKey(&move->mGravity, "Gravity");
                valueIter.tryGetFloatByKey(&move->mMoveFriction, "MoveFriction");
                valueIter.tryGetFloatByKey(&move->mTurnSpeedDegree, "TurnSpeedDegree");
                info->mMove = move;
            } else if (paramIter.tryGetIterByKey(&valueIter, "ActorParamJump")) {
                info->mType = ActorParamInfo::Type_Jump;
                ActorParamJump* jump = new ActorParamJump;
                valueIter.tryGetFloatByKey(&jump->mSpeedFront, "SpeedFront");
                valueIter.tryGetFloatByKey(&jump->mSpeedUp, "SpeedUp");
                info->mJump = jump;
            } else if (paramIter.tryGetIterByKey(&valueIter, "ActorParamSight")) {
                info->mType = ActorParamInfo::Type_Sight;
                ActorParamSight* sight = new ActorParamSight;
                valueIter.tryGetFloatByKey(&sight->mDistance, "Distance");
                valueIter.tryGetFloatByKey(&sight->mDegreeH, "DegreeH");
                valueIter.tryGetFloatByKey(&sight->mDegreeV, "DegreeV");
                info->mSight = sight;
            } else if (paramIter.tryGetIterByKey(&valueIter, "ActorParamRebound")) {
                info->mType = ActorParamInfo::Type_Rebound;
                ActorParamRebound* rebound = new ActorParamRebound;
                valueIter.tryGetFloatByKey(&rebound->mReboundRate, "ReboundRate");
                valueIter.tryGetFloatByKey(&rebound->mSpeedMinToRebound, "SpeedMinToRebound");
                valueIter.tryGetFloatByKey(&rebound->mFrictionH, "FrictionH");
                info->mRebound = rebound;
            }
        }
    }
};
