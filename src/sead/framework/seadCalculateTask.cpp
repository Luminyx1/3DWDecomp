#include "framework/seadCalculateTask.h"

namespace sead
{
CalculateTask::CalculateTask(const TaskConstructArg& rArg) : TaskBase(rArg)
{
    mCalcNode.bind(sead::Delegate<CalculateTask>{this, &CalculateTask::calc}, "CalculateTask");
}

CalculateTask::CalculateTask(const TaskConstructArg& rArg, const char* pName)
    : TaskBase(rArg, pName)
{
    mCalcNode.bind(sead::Delegate<CalculateTask>{this, &CalculateTask::calc}, pName);
}

CalculateTask::~CalculateTask() = default;

void CalculateTask::calc() {}
}  // namespace sead
