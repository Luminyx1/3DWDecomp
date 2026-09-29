#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
    class LiveActor;

    void preScaleMtx(sead::Matrix34f*, const sead::Vector3f&);

    void calcMxtInvertOrtho(sead::Matrix34f*, const sead::Matrix34f&);

    void calcMtxLocalTrans(sead::Vector3f*, const sead::Matrix34f&, const sead::Vector3f&);

    void makeMtxUpFrontPos(sead::Matrix34f *, const sead::Vector3f &, const sead::Vector3f &, const sead::Vector3f &);

    void updatePoseMtx(LiveActor *, const sead::Matrix34f *);

    void calcMtxScale(sead::Vector3f*, const sead::Matrix34f&);
    void makeMtxFrontNoSupportPos(sead::Matrix34f*, const sead::Vector3f&, const sead::Vector3f&);
};
