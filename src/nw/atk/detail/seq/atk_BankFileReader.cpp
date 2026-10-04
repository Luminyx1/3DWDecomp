#include <nn/atk/atk_BankFileReader.h>
#include <nn/atk/atk_BinaryFileUtil.h>

namespace nn::atk::detail {
/** @brief Constructs an empty bank reader. */
BankFileReader::BankFileReader() : mHeader(nullptr), mInfo(nullptr), mInitialized(false) {}

/**
 * @brief Constructs and initializes a reader from a bank resource.
 * @param pBankFile FBNK image, or nullptr to leave the reader empty.
 */
BankFileReader::BankFileReader(const void* pBankFile)
    : mHeader(nullptr), mInfo(nullptr), mInitialized(false) {
    Initialize(pBankFile);
}

/**
 * @brief Validates a version-1 bank header and attaches its INFO block.
 * @param pBankFile Complete FBNK resource; nullptr or incompatible headers are ignored.
 */
void BankFileReader::Initialize(const void* pBankFile) {
    if (pBankFile == nullptr) {
        return;
    }
    const auto* pHeader = static_cast<const BankFile::FileHeader*>(pBankFile);
    if (pHeader->signature != 0x4b4e4246 || pHeader->byteOrder != 0xfeff || pHeader->version != 0x10000) {
        return;
    }
    mHeader = pHeader;
    const auto* pInfo = pHeader->GetInfoBlock();
    if (pInfo->signature != 0x4f464e49) {
        return;
    }
    mInfo = &pInfo->body;
    mInitialized = true;
}

/** @brief Clears the resource pointers if the reader was successfully initialized. */
void BankFileReader::Finalize() {
    if (mInitialized) {
        mInitialized = false;
        mHeader = nullptr;
        mInfo = nullptr;
    }
}

/**
 * @brief Resolves a bank program, key and velocity into complete note parameters.
 * @param pInfo Output record receiving the wave and envelope parameters; must be non-null.
 * @param program Nonnegative instrument index.
 * @param key Note key used to select the key region, normally in [0, 127].
 * @param velocity Note velocity used to select the velocity region, normally in [0, 127].
 * @return True when every required reference is present and the output was populated.
 */
bool BankFileReader::ReadVelocityRegionInfo(VelocityRegionInfo* pInfo, int program, int key,
                                            int velocity) const {
    if (!mInitialized || program < 0) {
        return false;
    }
    if (static_cast<int>(mInfo->GetInstrumentReferenceTable()->count) <= program) {
        return false;
    }
    const auto* pInstrument = mInfo->GetInstrument(program);
    if (pInstrument == nullptr) {
        return false;
    }
    const auto* pKey = pInstrument->GetKeyRegion(key);
    if (pKey == nullptr) {
        return false;
    }
    const auto* pRegion = pKey->GetVelocityRegion(velocity);
    if (pRegion == nullptr) {
        return false;
    }
    if (!bank::ReadWaveId(mInfo, &pInfo->waveId, pRegion->waveIndex)) {
        return false;
    }
    const auto* pParameter = pRegion->GetRegionParameter();
    if (pParameter != nullptr) {
        pInfo->originalKey = pParameter->originalKey;
        pInfo->volume = pParameter->volume;
        pInfo->pan = pParameter->pan;
        pInfo->pitch = pParameter->pitch;
        pInfo->ignoreNoteOff = pParameter->ignoreNoteOff;
        pInfo->keyGroup = pParameter->keyGroup;
        pInfo->interpolationType = pParameter->interpolationType;
        std::memcpy(&pInfo->attack, &pParameter->envelope, sizeof(AdshrCurve));
    } else {
        pInfo->originalKey = pRegion->GetOriginalKey();
        pInfo->volume = pRegion->GetVolume();
        pInfo->pan = pRegion->GetPan();
        pInfo->pitch = pRegion->GetPitch();
        pInfo->ignoreNoteOff = pRegion->IsIgnoreNoteOff();
        pInfo->keyGroup = pRegion->GetKeyGroup();
        pInfo->interpolationType = pRegion->GetInterpolationType();
        const auto* pEnvelope = pRegion->GetAdshrCurve();
        std::memcpy(&pInfo->attack, pEnvelope, sizeof(AdshrCurve));
    }
    return true;
}

/** @brief Gets the bank's referenced waves. @return Wave table, or nullptr before initialization. */
const WaveIdTable* BankFileReader::GetWaveIdTable() const {
    if (!mInitialized) {
        return nullptr;
    }
    return mInfo->GetWaveIdTable();
}

} // namespace nn::atk::detail
