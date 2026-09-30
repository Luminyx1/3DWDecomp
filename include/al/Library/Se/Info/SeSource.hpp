#pragma once

#include <basis/seadTypes.h>
#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace al {
class AcLSoundHandle;
class AudioSystemInfo;
class SeadAudio3DActorWrapper;
class SeadAudioActorWrapper;
class SeSourcePose3D;
class SeSourcePose3DMtxBase;
class SoundStartInfo;

class SeSource {
public:
    SeSource(const sead::SafeString& rName);

    virtual void init() = 0;
    virtual void update() = 0;
    virtual bool startSound(AcLSoundHandle* pHandle, u32 soundId, const SoundStartInfo* pStartInfo) = 0;
    virtual bool isPlayingSound() const = 0;
    virtual s32 getPriority() const = 0;
    virtual void resetVelocity() = 0;
    virtual bool isCalc3D() const { return false; }
    virtual const sead::Vector3f* getPosition() const = 0;

    const sead::SafeString& getName() const { return mName; }
    f32 getVolume() const { return mVolume; }
    void setVolume(f32 volume) { mVolume = volume; }

private:
    sead::SafeString mName;
    f32 mVolume = 1.0f;
};
static_assert(sizeof(SeSource) == 0x20);

class SeSource3D : public SeSource {
public:
    SeSource3D(const sead::SafeString& rName, SeSourcePose3D* pPose, AudioSystemInfo* pInfo);

    void init() override;
    void update() override;
    bool startSound(AcLSoundHandle* pHandle, u32 soundId, const SoundStartInfo* pStartInfo) override;
    bool isPlayingSound() const override;
    s32 getPriority() const override { return mPriority; }
    void resetVelocity() override;
    bool isCalc3D() const override { return true; }
    const sead::Vector3f* getPosition() const override;
    virtual ~SeSource3D() {}

    virtual void calcPositionInitialize() = 0;
    virtual void calcPositionDynamic() = 0;
    virtual const sead::Vector3f* calcPosition(const sead::Vector3f& rListenerPos) = 0;

    void syncPosition();

protected:
    SeadAudio3DActorWrapper* mActor = nullptr;
    SeSourcePose3D* mPose;
    s32 mPriority = 0;
    AudioSystemInfo* mInfo;
};
static_assert(sizeof(SeSource3D) == 0x40);

class SeSource3DCircle : public SeSource3D {
public:
    SeSource3DCircle(SeSourcePose3DMtxBase* pPose, const f32* pRadius, AudioSystemInfo* pInfo, bool isVertical);

    void calcPositionInitialize() override;
    void calcPositionDynamic() override;
    const sead::Vector3f* calcPosition(const sead::Vector3f& rListenerPos) override;

private:
    SeSourcePose3DMtxBase* mMtxPose;
    const f32* mRadius;
    sead::Vector3f mPos = {0.0f, 0.0f, 0.0f};
    sead::Matrix34f mInvMtx;
    bool mIsVertical;
};
static_assert(sizeof(SeSource3DCircle) == 0x90);

class SeSource3DLine : public SeSource3D {
public:
    SeSource3DLine(SeSourcePose3DMtxBase* pPose, const sead::Vector3f* pLine, AudioSystemInfo* pInfo);

    void calcPositionInitialize() override;
    void calcPositionDynamic() override;
    const sead::Vector3f* calcPosition(const sead::Vector3f& rListenerPos) override;

private:
    SeSourcePose3DMtxBase* mMtxPose;
    const sead::Vector3f* mLine;
    sead::Vector3f mPos = {0.0f, 0.0f, 0.0f};
    f32 mLength = 0.0f;
    sead::Vector3f mDir = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mEndPos = {0.0f, 0.0f, 0.0f};
};
static_assert(sizeof(SeSource3DLine) == 0x78);

class SeSource3DPlaneRect : public SeSource3D {
public:
    SeSource3DPlaneRect(SeSourcePose3DMtxBase* pPose, const sead::BoundBox2f* pRect, AudioSystemInfo* pInfo);

    void calcPositionInitialize() override;
    void calcPositionDynamic() override;
    const sead::Vector3f* calcPosition(const sead::Vector3f& rListenerPos) override;

private:
    SeSourcePose3DMtxBase* mMtxPose;
    const sead::BoundBox2f* mRect;
    sead::Vector3f mPos = {0.0f, 0.0f, 0.0f};
    sead::Matrix34f mInvMtx;
};
static_assert(sizeof(SeSource3DPlaneRect) == 0x90);

class SeSource3DPoint : public SeSource3D {
public:
    SeSource3DPoint(SeSourcePose3D* pPose, AudioSystemInfo* pInfo);

    const sead::Vector3f* calcPosition(const sead::Vector3f& rListenerPos) override;
    void calcPositionDynamic() override;
    void calcPositionInitialize() override {}
};
static_assert(sizeof(SeSource3DPoint) == 0x40);

class SeSource3DRing : public SeSource3D {
public:
    SeSource3DRing(SeSourcePose3DMtxBase* pPose, const f32* pRadius, AudioSystemInfo* pInfo);

    void calcPositionInitialize() override;
    void calcPositionDynamic() override;
    const sead::Vector3f* calcPosition(const sead::Vector3f& rListenerPos) override;

private:
    SeSourcePose3DMtxBase* mMtxPose;
    const f32* mRadius;
    sead::Vector3f mPos = {0.0f, 0.0f, 0.0f};
    sead::Matrix34f mInvMtx;
};
static_assert(sizeof(SeSource3DRing) == 0x90);

class SeSource3DSphere : public SeSource3D {
public:
    SeSource3DSphere(SeSourcePose3D* pPose, const f32* pRadius, AudioSystemInfo* pInfo);

    void calcPositionInitialize() override;
    void calcPositionDynamic() override;
    const sead::Vector3f* calcPosition(const sead::Vector3f& rListenerPos) override;

private:
    const f32* mRadius;
    sead::Vector3f mPos = {0.0f, 0.0f, 0.0f};
};
static_assert(sizeof(SeSource3DSphere) == 0x58);

class SeSourceAmbient : public SeSource {
public:
    SeSourceAmbient(AudioSystemInfo* pInfo);

    bool startSound(AcLSoundHandle* pHandle, u32 soundId, const SoundStartInfo* pStartInfo) override;
    bool isPlayingSound() const override;
    s32 getPriority() const override;
    const sead::Vector3f* getPosition() const override;
    void init() override;
    void update() override;
    void resetVelocity() override;

private:
    SeadAudioActorWrapper* mActor = nullptr;
    AudioSystemInfo* mInfo;
};
static_assert(sizeof(SeSourceAmbient) == 0x30);
}  // namespace al
