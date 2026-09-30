#include "Library/Se/Info/SeAudioInfo.hpp"

#include "Library/Se/Function/SeDbFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"

namespace {
al::SeSoundSourceInfo* createSoundSourceInfo(const al::ByamlIter& rIter) {
    const char* name = nullptr;
    al::SeSoundSourceInfo* sourceInfo = nullptr;
    if (rIter.tryGetStringByKey(&name, "Name")) {
        if (alSeFunction::isSoundSourceAmbient(name)) {
            sourceInfo = new al::SeSoundSourceInfoAmbient(name);
        } else if (alSeFunction::isSoundSource3DPoint(name)) {
            sourceInfo = new al::SeSoundSourceInfo3DPoint(name);
        } else if (alSeFunction::isSoundSource3DSphere(name)) {
            al::SeSoundSourceInfo3DSphere* sphere = new al::SeSoundSourceInfo3DSphere(name);
            rIter.tryGetFloatByKey(&sphere->mRadius, "Radius");
            sourceInfo = sphere;
        } else if (alSeFunction::isSoundSource3DVector(name)) {
            al::SeSoundSourceInfo3DVector* vector = new al::SeSoundSourceInfo3DVector(name);
            al::ByamlIter vectorIter;
            rIter.tryGetIterByKey(&vectorIter, "Vector3f");
            if (!vectorIter.tryGetFloatByKey(&vector->mVector.x, "x")) {
                vectorIter.tryGetFloatByKey(&vector->mVector.x, "X");
            }
            if (!vectorIter.tryGetFloatByKey(&vector->mVector.y, "y")) {
                vectorIter.tryGetFloatByKey(&vector->mVector.y, "Y");
            }
            if (!vectorIter.tryGetFloatByKey(&vector->mVector.z, "z")) {
                vectorIter.tryGetFloatByKey(&vector->mVector.z, "Z");
            }
            sourceInfo = vector;
        } else if (alSeFunction::isSoundSource3DBox(name)) {
            al::SeSoundSourceInfo3DBox* box = new al::SeSoundSourceInfo3DBox(name);
            al::ByamlIter boxIter;
            rIter.tryGetIterByKey(&boxIter, "Box2f");
            boxIter.tryGetFloatByKey(&box->mMinX, "MinX");
            boxIter.tryGetFloatByKey(&box->mMinY, "MinY");
            boxIter.tryGetFloatByKey(&box->mMaxX, "MaxX");
            boxIter.tryGetFloatByKey(&box->mMaxY, "MaxY");
            sourceInfo = box;
        } else if (alSeFunction::isSoundSource3DRing(name)) {
            al::SeSoundSourceInfo3DRing* ring = new al::SeSoundSourceInfo3DRing(name);
            rIter.tryGetFloatByKey(&ring->mRadius, "Radius");
            sourceInfo = ring;
        } else if (alSeFunction::isSoundSource3DCircle(name)) {
            al::SeSoundSourceInfo3DCircle* circle = new al::SeSoundSourceInfo3DCircle(name);
            rIter.tryGetFloatByKey(&circle->mRadius, "Radius");
            if (!rIter.tryGetBoolByKey(&circle->mIsCircleRotated, "IsCircleRotated")) {
                circle->mIsCircleRotated = false;
            }
            sourceInfo = circle;
        } else {
            sourceInfo = nullptr;
        }
    }
    return sourceInfo;
}
}  // namespace

namespace al {
/**
 * Creates SE emitter information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
SeEmitterInfo* SeEmitterInfo::createInfo(const ByamlIter& rIter) {
    SeEmitterInfo* info = new SeEmitterInfo;
    rIter.tryGetStringByKey(&info->mName, "Name");
    if (!rIter.tryGetStringByKey(&info->mJointName, "JointName")) {
        info->mJointName = nullptr;
    }

    ByamlIter offsetIter;
    if (rIter.tryGetIterByKey(&offsetIter, "Offset")) {
        sead::Vector3f* offset = new sead::Vector3f;
        if (!offsetIter.tryGetFloatByKey(&offset->x, "x") && !offsetIter.tryGetFloatByKey(&offset->x, "X")) {
            offset->x = 0.0f;
        }
        if (!offsetIter.tryGetFloatByKey(&offset->y, "y") && !offsetIter.tryGetFloatByKey(&offset->y, "Y")) {
            offset->y = 0.0f;
        }
        if (!offsetIter.tryGetFloatByKey(&offset->z, "z") && !offsetIter.tryGetFloatByKey(&offset->z, "Z")) {
            offset->z = 0.0f;
        }
        info->mOffset = offset;
    } else {
        info->mOffset = nullptr;
    }

    ByamlIter sourceIter;
    if (rIter.tryGetIterByKey(&sourceIter, "SoundSource")) {
        info->mSoundSourceInfo = createSoundSourceInfo(sourceIter);
    } else {
        info->mSoundSourceInfo = alSeDbFunction::createDefaultSoundSourceInfo();
    }
    return info;
}

/**
 * Creates a copy of SE emitter information with copied names.
 * @param pInfo Information to copy.
 * @return Created information.
 */
SeEmitterInfo* SeEmitterInfo::duplicateInfo(const SeEmitterInfo* pInfo) {
    SeEmitterInfo* info = new SeEmitterInfo;
    info->mName = alSeDbFunction::createNameAreaAndCopy(pInfo->mName);
    info->mJointName = alSeDbFunction::createNameAreaAndCopy(pInfo->mJointName);
    if (pInfo->mOffset != nullptr) {
        __builtin_trap();
    }

    const SeSoundSourceInfo* srcInfo = pInfo->mSoundSourceInfo;
    SeSoundSourceInfo* sourceInfo = nullptr;
    if (srcInfo != nullptr && srcInfo->mName != nullptr) {
        const char* name = alSeDbFunction::createNameAreaAndCopy(srcInfo->mName);
        if (name != nullptr) {
            if (alSeFunction::isSoundSourceAmbient(name)) {
                sourceInfo = new SeSoundSourceInfoAmbient(name);
            } else if (alSeFunction::isSoundSource3DPoint(name)) {
                sourceInfo = new SeSoundSourceInfo3DPoint(name);
            } else if (alSeFunction::isSoundSource3DSphere(name)) {
                sourceInfo = new SeSoundSourceInfo3DSphere(*static_cast<const SeSoundSourceInfo3DSphere*>(srcInfo));
            } else if (alSeFunction::isSoundSource3DVector(name)) {
                sourceInfo = new SeSoundSourceInfo3DVector(*static_cast<const SeSoundSourceInfo3DVector*>(srcInfo));
            } else if (alSeFunction::isSoundSource3DBox(name)) {
                sourceInfo = new SeSoundSourceInfo3DBox(*static_cast<const SeSoundSourceInfo3DBox*>(srcInfo));
            } else if (alSeFunction::isSoundSource3DRing(name)) {
                sourceInfo = new SeSoundSourceInfo3DRing(*static_cast<const SeSoundSourceInfo3DRing*>(srcInfo));
            }
        }
    }
    info->mSoundSourceInfo = sourceInfo;
    return info;
}

/**
 * Compares two SE emitter information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeEmitterInfo::compareInfo(const SeEmitterInfo* pA, const SeEmitterInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Does nothing.
 */
void SeSoundSourceInfo::dummy() {}

}  // namespace al
