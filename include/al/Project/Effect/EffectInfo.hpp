#pragma once

#include <gfx/seadColor.h>

#include <basis/seadTypes.h>

namespace al {
struct EffectInfo {
    EffectInfo();

    const char* mName;
    s32 mEmitInfoNum;
    void* mEmitInfos;
    s32 _18;
    const char* mJointName;
    void* _28;
    void* _30;
    void* _38;
    f32 _40;
    f32 _44;
    f32 _48;
    f32 mFarClipDistance;
    s32 _50;
    s32 _54;
    sead::Color4f mColor;
    u32 _68;
    u32 _6c;
    bool _70;
    bool _71;
    bool _72;
    u8 _73[0x10];
    bool mIsSnapshotCameraMode;
    s32 _88;
    void* _90;
    s32 _98;
};

static_assert(sizeof(EffectInfo) == 0xa0);

struct EffectNamedMtxList {
    const char** mNames;
    s32 mNum;
};

struct EffectUserInfo {
    EffectUserInfo();

    EffectInfo* tryFindEffectInfo(const char* pName) const;

    const char* mName;
    s32 mEffectNum;
    EffectInfo* mEffects;
    s32 _18;
    void* _20;
    EffectNamedMtxList* mNamedMtxList;
};

static_assert(sizeof(EffectUserInfo) == 0x30);

class EffectDataBase {
public:
    EffectDataBase(const char* pArchiveName);

    s32 mUserNum;
    EffectUserInfo** mUsers;
    s32 _10;
    void* _18;
};
}  // namespace al
