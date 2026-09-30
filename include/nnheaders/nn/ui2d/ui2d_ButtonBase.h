#pragma once

#include <nn/types.h>

namespace nn::ui2d {

class ButtonBase {
public:
    enum State { cState_Off, cState_OnStart, cState_OffStart, cState_On,
                 cState_DownStart, cState_Down, cState_CancelStart };
    enum Action { cAction_On, cAction_Off, cAction_Down, cAction_Cancel };
    struct ActionQueue {
        void PushWithOmit(Action action);
        void Pop();
        bool HasDownAction() const;
        Action actions[4] = {};
        s32 count = 0;
    };

    ButtonBase();
    virtual ~ButtonBase();
    virtual void On();
    virtual void Off();
    virtual void Down();
    virtual void Cancel();
    virtual void ForceOff();
    virtual void ForceOn();
    virtual void ForceDown();
    virtual void Update();
    virtual void SetActive(bool active);
    virtual bool ProcessOn();
    virtual bool ProcessOff();
    virtual bool ProcessDown();
    virtual bool ProcessCancel();
    virtual bool UpdateOn();
    virtual bool UpdateOff();
    virtual bool UpdateDown();
    virtual bool UpdateCancel();
    virtual void StartOn();
    virtual void StartOff();
    virtual void StartDown();
    virtual void StartCancel();
    virtual void FinishOn();
    virtual void FinishOff();
    virtual void FinishDown();
    virtual void FinishCancel();
    virtual void ChangeState(State state);
    virtual void ForceChangeState(State state);

    void ProcessActionFromQueue();
    bool IsDowning() const;

    State mState;
    u32 mFlags;
    ActionQueue mActions;
};
static_assert(sizeof(ButtonBase) == 0x28, "ButtonBase size");

}  // namespace nn::ui2d
