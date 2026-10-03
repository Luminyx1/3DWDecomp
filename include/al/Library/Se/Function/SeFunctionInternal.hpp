#pragma once

#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Se/Project/SeKeeper.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"
#include "Project/Audio/System/SoundHeapPtrWrapper.hpp"

namespace al {
namespace detail {
/**
 * @brief Tests whether the audio user has an audio keeper and an SE keeper.
 * @param pUser Non-null audio user to query.
 * @return True when both keepers exist.
 */
inline bool isEnableSeKeeper(const IUseAudioKeeper* pUser) {
    return pUser->getAudioKeeper() != nullptr && pUser->getAudioKeeper()->getSeKeeper() != nullptr;
}

/**
 * @brief Checks whether an audio user explicitly disables sound effects.
 * @param pUser Optional audio user; nullptr or a missing audio keeper does not force invalidation.
 * @return True when the user's audio keeper disables SE.
 */
inline bool isForceInvalidSe(const IUseAudioKeeper* pUser) {
    return pUser != nullptr && pUser->getAudioKeeper() != nullptr &&
           pUser->getAudioKeeper()->isForceInvalidSe();
}

/**
 * @brief Requests a sound by its archive name.
 * @param pUser Audio user with an SE keeper unless sound effects are disabled.
 * @param rName Sound-archive name resolved to a sound identifier.
 * @param pEmitterName Optional emitter name passed to the SE keeper.
 * @return Request parameters, or nullptr if sound effects are disabled or the request fails.
 */
inline SePlayParamList* startSeOld(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                   const char* pEmitterName) {
    if (detail::isForceInvalidSe(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestPlaySe(
        alSoundNameUtil::getSoundId(rName.cstr(), false), pEmitterName, false, nullptr, nullptr, nullptr);
}

/**
 * @brief Requests or extends a held sound by its archive name.
 * @param pUser Audio user with an SE keeper unless sound effects are disabled.
 * @param rName Sound-archive name resolved to a sound identifier.
 * @param pEmitterName Optional emitter name passed to the SE keeper.
 * @return Request parameters, or nullptr if sound effects are disabled or the request fails.
 */
inline SePlayParamList* holdSeOld(const IUseAudioKeeper* pUser, const sead::SafeString& rName,
                                  const char* pEmitterName) {
    if (detail::isForceInvalidSe(pUser)) {
        return nullptr;
    }

    return pUser->getAudioKeeper()->getSeKeeper()->requestHoldSe(
        alSoundNameUtil::getSoundId(rName.cstr(), false), pEmitterName, nullptr, nullptr, nullptr);
}
} // namespace detail
} // namespace al
