#include <nn/atk/atk_Task.h>

namespace nn::atk::detail {
Task::Task() {
    nn::os::InitializeEvent(&mCompletionEvent, false, nn::os::EventClearMode_ManualClear);
    mState = 0;
    mId = 0;
    nn::os::SignalEvent(&mCompletionEvent);
}

Task::~Task() { nn::os::FinalizeEvent(&mCompletionEvent); }
}
