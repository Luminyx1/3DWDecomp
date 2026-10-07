#include <nn/atk/atk_Sound3DManager.h>
#include <nn/atk/atk_Sound3DParam.h>
#include <nn/atk/atk_SoundArchive.h>
#include <nn/diag.h>
#include <new>

namespace {
nn::atk::Sound3DEngine sDefaultSound3DEngine;

/**
 * @brief Checks the alignment required by a spatial vector.
 * @param pVector Vector storage, which must be aligned to sixteen bytes.
 */
inline void CheckVectorAlignment(const nn::util::Vector3fType* pVector) {
    if ((reinterpret_cast<uintptr_t>(pVector) & 15) != 0) {
        nn::diag::detail::AbortImpl("", "", "", 0);
        __builtin_unreachable();
    }
}
} // namespace

namespace nn::atk {
/** @brief Constructs an inactive spatial manager with the default engine and attenuation settings. */
Sound3DManager::Sound3DManager()
    : mPool(), m_pSound3DEngine(&sDefaultSound3DEngine), m_MaxPriorityReduction(32), m_PanRange(0.9f),
      m_SonicVelocity(0), m_BiquadFilterType(-1), mRemainingSize(0), mInitialized(false) {}

/** @brief Destroys a manager after its owner has finalized its resources. */
Sound3DManager::~Sound3DManager() = default;

/**
 * @brief Computes spatial-argument storage for the archive's sound instances.
 * @param pArchive Archive whose player configuration is read; must be non-null.
 * @return Required bytes, or zero if the player configuration cannot be read.
 */
size_t Sound3DManager::GetRequiredMemSize(const SoundArchive* pArchive) {
    SoundArchive::SoundArchivePlayerInfo info;
    if (!pArchive->ReadSoundArchivePlayerInfo(&info)) {
        return 0;
    }
    return (info.sequenceSoundCount + info.streamSoundCount + info.waveSoundCount) * sizeof(Sound3DParam);
}

/**
 * @brief Initializes the ambient-argument pool for an archive.
 * @param pArchive Archive supplying the number of sound instances; must be non-null.
 * @param pBuffer Caller-owned storage for spatial arguments, aligned to sixteen bytes.
 * @param size Available buffer bytes; must cover the required archive storage.
 * @return True when initialized, or false if initialization has already occurred.
 */
bool Sound3DManager::Initialize(const SoundArchive* pArchive, void* pBuffer, size_t size) {
    if (mInitialized) {
        return false;
    }
    size_t required = GetRequiredMemSize(pArchive);
    mPool.CreateImpl(pBuffer, size, sizeof(Sound3DParam), alignof(Sound3DParam));
    mRemainingSize = size - required;
    mInitialized = true;
    return true;
}

/**
 * @brief Accounts for another archive using the existing argument pool.
 * @param pArchive Additional archive whose required storage is reserved; must be non-null.
 * @return True when initialized, or false if no argument pool has been initialized.
 */
bool Sound3DManager::InitializeWithMoreSoundArchive(const SoundArchive* pArchive) {
    if (!mInitialized) {
        return false;
    }
    mRemainingSize -= GetRequiredMemSize(pArchive);
    return true;
}

/**
 * @brief Restores default settings, detaches listeners, and releases pool bookkeeping.
 * @return True if the manager was initialized, or false if no cleanup was required.
 */
bool Sound3DManager::Finalize() {
    if (!mInitialized) {
        return false;
    }
    m_pSound3DEngine = &sDefaultSound3DEngine;
    m_MaxPriorityReduction = 32;
    m_PanRange = 0.9f;
    m_SonicVelocity = 0;
    m_BiquadFilterType = -1;
    while (!m_ListenerList.empty()) {
        m_ListenerList.erase(--m_ListenerList.end());
    }
    mPool.DestroyImpl();
    mRemainingSize = 0;
    mInitialized = false;
    return true;
}

/**
 * @brief Selects the spatial-processing engine.
 * @param pEngine Engine retained without ownership, or nullptr to disable spatial updates.
 */
void Sound3DManager::SetEngine(Sound3DEngine* pEngine) { m_pSound3DEngine = pEngine; }

/**
 * @brief Updates a sound's ambient parameters using the selected engine.
 * @param pArg Spatial argument previously allocated by this manager.
 * @param soundId Archive sound identifier passed through to the engine.
 * @param pParam Ambient parameter output, left unchanged when no engine is selected.
 */
void Sound3DManager::detail_UpdateAmbientParam(const void* pArg, u32 soundId, SoundAmbientParam* pParam) {
    if (m_pSound3DEngine != nullptr) {
        m_pSound3DEngine->detail_UpdateAmbientParam(this, static_cast<const Sound3DParam*>(pArg), soundId,
                                                    pParam);
    }
}

/**
 * @brief Computes the sound's spatial priority adjustment.
 * @param pArg Spatial argument previously allocated by this manager.
 * @param soundId Archive sound identifier passed through to the engine.
 * @return Engine priority adjustment, or zero when no engine is selected.
 */
int Sound3DManager::detail_GetAmbientPriority(const void* pArg, u32 soundId) {
    if (m_pSound3DEngine != nullptr) {
        return m_pSound3DEngine->GetAmbientPriority(this, static_cast<const Sound3DParam*>(pArg), soundId);
    }
    return 0;
}

/**
 * @brief Allocates and initializes a spatial argument from the pool.
 * @param size Requested argument size, which must equal sizeof(Sound3DParam).
 * @return Initialized argument, or nullptr when the size is wrong or the pool is exhausted.
 */
void* Sound3DManager::detail_AllocAmbientArg(size_t size) {
    if (size != sizeof(Sound3DParam)) {
        return nullptr;
    }
    void* pMemory = mPool.AllocImpl();
    return pMemory != nullptr ? new (pMemory) Sound3DParam : nullptr;
}

/**
 * @brief Returns a spatial argument to the pool.
 * @param pArg Previously allocated argument, or nullptr for no action.
 * @param pSound Unused; pool ownership is independent of the sound object.
 */
void Sound3DManager::detail_FreeAmbientArg(void* pArg, const detail::BasicSound* pSound) {
    if (pArg != nullptr) {
        mPool.FreeImpl(pArg);
    }
}

/**
 * @brief Selects the biquad filter applied by spatial attenuation.
 * @param type Filter preset identifier; -1 disables spatial filtering.
 */
void Sound3DManager::SetBiquadFilterType(int type) { m_BiquadFilterType = type; }

/** @brief Initializes spatial metadata and verifies the alignment of its vector storage. */
Sound3DParam::Sound3DParam()
    : flags(0), userParam(0), decayDistance(0), decayRatio(0.5f), decayCurve(1), dopplerFactor(0) {
    CheckVectorAlignment(&position);
    CheckVectorAlignment(&velocity);
}
} // namespace nn::atk
