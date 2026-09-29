#pragma once

#include <basis/seadTypes.h>
#include <prim/seadEnum.h>
#include <prim/seadStringId.h>

namespace erepo {

using StringId = sead::StringIdBase<4096, 32, 'sead', u32>;

SEAD_ENUM(EPlayStyle, Console, Handheld, Unknown)
SEAD_ENUM(EControllerStyle, Handheld, Dual, ExtGrip, FullKey, Unknown)
SEAD_ENUM(ReporterType, cSystem, cGame)

}  // namespace erepo
