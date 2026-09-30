#include <eui/euiBoxCursorMgr.h>
namespace eui {
BoxCursorMgr::BoxCursorMgr() : mScreenMgr(nullptr), mAction(cAction_None), mEnabledTargets(0), mControls{} {}
BoxCursorMgr::~BoxCursorMgr() = default;
// action is the cursor movement or decision to process on the next update.
void BoxCursorMgr::setAction(Action action) { mAction = action; }
}
