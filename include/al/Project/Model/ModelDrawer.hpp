#pragma once

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Model/ModelDrawerBase.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace al {
class MeshDrawerTable;

class ModelDrawer : public ModelDrawerBase {
public:
    ModelDrawer(const char* pName);

    void createTable() override;
    void registerModel(alModelCafe* pModel) override;
    void addModel(alModelCafe* pModel) override;
    void removeModel(alModelCafe* pModel) override;

    bool isDraw() const;

protected:
    void tryUpdateModelLod(SimpleModelG3D* pModel) const {
        if (!pModel->isLodDisabled()) {
            s32 updateCount = mGraphicsSystemInfo->getDrawEnvUpdateCount();

            if (pModel->getLodUpdateCount() != updateCount) {
                pModel->updateLod(mGraphicsSystemInfo->getDrawCameraPos(), updateCount);
            }
        }
    }

    s32 mModelNumMax = 0;
    s32 mModelNum = 0;
    alModelCafe** mModels = nullptr;
    MeshDrawerTable* mMeshDrawerTable = nullptr;
};

static_assert(sizeof(ModelDrawer) == 0x38);

}  // namespace al
