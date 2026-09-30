#include "Project/Audio/System/SeadAudioResourceLoader.hpp"

namespace al {
/**
 * Sets the path of the sound archive on the file system.
 * @param rPath Archive path.
 */
void SeadAudioResourceLoader::setArchivePath(const sead::SafeString& rPath) {
    setArchiveOnFs(rPath);
}
}  // namespace al
