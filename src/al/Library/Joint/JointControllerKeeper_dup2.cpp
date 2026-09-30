#include "Library/Joint/JointControllerKeeper.hpp"

#include <nn/util/util_MatrixApi.h>

#include "Library/Joint/JointControllerBase.hpp"

namespace al {

/**
 * Constructs a keeper with room for a number of controllers.
 * @param maxControllers Maximum number of controllers.
 */
JointControllerKeeper::JointControllerKeeper(s32 maxControllers) {
    mControllers.allocBuffer(maxControllers, nullptr);
}

/**
 * Runs every controller registered for the current bone and schedules the next callback bone.
 * @param rArg Callback bone information.
 * @param rManip World matrix of the current bone.
 */
void JointControllerKeeper::Exec(CallbackArg& rArg, nn::g3d::WorldMtxManip& rManip) {
    if (mControllers.size() == 0) {
        return;
    }

    s32 nextBoneIndex = 0x7fffffff;
    for (s32 i = 0; i < mControllers.size(); i++) {
        JointControllerBase* controller = *mControllers.unsafeAt(i);
        if (controller->isExistId(rArg.GetBoneIndex())) {
            sead::Matrix34f mtx = sead::Matrix34f::ident;
            nn::util::MatrixStore(reinterpret_cast<nn::util::FloatColumnMajor4x3*>(&mtx),
                                  *rManip.GetMtx());
            controller->calcJointCallback(rArg.GetBoneIndex(), &mtx);
            nn::util::MatrixLoad(rManip.GetMtx(),
                                 *reinterpret_cast<const nn::util::FloatColumnMajor4x3*>(&mtx));
        }

        s32 nextId = 0xffff;
        if (controller->findNextId(&nextId, rArg.GetBoneIndex()) && nextBoneIndex > nextId) {
            nextBoneIndex = nextId;
        }
    }
    rArg.SetCallbackBoneIndex(nextBoneIndex);
}

/**
 * Registers a controller.
 * @param pController Controller to add.
 */
void JointControllerKeeper::pushBackController(JointControllerBase* pController) {
    *mControllers.birthBack() = pController;
}

}  // namespace al
