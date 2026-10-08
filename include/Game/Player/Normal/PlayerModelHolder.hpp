#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>

namespace al {
class LiveActor;
}

/// Holds the models of a player's figures and switches between them.
class PlayerModelHolder {
public:
    PlayerModelHolder(u32 modelNum);

    virtual void change(s32 index);
    virtual void show();
    virtual void hide();
    virtual bool isHidden() const;
    virtual void showSilhouette();
    virtual void hideSilhouette();
    virtual bool isSilhouetteHidden() const;
    virtual void showShadow();
    virtual void hideShadow();
    virtual bool isShadowHidden() const;
    virtual void validateMash();
    virtual void invalidateMash();
    virtual bool isMash() const;

    void appear();
    void kill();
    void startInvincible();
    void setInvincibleColor(const sead::Color4f& rColor);
    void endInvincible();
    void showFur();
    void hideFur();
    void resetSkirtDynamics();
    void resetTailDynamics();
    void setShadowLength(f32 length);

    al::LiveActor* getCurrentModel() const { return mModels[mCurrentIndex]; }
    al::LiveActor* getModel(s32 index) const { return mModels[index]; }
    f32 getShadowLength() const { return mShadowLength; }

private:
    u8 _8[0x28 - 0x8];
    al::LiveActor** mModels;  // 0x28
    s32 mCurrentIndex;  // 0x30
    u8 _34[0x54 - 0x34];
    f32 mShadowLength;  // 0x54
};
