#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class AudioSystemInfo;
class IAudioResourceLoader;
class MeInfo;
class ModelKeeper;
class SeadAudioPlayer;
class SeDataBase;
class SeDirector;
class SeEmitterHolder;
class SeSourcePose;
class SePlayParamList;
class SeResourceSpecificInfo;
class SeUserInfo;
class ISeModifier;

struct SeqLocalVariableDefault {
    s32 index = -1;
    s32 value = -1;
};

struct BiquadFilterDefault {
    s32 type = 0;
    f32 value = 0.0f;
};

class SeKeeper {
  public:
    SeKeeper(AudioSystemInfo* pInfo, SeDirector* pDirector, const char* pUserName,
             const sead::Vector3f* pTrans, const sead::Matrix34f* pMtx, const ModelKeeper* pModelKeeper,
             const char* pPlayName);

    void update();
    SePlayParamList* requestPlaySe(u32 id, const char* pEmitterName, bool isHold, MeInfo* pMeInfo,
                                   const SeResourceSpecificInfo* pSpecificInfo, const char* pPlayName);
    s32 getWaterState();
    void applyKeeperParamsToParamList(SePlayParamList* pParamList, u32 id, bool isHold);
    SePlayParamList* requestHoldSe(u32 id, const char* pEmitterName, MeInfo* pMeInfo,
                                   const SeResourceSpecificInfo* pSpecificInfo, const char* pPlayName);
    s32 requestPlaySe(const char* pName, MeInfo* pMeInfo, bool isHold);
    s32 requestPlaySeFromName(const char* pName, MeInfo* pMeInfo, bool isHold);
    SePlayParamList* requestPlaySeFromNameWithParam(const char* pName, f32 param, MeInfo* pMeInfo,
                                                    bool isHold, bool isTry);
    SePlayParamList* requestPlaySeFromNameGetParamList(const char* pName, MeInfo* pMeInfo, bool isHold);
    void stopSe(u32 id, s32 fadeFrames, const char* pEmitterName, const char* pPlayName);
    void stopAllSeFromName(const char* pName, s32 fadeFrames, const char* pPlayName, bool isSingle);
    void stopSe(const char* pName);
    void stopSeFromName(const char* pName);
    void stopAll(s32 fadeFrames);
    void activate();
    void deactivate(bool isClipped);
    void resetVelocity();
    void setIsInWater(bool isInWater);
    void setIsMaterialWet(bool isWet);
    void setIsMaterialWetSingleMode(bool isWet);
    void tryUpdateMaterial(const char* pMaterialName);
    void setSeqLocalVariableDefault(s32 index, s32 value);
    void setBiquadFilterDefault(s32 type, f32 value);
    void setSeSourceVolume(f32 volume);
    void loadSe(IAudioResourceLoader* pLoader);
    void verifySe(SeadAudioPlayer* pPlayer, const char* pName);

    /** @brief Gets the director receiving sound requests. @return Associated sound director. */
    SeDirector* getSeDirector() const { return mSeDirector; }
    /** @brief Gets the resolved user definition. @return User information, or nullptr if unresolved. */
    const SeUserInfo* getUserInfo() const { return mUserInfo; }
    /** @brief Sets the optional request modifier. @param pModifier Modifier to retain, or nullptr to clear it. */
    void setModifier(ISeModifier* pModifier) { mModifier = pModifier; }
    /** @brief Gets the current material name. @return Retained material name, or nullptr. */
    const char* getMaterialName() const { return mMaterialName; }
    /** @brief Clears the material name without notifying emitter sources. */
    void resetMaterialName() { mMaterialName = nullptr; }
    /** @brief Checks the single-mode wet flag. @return True when this flag is set. */
    bool isMaterialWetSingleMode() const { return mIsMaterialWetSingleMode; }
    /** @brief Sets the wall-occlusion flag. @param isBeyondWall Whether a wall separates the source and listener. */
    void setIsBeyondWall(bool isBeyondWall) { mIsBeyondWall = isBeyondWall; }

  private:
    /** @brief Resolves material-state precedence: water, single-mode wet, then wet.
     * @return State selector in the range 0 through 3.
     */
    s32 calcWaterState() const {
        u8 state = mIsMaterialWet;
        if (mIsMaterialWetSingleMode) {
            state = 3;
        }
        if (mIsInWater) {
            state = 2;
        }
        return state;
    }

    SeEmitterHolder* mEmitterHolder = nullptr;
    SeDirector* mSeDirector;
    SeSourcePose* mPose = nullptr;
    const SeUserInfo* mUserInfo = nullptr;
    const char* mUserName;
    const char* mMaterialName = nullptr;
    bool mIsActive = true;
    const ModelKeeper* mModelKeeper;
    SeqLocalVariableDefault** mSeqLocalVariables;
    BiquadFilterDefault* mBiquadFilter;
    bool mIsInWater = false;
    bool mIsMaterialWet = false;
    bool mIsMaterialWetSingleMode = false;
    bool mIsBeyondWall = false;
    const char* mPlayName;
    ISeModifier* mModifier = nullptr;
    SeDataBase* mSeDataBase = nullptr;
};

static_assert(sizeof(SeKeeper) == 0x70);
} // namespace al
