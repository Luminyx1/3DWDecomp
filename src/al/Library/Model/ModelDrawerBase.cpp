#include "Library/Model/ModelDrawerBase.hpp"

namespace al {

/**
 * Constructs a model drawer.
 * @param pName Name of the drawer.
 */
ModelDrawerBase::ModelDrawerBase(const char* pName)
    : mName(pName), mModel(nullptr), mGraphicsSystemInfo(nullptr) {}

/**
 * Sets the graphics system info and the model drawn by this drawer.
 * @param pInfo Graphics system info.
 * @param pModel Model.
 */
void ModelDrawerBase::setDrawInfo(GraphicsSystemInfo* pInfo, alModelCafe* pModel) {
    mModel = pModel;
    mGraphicsSystemInfo = pInfo;
}

}  // namespace al
