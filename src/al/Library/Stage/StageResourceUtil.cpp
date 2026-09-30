#include "Library/Stage/StageResourceList.hpp"

#include "Library/File/FileUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Makes the path of the stage data archive of a stage.
 * @param pOut output path
 * @param pStageName name of the stage
 * @param scenarioNo scenario number
 * @param pResourceType type of stage data
 * @param isOneResource whether the stage data is a single archive for all scenarios
 */
void makeStageDataArchivePath(sead::BufferedSafeString* pOut, const char* pStageName,
                              s32 scenarioNo, const char* pResourceType, bool isOneResource) {
    if (isOneResource) {
        pOut->format("StageData/%s", pStageName);
        return;
    }

    pOut->format("StageData/%s%s%d", pStageName, pResourceType, scenarioNo);
}

/**
 * Checks whether a stage has a single stage data archive for all scenarios.
 * @param pStageName name of the stage
 * @return true if the archive exists
 */
bool isOneStageDataArchiveExists(const char* pStageName) {
    StringTmp<128> archivePath;
    archivePath.format("StageData/%s", pStageName);
    return isExistArchive(archivePath);
}
}  // namespace al
