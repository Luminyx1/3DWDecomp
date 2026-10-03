#include "Library/Se/Project/SePlayParamList.hpp"

#include <attributes.h>
#include "Library/Se/Project/SeMaterialInfoKeeper.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
/**
 * @brief Writes a parameter only when its existing or a free record is available.
 * @tparam Value Primary value type, converted to float after record selection.
 * @tparam Secondary Secondary value type, converted to float after record selection.
 * @param pList Initialized list receiving the parameter.
 * @param type Parameter type to store.
 * @param value Primary value to store.
 * @param secondary Secondary value to store.
 * @param index Secondary value to match, or -1 to match by type alone.
 */
template <typename Value, typename Secondary>
inline void setParamRecord(SePlayParamList* pList, s32 type, Value value, Secondary secondary,
                           s32 index = -1) {
    SePlayParam* pParam = pList->findAvailableRecord(type, index);
    if (pParam != nullptr) {
        pParam->set(type, value, secondary);
    }
}
} // namespace
/** @brief Constructs an unused record with a neutral multiplier. */
SePlayParam::SePlayParam() { reset(); }

/** @brief Clears the type and restores the primary and secondary defaults. */
void SePlayParam::reset() {
    mType = 0;
    mValue = 1.0f;
    mSecondary = 0.0f;
}

/**
 * @brief Replaces a parameter record.
 * @param type Parameter type; zero marks an unused record.
 * @param value Primary value.
 * @param secondary Secondary value, filter type, or sequence variable index.
 */
void SePlayParam::set(s32 type, f32 value, f32 secondary) {
    mType = type;
    mValue = value;
    mSecondary = secondary;
}

/**
 * @brief Multiplies a record's primary value and replaces its other fields.
 * @param type Parameter type to assign.
 * @param value Factor applied to the current primary value.
 * @param secondary Replacement secondary value.
 */
void SePlayParam::setMul(s32 type, f32 value, f32 secondary) {
    mType = type;
    mValue *= value;
    mSecondary = secondary;
}

/** @brief Allocates the twelve parameter records and initializes output defaults. */
SePlayParamList::SePlayParamList() : _8(false), _c(1) {
    mParams = new SePlayParam*[cRecordCount];
    for (s32 i = 0; i < cRecordCount; ++i) {
        mParams[i] = new SePlayParam;
    }
}

/** @brief Clears every record and restores the output defaults. */
void SePlayParamList::reset() {
    for (s32 i = 0; i < cRecordCount; ++i) {
        mParams[i]->reset();
    }
    _8 = false;
    _c = 1;
}

/**
 * @brief Finds an existing parameter record or the first unused record.
 * @param type Parameter type to find.
 * @param index Required secondary value, or a negative value to ignore that field.
 * @return Matching record, first unused record, or nullptr when all records are occupied.
 */
NOINLINE SePlayParam* SePlayParamList::findAvailableRecord(s32 type, s32 index) const {
    for (s32 i = 0; i < cRecordCount; ++i) {
        SePlayParam* pParam = mParams[i];
        if (pParam->getType() == type && (index < 0 || pParam->getSecondary() == index)) {
            return pParam;
        }
    }
    for (s32 i = 0; i < cRecordCount; ++i) {
        if (mParams[i]->getType() == 0) {
            return mParams[i];
        }
    }
    return nullptr;
}

/**
 * @brief Stores the sound's volume when a record is available.
 * @param volume Requested volume; no clamping is performed.
 */
void SePlayParamList::setVolume(f32 volume) { setParamRecord(this, 1, volume, 0.0f); }

/**
 * @brief Stores the sound's volume multiplier when a record is available.
 * @param volume Requested volume multiplier; no clamping is performed.
 */
void SePlayParamList::setMulVolume(f32 volume) {
    SePlayParam* pParam = findAvailableRecord(1, -1);
    if (pParam != nullptr) {
        pParam->setMul(1, volume, 0.0f);
    }
}

/**
 * @brief Stores the sound's pitch ratio when a record is available.
 * @param pitch Requested pitch ratio; no clamping is performed.
 */
void SePlayParamList::setPitch(f32 pitch) { setParamRecord(this, 2, pitch, 0.0f); }

/**
 * @brief Stores the sound's tempo ratio when a record is available.
 * @param tempo Requested tempo ratio; no clamping is performed.
 */
void SePlayParamList::setTempo(f32 tempo) { setParamRecord(this, 3, tempo, 0.0f); }

/**
 * @brief Stores the sound's low-pass filter frequency when a record is available.
 * @param freq Requested low-pass filter frequency; no clamping is performed.
 */
void SePlayParamList::setLpfFreq(f32 freq) { setParamRecord(this, 6, freq, 0.0f); }

/**
 * @brief Stores the biquad filter setting when a record is available.
 * @param value Filter amount; no clamping is performed.
 * @param type Filter type, stored as a floating-point secondary value.
 */
void SePlayParamList::setBiquadFilter(f32 value, s32 type) { setParamRecord(this, 7, value, type); }

/**
 * @brief Stores a local sequence variable when a record is available.
 * @param value Integer variable value, stored as a float.
 * @param index Variable index; nonnegative indices distinguish records of the same type.
 */
void SePlayParamList::setLocalVariable(s32 value, s32 index) { setParamRecord(this, 4, value, index, index); }

/**
 * @brief Stores a global sequence variable when a record is available.
 * @param value Integer variable value, stored as a float.
 * @param index Variable index; nonnegative indices distinguish records of the same type.
 */
