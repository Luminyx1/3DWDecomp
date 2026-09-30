#include "Library/Se/Info/SeSource.hpp"

#include "Project/Audio/System/SeadAudio3DMgr.hpp"

namespace al {
/**
 * Constructs the SE source base.
 * @param rName Source name.
 */
SeSource::SeSource(const sead::SafeString& rName) : mName(rName) {}

/**
 * Gets the position of the default listener.
 * @return Listener position.
 */
const sead::Vector3f& SeadAudio3DMgr::getListenerPosition() const {
    const nn::atk::Sound3DListener* listener = getDefaultListener();
    return reinterpret_cast<const sead::Vector3f&>(listener->GetPosition());
}
}  // namespace al
