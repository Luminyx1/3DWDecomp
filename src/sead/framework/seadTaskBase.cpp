#include "framework/seadTaskBase.h"

namespace sead
{
TaskBase::CreateArg::CreateArg() = default;

TaskBase::CreateArg::CreateArg(const TaskClassID& rFactory) : factory(rFactory) {}
}  // namespace sead
