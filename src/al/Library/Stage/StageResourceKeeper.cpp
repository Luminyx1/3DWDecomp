#include "Library/Stage/StageResourceKeeper.hpp"

#include "Library/File/FileUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Stage/StageInfo.hpp"
#include "Library/Stage/StageResourceList.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs an empty stage resource keeper.
 */
StageResourceKeeper::StageResourceKeeper() = default;

/**
 * Loads the map, design and sound resources of a stage.
 * @param pStageName name of the stage
 * @param scenarioNo scenario number
 */
void StageResourceKeeper::initAndLoadResource(const char* pStageName, s32 scenarioNo) {
    mStageResourceLists = new StageResourceList*[3];
    StringTmp<128> archivePath;
    makeStageDataArchivePath(&archivePath, pStageName, scenarioNo, "Map", true);
    bool isOneResource = isExistArchive(archivePath);
    mStageResourceLists[0] = new StageResourceList(pStageName, scenarioNo, "Map", isOneResource);
    mStageResourceLists[1] =
        new StageResourceList(pStageName, scenarioNo, "Design", isOneResource);
    mStageResourceLists[2] = new StageResourceList(pStageName, scenarioNo, "Sound", isOneResource);

    if (mStageResourceLists[1]->getStageResourceNum() > 0) {
        StringTmp<256> fileName("%sDesign%d.bfres", pStageName, scenarioNo);
        Resource* resource = mStageResourceLists[1]->getStageInfo(0)->getResource();

        if (resource->isExistFile(fileName)) {
            resource->tryCreateResGraphicsFile(fileName, nullptr);
        }
    }
}
}  // namespace al
