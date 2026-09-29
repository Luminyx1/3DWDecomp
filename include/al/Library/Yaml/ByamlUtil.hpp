#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;

bool tryGetByamlU8(u8*, const ByamlIter&, const char*);

bool tryGetByamlS32(s32*, const ByamlIter&, const char*);

bool tryGetByamlV3f(sead::Vector3f*, const ByamlIter&, const char*);

bool tryGetByamlU16(u16*, const ByamlIter&, const char*);
bool tryGetByamlS16(s16*, const ByamlIter&, const char*);
bool tryGetByamlV2f(sead::Vector2f*, const ByamlIter&, const char*);
bool tryGetByamlColor(sead::Color4f*, const ByamlIter&, const char*);
bool tryGetByamlF32(f32*, const ByamlIter&, const char*);
s32 tryGetByamlKeyIntOrZero(const ByamlIter&, const char*);
bool tryGetByamlKeyBoolOrFalse(const ByamlIter&, const char*);
const char* tryGetByamlKeyStringOrNULL(const ByamlIter&, const char*);
}  // namespace al
