#include "Project/Model/ModelAdditionalInfo.hpp"

namespace al {

/**
 * Constructs the additional info and fetches the light info of its cube map category.
 * @param pInfo Graphics system info.
 * @param isSecondCategory Whether to use cube map category 1 instead of 0.
 */
ModelAdditionalInfo::ModelAdditionalInfo(const GraphicsSystemInfo* pInfo, bool isSecondCategory)
    : ModelAdditionalInfo(pInfo) {
    if (isSecondCategory) {
        mCategory = 1;
    }
    updateLightInfo();
}

}  // namespace al
