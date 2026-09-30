#include "Library/MapObj/EffectMtxSetter.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Effect/Core/EffectUtil.hpp"

namespace al {
/**
 * Constructs an effect matrix setter.
 * @param pActor actor owning the effects
 */
EffectMtxSetter::EffectMtxSetter(LiveActor* pActor) : mActor(pActor) {}

/**
 * Creates the matrix infos from a byml array.
 * @param rIter array of matrix infos
 */
void EffectMtxSetter::init(const ByamlIter& rIter) {
    if (!rIter.isValid() || !rIter.isTypeArray()) {
        return;
    }
    s32 size = rIter.getSize();
    mInfoNum = size;
    mInfos = new EffectMtxInfo[size];
    for (s32 i = 0; i < mInfoNum; i++) {
        ByamlIter infoIter;
        rIter.tryGetIterByIndex(&infoIter, i);
        mInfos[i].init(infoIter);
    }
}

/**
 * Reads the matrix name and the effect names.
 * @param rIter matrix info
 */
void EffectMtxInfo::init(const ByamlIter& rIter) {
    if (!rIter.tryGetStringByKey(&mMtxName, "MtxName")) {
        return;
    }
    if (!rIter.isExistKey("EffectName")) {
        return;
    }
    if (isTypeStringByKey(rIter, "EffectName")) {
        mEffectNum = 1;
        mEffectNames = new const char*[1];
        rIter.tryGetStringByKey(mEffectNames, "EffectName");
        return;
    }
    if (!isTypeArrayByKey(rIter, "EffectName")) {
        return;
    }
    ByamlIter nameIter;
    rIter.tryGetIterByKey(&nameIter, "EffectName");
    s32 size = nameIter.getSize();
    mEffectNum = size;
    mEffectNames = new const char*[size];
    for (s32 i = 0; i < mEffectNum; i++) {
        mEffectNames[i] = nullptr;
        if (isTypeStringByIndex(nameIter, i)) {
            nameIter.tryGetStringByIndex(&mEffectNames[i], i);
        }
    }
}

/**
 * Sets the matrix of the effects registered to a matrix name.
 * @param pMtx matrix
 * @param pMtxName matrix name
 */
void EffectMtxSetter::setMtxPtr(const sead::Matrix34f* pMtx, const char* pMtxName) {
    EffectMtxInfo* info = tryFindEffectMtxInfo(pMtxName);
    if (info) {
        info->setMtxPtr(mActor, pMtx);
    }
}

/**
 * Finds the matrix info of a matrix name.
 * @param pMtxName matrix name
 * @return matrix info, or null
 */
EffectMtxInfo* EffectMtxSetter::tryFindEffectMtxInfo(const char* pMtxName) {
    for (s32 i = 0; i < mInfoNum; i++) {
        if (isEqualString(mInfos[i].mMtxName, pMtxName)) {
            return &mInfos[i];
        }
    }
    return nullptr;
}

/**
 * Sets the follow matrix of all effects.
 * @param pActor actor owning the effects
 * @param pMtx matrix
 */
void EffectMtxInfo::setMtxPtr(LiveActor* pActor, const sead::Matrix34f* pMtx) {
    mMtx = pMtx;
    for (s32 i = 0; i < mEffectNum; i++) {
        if (mEffectNames[i]) {
            setEffectFollowMtxPtr(pActor, mEffectNames[i], pMtx);
        }
    }
}

/**
 * Constructs an empty matrix info.
 */
EffectMtxInfo::EffectMtxInfo() = default;

/**
 * Creates an effect matrix setter if the model has the yaml resource.
 * @param pActor actor
 * @param pName yaml resource name
 * @return effect matrix setter, or null
 */
EffectMtxSetter* tryCreateEffectMtxSetter(LiveActor* pActor, const char* pName) {
    if (!isExistModelResourceYaml(pActor, pName, nullptr)) {
        return nullptr;
    }
    EffectMtxSetter* setter = new EffectMtxSetter(pActor);
    setter->init(ByamlIter(getModelResourceYaml(pActor, pName, nullptr)));
    return setter;
}
}  // namespace al
