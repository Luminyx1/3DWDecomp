#pragma once

#include <audio/seadAudioResourceLoaderNin.h>

namespace al {
class SeadAudioResourceLoader : public sead::AudioResourceLoaderNin {
public:
    void setArchivePath(const sead::SafeString& rPath);
};
}  // namespace al
