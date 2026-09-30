/**
 * @file ResSceneAnim.h
 * @brief Resource file for scene animations.
 */

#pragma once

#include <nn/types.h>
#include <nn/g3d/g3d_Resources.h>

namespace nn {
namespace g3d {
class ResLightAnim;
class ResFogAnim;
class BindFuncTable;

class ResSceneAnim {
public:
    // table resolves light and fog function names across this scene.
    BindResult Bind(nn::g3d::BindFuncTable const& table);
    void Release();
    void Reset();

    char mMagic[4];                      // _0
    s32 mBlockOffset;                    // _4
    u64 mNameOffset;                     // _8
    u64 mPathOffset;                     // _10
    u64 mCameraAnimOffset;               // _18
    u64 mCameraAnimDictOffset;           // _20
    nn::g3d::ResLightAnim* mLightAnims;  // _28
    u64 mLightAnimDictOffset;            // _30
    nn::g3d::ResFogAnim* mFogAnims;      // _38
    u64 mFogAnimDictOffset;              // _40
    u64 mUserDataOffset;                 // _48
    u64 mUserDataDictOffset;             // _50
    u16 mUserDataCount;                  // _58
    u16 mCameraAnimCount;                // _5A
    u16 mLightAnimCount;                 // _5C
    u16 mFogAnimCount;                   // _5E
};
}  // namespace g3d
}  // namespace nn
