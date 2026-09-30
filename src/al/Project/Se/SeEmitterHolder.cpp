#include "Project/Se/SeEmitterHolder.hpp"

#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Library/Se/Info/SeSource.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Se/SeEmitter.hpp"

namespace al {
/**
 * Creates the emitters of all emitter information.
 * @param pInfo Audio system information.
 * @param rName Unused.
 * @param pEmitterInfoList Emitter information list.
 * @param pModelKeeper Model keeper of the owner.
 * @param pPose SE source pose.
 * @param isUseModel Whether the emitters use the model.
 */
SeEmitterHolder::SeEmitterHolder(AudioSystemInfo* pInfo, const sead::SafeString& rName,
                                 const AudioInfoList<SeEmitterInfo>* pEmitterInfoList,
                                 const ModelKeeper* pModelKeeper, SeSourcePose* pPose, bool isUseModel) {
    mEmitters.allocBuffer(pEmitterInfoList != nullptr ? pEmitterInfoList->getInfoNum() : 0, nullptr);
    for (s32 i = 0; i < (pEmitterInfoList != nullptr ? pEmitterInfoList->getInfoNum() : 0); i++) {
        const SeEmitterInfo* emitterInfo = pEmitterInfoList->getInfo(i);
        mEmitters.pushBack(new SeEmitter(pInfo, emitterInfo, pModelKeeper, pPose, isUseModel));
    }
}

/**
 * Updates all emitters.
 */
void SeEmitterHolder::update() {
    bool isAllEnd = true;
    for (s32 i = 0; i < mEmitters.size(); i++) {
        isAllEnd &= mEmitters.unsafeAt(i)->update();
    }

    if (isAllEnd) {
        mIsActive = false;
    }
}

/**
 * Finds an emitter by name.
 * @param pName Emitter name, or nullptr for the first emitter.
 * @return Found emitter, or nullptr.
 */
SeEmitter* SeEmitterHolder::findEmitter(const char* pName) const {
    if (pName == nullptr) {
        return mEmitters.unsafeAt(0);
    }

    for (s32 i = 0; i < mEmitters.size(); i++) {
        SeEmitter* emitter = mEmitters.unsafeAt(i);
        if (isEqualString(emitter->getName(), pName)) {
            return emitter;
        }
    }

    return nullptr;
}

/**
 * Gets an emitter by index.
 * @param index Emitter index.
 * @return Emitter, or nullptr if the index is out of range.
 */
SeEmitter* SeEmitterHolder::getEmitter(s32 index) const {
    if (index < mEmitters.size()) {
        return mEmitters.unsafeAt(index);
    }

    return nullptr;
}

/**
 * Resets the velocity of all emitter sources.
 */
void SeEmitterHolder::resetVelocity() {
    for (s32 i = 0; i < mEmitters.size(); i++) {
        mEmitters.unsafeAt(i)->getSeSource()->resetVelocity();
    }
}
}  // namespace al