void SePlayParamList::setGlobalVariable(s32 value, s32 index) {
    setParamRecord(this, 5, value, index, index);
}

/**
 * @brief Reads a local sequence variable without modifying the output on failure.
 * @param rValue Receives the stored value converted to an integer.
 * @param index Variable index to find.
 * @return True when a matching local variable record exists.
 */
bool SePlayParamList::tryGetLocalVariable(s32& rValue, s32 index) const {
    for (s32 i = 0; i < cRecordCount; ++i) {
        const SePlayParam* pParam = mParams[i];
        if (pParam->getType() == 4 && pParam->getSecondary() == index) {
            rValue = pParam->getValue();
            return true;
        }
    }
    return false;
}

/**
 * @brief Stores a pair of speaker volumes when a record is available.
 * @param lfe Lfe speaker volume; no clamping is performed.
 * @param center Center speaker volume; no clamping is performed.
 */
void SePlayParamList::setSpeakerVolumeLfeCenter(f32 lfe, f32 center) { setParamRecord(this, 8, lfe, center); }

/**
 * @brief Stores a pair of speaker volumes when a record is available.
 * @param left Left speaker volume; no clamping is performed.
 * @param right Right speaker volume; no clamping is performed.
 */
void SePlayParamList::setSpeakerVolumeFrontLR(f32 left, f32 right) { setParamRecord(this, 9, left, right); }

/**
 * @brief Stores a pair of speaker volumes when a record is available.
 * @param left Left speaker volume; no clamping is performed.
 * @param right Right speaker volume; no clamping is performed.
 */
void SePlayParamList::setSpeakerVolumeRearLR(f32 left, f32 right) { setParamRecord(this, 10, left, right); }

/**
 * @brief Gets a record by its physical slot.
 * @param index Slot index in the range [0, cRecordCount).
 * @return Mutable parameter record at the requested slot.
 */
SePlayParam* SePlayParamList::getParam(s32 index) const { return mParams[index]; }

/** @brief Checks whether all records are unused. @return True if every record has type zero. */
bool SePlayParamList::isParamEmpty() const {
    for (s32 i = 0; i < cRecordCount; ++i) {
        if (mParams[i]->getType() != 0) {
            return false;
        }
    }
    return true;
}
} // namespace al

namespace al {
namespace {
using MaterialInfoList = AudioInfoList<SeMaterialSettingInfo>;

/**
 * @brief Looks up a material entry when both the list and name exist.
 * @param pList Optional list of material overrides.
 * @param pName Optional null-terminated material key.
 * @return Matching entry, or nullptr if no entry can be found.
 */
inline SeMaterialSettingInfo* tryFindMaterial(const MaterialInfoList* pList, const char* pName) {
    return pList != nullptr && pName != nullptr ? pList->tryFindInfo(pName) : nullptr;
}

/**
 * @brief Finds a state-specific material override, falling back to the state alone.
 * @param pInfo Resource settings; must not be nullptr.
 * @param pMaterialName Optional material suffix; nullptr is treated as an empty suffix.
 * @param pStateName Null-terminated state prefix.
 * @return Matching override, or nullptr when neither key exists.
 */
ALWAYS_INLINE inline SeMaterialSettingInfo*
findStateMaterial(const SeResourceSpecificInfo* pInfo, const char* pMaterialName, const char* pStateName) {
    StringTmp<64> name("%s%s", pStateName, pMaterialName != nullptr ? pMaterialName : "");
    SeMaterialSettingInfo* pResult = tryFindMaterial(pInfo->mMaterialInfoList, name.cstr());
    if (pResult == nullptr) {
        pResult = tryFindMaterial(pInfo->mMaterialInfoList, pStateName);
    }
    return pResult;
}
} // namespace

/**
 * @brief Retains the sound database associated with material overrides.
 * @param pDataBase Database pointer, stored without taking ownership.
 */
SeMaterialInfoKeeper::SeMaterialInfoKeeper(SeDataBase* pDataBase) : mDataBase(pDataBase) {}

/** @brief Performs no additional initialization in this version. */
void SeMaterialInfoKeeper::init() {}

/**
 * @brief Resolves state-specific and material-specific sound replacements.
 * @param soundId Original sound identifier, returned when no override exists.
 * @param pMaterialName Optional null-terminated material name.
 * @param materialState State selector: 1 is wet, 2 is water, and 3 is single-mode wet.
 * @param pInfo Optional resource settings containing material overrides.
 * @return Replacement sound identifier, or the original identifier.
 */
s32 SeMaterialInfoKeeper::findReplacedId(s32 soundId, const char* pMaterialName, s32 materialState,
                                         const SeResourceSpecificInfo* pInfo) {
    if (pInfo == nullptr) {
        return soundId;
    }
    SeMaterialSettingInfo* pResult = nullptr;
    switch (materialState) {
    case 2:
        pResult = findStateMaterial(pInfo, pMaterialName, "Water");
        break;
    case 1:
        pResult = findStateMaterial(pInfo, pMaterialName, "Wet");
        break;
    case 3:
        pResult = findStateMaterial(pInfo, pMaterialName, "SingleModeWet");
        break;
    default:
        break;
    }
    if (pMaterialName != nullptr && pResult == nullptr) {
        pResult = tryFindMaterial(pInfo->mMaterialInfoList, pMaterialName);
    }
    return pResult != nullptr ? pResult->mSoundId : soundId;
}
} // namespace al
