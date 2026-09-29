#pragma once

#include "controller/seadController.h"

namespace sead
{
class NinDebugController : public Controller
{
    SEAD_RTTI_OVERRIDE(NinDebugController, Controller)

public:
    explicit NinDebugController(ControllerMgr* pMgr);

    bool isConnected() const override { return mIsConnected; }

protected:
    void calcImpl_() override;

private:
    bool mIsConnected;
};

}  // namespace sead
