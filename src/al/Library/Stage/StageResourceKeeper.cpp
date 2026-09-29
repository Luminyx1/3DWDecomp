#include "Library/Stage/StageResourceKeeper.hpp"

#include "Library/File/FileUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Stage/StageInfo.hpp"
#include "Library/Stage/StageResourceList.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * @brief Constructs an empty resource keeper.
 */
StageResourceKeeper::StageResourceKeeper() = default;

/**
 * @brief Creates the Map, Design and Sound resource lists of a stage and loads its design model.
 * @param pStageName The name of the stage.
 * @param scenarioNo The scenario number of the stage.
 */
void StageResourceKeeper::initAndLoadResource(const char* pStageName, s32 scenarioNo) {
    mResourceLists = new StageResourceList*[3];

    StringTmp<128> archivePath;
    makeStageDataArchivePath(&archivePath, pStageName, scenarioNo, "Map", true);
    bool isExist = isExistArchive(archivePath);

    mResourceLists[0] = new StageResourceList(pStageName, scenarioNo, "Map", isExist);
    mResourceLists[1] = new StageResourceList(pStageName, scenarioNo, "Design", isExist);
    mResourceLists[2] = new StageResourceList(pStageName, scenarioNo, "Sound", isExist);

    if (mResourceLists[1]->getStageResourceNum() > 0) {
        StringTmp<256> modelName("%sDesign%d.bfres", pStageName, scenarioNo);
        Resource* pResource = mResourceLists[1]->getStageInfo(0)->mResource;
        if (pResource->isExistFile(modelName)) {
            pResource->tryCreateResGraphicsFile(modelName, nullptr);
        }
    }
}
}  // namespace al
