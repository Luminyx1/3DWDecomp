#include "Project/OceanWave/OceanWaveKeeper.hpp"

#include "Project/OceanWave/OceanWaveUserInfo.hpp"

namespace al {
/**
 * Constructs an ocean wave keeper.
 * @param pDirector ocean wave director
 */
OceanWaveKeeper::OceanWaveKeeper(OceanWaveDirector* pDirector) : mDirector(pDirector) {}

/**
 * Initializes the ocean wave user info.
 * @param pName user name
 * @param rIter ocean wave parameter iterator
 */
void OceanWaveKeeper::init(const char* pName, ByamlIter& rIter) {
    mName = pName;
    mUserInfo = OceanWaveUserInfo::createInfo(rIter, pName);
}
}  // namespace al
