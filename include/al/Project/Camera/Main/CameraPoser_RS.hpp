#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/HostIO/IUseName.hpp"
#include "Library/Nerve/IUseNerve.hpp"
#include "Library/Rail/IUseRail.hpp"
#include "Project/AreaObj/IUseAreaObj.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"
#include "Project/Collision/IUseCollision.hpp"

namespace sead {
class LookAtCamera;
}

namespace al {
class ByamlIter;
class CameraAngleCtrlInfo;
class CameraObjectRequestInfo;
class CameraPoserFlag;
class CameraPoserSceneInfo_RS;
class CameraStartInfo;
class CameraTurnInfo;
class CameraVerticalAbsorber;
class CameraViewInfo;
struct PlacementInfo;

/// Base class of the cameras that calculate a camera pose each frame.
class CameraPoser_RS : public IUseAreaObj,
                       public IUseAudioKeeper,
                       public IUseCollision,
                       public IUseName,
                       public IUseNerve,
                       public IUseRail {
public:
    CameraPoser_RS(const char* pName);

    AreaObjDirector* getAreaObjDirector() const override;
    virtual void init();
    virtual void initByPlacementObj(const PlacementInfo& rInfo);
    virtual void endInit();
    virtual void start(const CameraStartInfo& rInfo);
    virtual void update();
    virtual void end();
    virtual void loadParam(const ByamlIter& rIter);
    virtual void makeLookAtCamera(sead::LookAtCamera* pCamera) const;
    virtual bool receiveRequestFromObject(const CameraObjectRequestInfo& rInfo);
    virtual bool isZooming() const;
    virtual bool isEnableRotateByPad() const;
    virtual void reset();
    virtual void startSnapShotMode();
    virtual void endSnapShotMode();
    const char* getName() const override;
    CollisionDirector* getCollisionDirector() const override;
    NerveKeeper* getNerveKeeper() const override;
    AudioKeeper* getAudioKeeper() const override;
    RailRider* getRailRider() const override;
    virtual void load(const ByamlIter& rIter);
    virtual void movement();
    virtual void calcCameraPose(sead::LookAtCamera* pCamera) const;
    virtual void startCameraReset(bool isResetAngleV);
    virtual f32 getVerticalAngle();
    virtual void requestTurnToDirection(const CameraTurnInfo* pInfo);

    const char* mPoserName;                     // _30
    void* _38;                                  // _38
    void* _40;                                  // _40
    void* _48;                                  // _48
    f32 _50;                                    // _50
    sead::Vector3f mCameraUp;                   // _54
    f32 mFovyDegree;                            // _60
    f32 _64;                                    // _64
    f32 _68;                                    // _68
    sead::Matrix34f mViewMtx;                   // _6C
    bool _9C;                                   // _9C
    CameraPoserSceneInfo_RS* mSceneInfo;        // _A0
    CameraViewInfo* mViewInfo;                  // _A8
    CameraPoserFlag* mPoserFlag;                // _B0
    CameraVerticalAbsorber* mVerticalAbsorber;  // _B8
    CameraAngleCtrlInfo* mAngleCtrlInfo;        // _C0
    void* _C8;                                  // _C8
    void* _D0;                                  // _D0
    void* _D8;                                  // _D8
    void* _E0;                                  // _E0
    void* _E8;                                  // _E8
    void* _F0;                                  // _F0
    void* _F8;                                  // _F8
    void* _100;                                 // _100
    void* _108;                                 // _108
    void* _110;                                 // _110
    void* _118;                                 // _118
    void* _120;                                 // _120
    void* _128;                                 // _128
    void* _130;                                 // _130
    void* _138;                                 // _138
    bool _140;                                  // _140
    bool _141;                                  // _141
};
}  // namespace al
