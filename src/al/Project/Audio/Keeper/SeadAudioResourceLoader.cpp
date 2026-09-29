#include "Project/Audio/Keeper/SeadAudioResourceLoader.hpp"

namespace al {
/**
 * @brief Sets the path of the sound archive to load from the file system.
 * @param rPath The file system path of the sound archive.
 */
void SeadAudioResourceLoader::setArchivePath(const sead::SafeString& rPath) {
    setArchiveOnFs(rPath);
}
}  // namespace al
