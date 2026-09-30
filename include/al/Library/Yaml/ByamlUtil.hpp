#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadBoundBox.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;

bool tryGetByamlU8(u8* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlU16(u16* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlS16(s16* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlS32(s32* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlU32(u32* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlF32(f32* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlV2f(sead::Vector2f* pValue, const ByamlIter& rIter);
bool tryGetByamlV3f(sead::Vector3f* pValue, const ByamlIter& rIter);
bool tryGetByamlV4f(sead::Vector4f* pValue, const ByamlIter& rIter);
bool tryGetByamlMinMax(sead::Vector2f* pValue, const ByamlIter& rIter);
bool tryGetByamlScale(sead::Vector3f* pValue, const ByamlIter& rIter);
bool tryGetByamlV3s32(sead::Vector3i* pValue, const ByamlIter& rIter);
bool tryGetByamlBox3f(sead::BoundBox3f* pValue, const ByamlIter& rIter);
bool tryGetByamlV3f(sead::Vector3f* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlV2f(sead::Vector2f* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlV4f(sead::Vector4f* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlMinMax(sead::Vector2f* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlScale(sead::Vector3f* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlV3s32(sead::Vector3i* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlBox3f(sead::BoundBox3f* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlString(const char** pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlColor(sead::Color4f* pValue, const ByamlIter& rIter);
bool tryGetByamlColor(sead::Color4f* pValue, const ByamlIter& rIter, const char* pKey);
bool tryGetByamlBool(bool* pValue, const ByamlIter& rIter, const char* pKey);
const char* getByamlKeyString(const ByamlIter& rIter, const char* pKey);
s32 getByamlKeyInt(const ByamlIter& rIter, const char* pKey);
f32 getByamlKeyFloat(const ByamlIter& rIter, const char* pKey);
bool getByamlKeyBool(const ByamlIter& rIter, const char* pKey);
const char* tryGetByamlKeyStringOrNULL(const ByamlIter& rIter, const char* pKey);
s32 tryGetByamlKeyIntOrZero(const ByamlIter& rIter, const char* pKey);
f32 tryGetByamlKeyFloatOrZero(const ByamlIter& rIter, const char* pKey);
bool tryGetByamlKeyBoolOrFalse(const ByamlIter& rIter, const char* pKey);
bool tryGetByamlIterByKey(ByamlIter* pIter, const ByamlIter& rIter, const char* pKey);
void getByamlIterByKey(ByamlIter* pIter, const ByamlIter& rIter, const char* pKey);
void getByamlIterByIndex(ByamlIter* pIter, const ByamlIter& rIter, s32 index);
bool isTypeBoolByIndex(const ByamlIter& rIter, s32 index);
bool isTypeBoolByKey(const ByamlIter& rIter, const char* pKey);
bool isTypeIntByIndex(const ByamlIter& rIter, s32 index);
bool isTypeIntByKey(const ByamlIter& rIter, const char* pKey);
bool isTypeFloatByIndex(const ByamlIter& rIter, s32 index);
bool isTypeFloatByKey(const ByamlIter& rIter, const char* pKey);
bool isTypeStringByIndex(const ByamlIter& rIter, s32 index);
bool isTypeStringByKey(const ByamlIter& rIter, const char* pKey);
bool isTypeArrayByIndex(const ByamlIter& rIter, s32 index);
bool isTypeArrayByKey(const ByamlIter& rIter, const char* pKey);
bool isTypeHashByIndex(const ByamlIter& rIter, s32 index);
bool isTypeHashByKey(const ByamlIter& rIter, const char* pKey);
bool tryGetByamlKeyAndIntByIndex(const char** pKey, s32* pValue, const ByamlIter& rIter, s32 index);
s32 getByamlIterDataNum(const ByamlIter& rIter);
void printByamlIter(const u8* pData);
void printByamlIter(const ByamlIter& rIter);
}  // namespace al
