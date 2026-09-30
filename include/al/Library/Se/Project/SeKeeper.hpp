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
    SeKeeper(AudioSystemInfo* pInfo, SeDirector* pDirector, const char* pUserName, const sead::Vector3f* pTrans,
             const sead::Matrix34f* pMtx, const ModelKeeper* pModelKeeper, const char* pPlayName);

    void update();
    SePlayParamList* requestPlaySe(u32 id, const char* pEmitterName, bool isHold, MeInfo* pMeInfo,
                                   const SeResourceSpecificInfo* pSpecificInfo, const char* pPlayName);
    s32 getWaterState();
    void applyKeeperParamsToParamList(SePlayParamList* pParamList, u32 id, bool isHold);
    SePlayParamList* requestHoldSe(u32 id, const char* pEmitterName, MeInfo* pMeInfo,
                                   const SeResourceSpecificInfo* pSpecificInfo, const char* pPlayName);
    s32 requestPlaySe(const char* pName, MeInfo* pMeInfo, bool isHold);
    s32 requestPlaySeFromName(const char* pName, MeInfo* pMeInfo, bool isHold);
    SePlayParamList* requestPlaySeFromNameWithParam(const char* pName, f32 param, MeInfo* pMeInfo, bool isHold,
                                                    bool isTry);
    SePlayParamList* requestPlaySeFromNameGetParamList(const char* pName, MeInfo* pMeInfo, bool isHold);
    void stopSe(u32 id, s32 fadeFrames, const char* pEmitterName, const char* pPlayName);
    void stopAllSeFromName(const char* pName, s32 fadeFrames, const char* pPlayName, bool isAll);
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

    SeDirector* getSeDirector() const { return mSeDirector; }
    const SeUserInfo* getUserInfo() const { return mUserInfo; }
    void setModifier(ISeModifier* pModifier) { mModifier = pModifier; }
    const char* getMaterialName() const { return mMaterialName; }
    void resetMaterialName() { mMaterialName = nullptr; }
    bool isMaterialWetSingleMode() const { return mIsMaterialWetSingleMode; }
    void setIsBeyondWall(bool isBeyondWall) { mIsBeyondWall = isBeyondWall; }

private:
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
}  // namespace al
