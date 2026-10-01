#include "Project/Camera/Holder/CameraShaker_RS.hpp"

#include <math/seadMathCalcCommon.h>

#include "Project/Base/StringUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(CameraShaker_RS, Wait)
NERVE_DECL(CameraShaker_RS, ShakeLoop)
NERVE_DECL(CameraShaker_RS, Shake)
NERVE_DECL(CameraShaker_RS, ShakeMultiple)

NERVES_MAKE_NOSTRUCT(CameraShaker_RS, Wait, ShakeLoop, Shake, ShakeMultiple)

const CameraShakeParam sShakeParams[] = {
    {"GigaStomp", 20, 3.0f, 0.003f, 0, 1.0f, true, false, false, false},
    {"微弱", 15, 2.5f, 0.0015f, 0, 1.0f, false, false, false, false},
    {"微弱[短]", 10, 2.0f, 0.0008f, 0, 1.0f, false, false, false, false},
    {"弱", 25, 2.5f, 0.0025f, 0, 1.0f, false, false, false, false},
    {"Giga中", 20, 2.5f, 0.008f, 0, 1.0f, true, false, false, false},
    {"中", 25, 2.5f, 0.004f, 0, 1.0f, false, false, false, false},
    {"Giga強", 20, 3.0f, 0.01f, 0, 1.0f, true, true, false, false},
    {"強", 30, 3.0f, 0.008f, 0, 1.0f, false, false, false, false},
    {"Giga最強", 40, 3.5f, 0.015f, 0, 2.0f, true, true, false, false},
    {"最強", 45, 3.5f, 0.015f, 0, 1.0f, false, false, false, false},
    {"超最強", 45, 3.5f, 0.05f, 0, 1.0f, false, false, false, false},
    {"長い微弱", 60, 6.0f, 0.0025f, 0, 1.0f, false, false, false, false},
    {"長い弱", 60, 6.0f, 0.004f, 0, 1.0f, false, false, false, false},
    {"船内振動", 100, 5.0f, 0.0005f, 0, 1.0f, false, false, false, false},
    {"弱[縦]", 25, 2.5f, 0.004f, 1, 1.0f, false, false, false, false},
    {"Laser", 40, 15.0f, 0.005f, 0, 1.0f, false, false, false, false},
    {"DMLand", 40, 3.0f, 0.01f, 0, 2.0f, true, true, false, false},
};

const CameraShakeParam sLoopShakeParams[] = {
    {nullptr, 0, 0.0f, 0.0f, 0, 0.0f, false, false, false, false},
    {"弱", -1, 7.5f, 0.0007f, 0, 1.0f, false, false, false, false},
    {"強", -1, 2.0f, 0.001f, 0, 1.0f, false, false, false, false},
};

inline bool isWeakerShake(const CameraShakeParam* pParam, const CameraShakeParam* pCurrent) {
    if (pCurrent->power < pParam->power) {
        return false;
    }

    if (pParam->power < pCurrent->power) {
        return true;
    }

    if (pCurrent->steps < 0) {
        return false;
    }

    if (pCurrent->steps < pParam->steps) {
        return false;
    }

    if (pParam->steps < pCurrent->steps) {
        return true;
    }

    if (pParam->direction < pCurrent->direction) {
        return false;
    }

    if (pCurrent->direction < pParam->direction) {
        return true;
    }

    return pCurrent->speed < pParam->speed;
}
}  // namespace

