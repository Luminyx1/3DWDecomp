#include "controller/nin/seadNinDebugController.h"

namespace sead
{
/**
 * Constructs a disconnected debug controller.
 * @param pMgr owning controller manager
 */
NinDebugController::NinDebugController(ControllerMgr* pMgr) : Controller(pMgr), mIsConnected(false)
{
    mId = ControllerDefine::cController_NinDebug;
}

/**
 * Does nothing; the debug pad is not read in release builds.
 */
void NinDebugController::calcImpl_() {}

}  // namespace sead
