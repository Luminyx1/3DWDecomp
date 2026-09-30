#pragma once

namespace al {
class ByamlIter;
class OceanWaveDirector;
class OceanWaveUserInfo;

class OceanWaveKeeper {
public:
    OceanWaveKeeper(OceanWaveDirector* pDirector);

    void init(const char* pName, ByamlIter& rIter);

    OceanWaveDirector* getDirector() const {
        return mDirector;
    }

    OceanWaveUserInfo* getUserInfo() const {
        return mUserInfo;
    }

private:
    const char* mName = nullptr;
    OceanWaveDirector* mDirector;
    OceanWaveUserInfo* mUserInfo = nullptr;
};
}  // namespace al
