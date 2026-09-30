#include "Library/Camera/CameraSubTargetBase.hpp"

#include "Project/Camera/CameraSubTargetTurnParam.hpp"

namespace al {
namespace {
const CameraSubTargetTurnParam sDefaultTurnParam;
}  // namespace

CameraSubTargetBase::CameraSubTargetBase() : mTurnParam(&sDefaultTurnParam) {}

}  // namespace al