namespace al {

CameraShaker_RS::CameraShaker_RS() : NerveExecutor("カメラ振動") {
    initNerve(&NrvCameraShaker_RSWait, 0);
    mCustomParam.name = "NULL";
    mCustomParam.steps = 0;
    mCustomParam.speed = 0.0f;
    mCustomParam.power = 0.0f;
    mCustomParam.direction = 0;
}

void CameraShaker_RS::update(const char* pLoopShakeName, bool isPaused) {
    if (isPaused) {
        return;
    }

    if (pLoopShakeName != nullptr) {
        if (isEqualString(pLoopShakeName, "弱")) {
            mLoopParam = &sLoopShakeParams[1];
        } else {
            mLoopParam = isEqualString(pLoopShakeName, "強") ? &sLoopShakeParams[2] : nullptr;
        }

        if (isNerve(this, &NrvCameraShaker_RSWait)) {
            setNerve(this, &NrvCameraShaker_RSShakeLoop);
        }
    } else {
        mLoopParam = nullptr;

        if (isNerve(this, &NrvCameraShaker_RSShakeLoop)) {
            setNerve(this, &NrvCameraShaker_RSWait);
        }
    }

    updateNerve();
}

void CameraShaker_RS::startShakeByAction(const char* pShakeName, const char* pActionName,
                                         const char* pActorName, s32 steps) {
    startShakeByName(pShakeName, steps);
}

void CameraShaker_RS::startShakeByName(const char* pShakeName, s32 steps) {
    s32 index;

    if (isEqualString(pShakeName, "GigaStomp")) {
        index = 0;
    } else if (isEqualString(pShakeName, "微弱")) {
        index = 1;
    } else if (isEqualString(pShakeName, "微弱[短]")) {
        index = 2;
    } else if (isEqualString(pShakeName, "弱")) {
        index = 3;
    } else if (isEqualString(pShakeName, "Giga中")) {
        index = 4;
    } else if (isEqualString(pShakeName, "中")) {
        index = 5;
    } else if (isEqualString(pShakeName, "Giga強")) {
        index = 6;
    } else if (isEqualString(pShakeName, "強")) {
        index = 7;
    } else if (isEqualString(pShakeName, "Giga最強")) {
        index = 8;
    } else if (isEqualString(pShakeName, "最強")) {
        index = 9;
    } else if (isEqualString(pShakeName, "超最強")) {
        index = 10;
    } else if (isEqualString(pShakeName, "長い微弱")) {
        index = 11;
    } else if (isEqualString(pShakeName, "長い弱")) {
        index = 12;
    } else if (isEqualString(pShakeName, "船内振動")) {
        index = 13;
    } else if (isEqualString(pShakeName, "弱[縦]")) {
        index = 14;
    } else if (isEqualString(pShakeName, "Laser")) {
        index = 15;
    } else if (isEqualString(pShakeName, "DMLand")) {
        index = 16;
    } else {
        index = -1;
    }

    startShakeByIndex(index, steps);
}

void CameraShaker_RS::startShakeByHitReaction(const char* pShakeName, const char* pReactionName,
                                              const char* pActorName, s32 steps) {
    startShakeByName(pShakeName, steps);
}

void CameraShaker_RS::cancelShake() {
    setNerve(this, &NrvCameraShaker_RSWait);
}

void CameraShaker_RS::exeWait() {
    if (isFirstStep(this)) {
        mRoll = 0.0f;
        mShakeParam = nullptr;
        mLoopParam = nullptr;
    }

    mOffset = {0.0f, 0.0f};
}

void CameraShaker_RS::exeShake() {
    if (isGreaterEqualStep(this, mShakeParam->steps)) {
        if (mLoopParam != nullptr) {
            setNerve(this, &NrvCameraShaker_RSShakeLoop);
            return;
        }

        mOffset = {0.0f, 0.0f};
        mShakeParam = nullptr;
        setNerve(this, &NrvCameraShaker_RSWait);
        return;
    }

    f32 speed = mShakeParam->speed * 360.0f / mShakeParam->steps;
    f32 wave = cosf(sead::Mathf::deg2rad(speed * getNerveStep(this)));
    f32 power =
        mShakeParam->power * (mShakeParam->steps - getNerveStep(this)) / mShakeParam->steps;
    f32 value = wave * power;
    mOffset.x = value;
    mOffset.y = value;

    if (mShakeParam->direction == 1) {
        mOffset.x = 0.0f;
    }
}

void CameraShaker_RS::exeShakeMultiple() {
    if (isGreaterEqualStep(this, mShakeParam->steps)) {
        if (mLoopParam != nullptr) {
            setNerve(this, &NrvCameraShaker_RSShakeLoop);
            return;
        }

        mOffset = {0.0f, 0.0f};
        mShakeParam = nullptr;
        setNerve(this, &NrvCameraShaker_RSWait);
        return;
    }

    f32 speed = mShakeParam->speed * 360.0f / mShakeParam->steps;
    f32 angle = sead::Mathf::deg2rad(speed * getNerveStep(this));
    f32 waveCos = cosf(angle);
    f32 waveSin = sinf(angle);
    f32 power =
        mShakeParam->power * (mShakeParam->steps - getNerveStep(this)) / mShakeParam->steps;
    f32 valueCos = waveCos * power;
    f32 valueSin = waveSin * power;
    mOffset.x = valueCos + valueSin;
    mOffset.y = valueCos;
    mOffset.y = valueCos + valueSin * mShakeParam->multipleRate;
    mRoll = waveSin * mShakeParam->power * 100.0f;

    if (mShakeParam->direction == 1) {
        mOffset.x = 0.0f;
    }
}

void CameraShaker_RS::exeShakeLoop() {
    s32 step = getNerveStep(this);
    f32 angle = step > 0 ? step / mLoopParam->speed * sead::Mathf::pi2() : 0.0f;
    f32 value = mLoopParam->power * cosf(angle);
    mOffset.x = value;
    mOffset.y = value;

    if (mLoopParam->direction == 1) {
        mOffset.x = 0.0f;
    }
}

void CameraShaker_RS::startShakeByIndex(s32 index, s32 steps) {
    const CameraShakeParam* param = &sShakeParams[index];

    if (mShakeParam != nullptr) {
        if (isWeakerShake(param, mShakeParam)) {
            return;
        }

        if (mShakeParam->isKeepMultiple && isNerve(this, &NrvCameraShaker_RSShakeMultiple)) {
            return;
        }
    }

    mShakeParam = param;

    if (steps >= 1) {
        f32 rate = (f32)steps / param->steps;
        mCustomParam.name = param->name;
        mCustomParam.speed = rate * param->speed;
        mCustomParam.power = param->power;
        mCustomParam.direction = param->direction;
        mCustomParam.steps = steps;
        mShakeParam = &mCustomParam;
    }

    if (sShakeParams[index].isMultiple) {
        setNerve(this, &NrvCameraShaker_RSShakeMultiple);
    } else {
        setNerve(this, &NrvCameraShaker_RSShake);
    }
}

}  // namespace al
