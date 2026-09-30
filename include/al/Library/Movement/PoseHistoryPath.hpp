#pragma once

#include <container/seadRingBuffer.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {

class PoseHistoryPath {
public:
    struct PoseInfo {
        PoseInfo();
        PoseInfo(const sead::Quatf& rQuat, const sead::Vector3f& rTrans, const char* pName,
                 f32 distance);

        sead::Quatf quat;
        sead::Vector3f trans;
        const char* name;
        f32 distance;
        bool isKeep;
    };

    PoseHistoryPath(s32 maxHistory);

    void clear();
    void addHistory(const sead::Quatf& rQuat, const sead::Vector3f& rTrans, f32 minDistance);
    void addHistory(const sead::Quatf& rQuat, const sead::Vector3f& rTrans, const char* pName,
                    f32 minDistance);
    void calcPoseAndTrans(sead::Quatf* pQuat, sead::Vector3f* pTrans, f32 distance) const;
    void calcPoseAndTrans(sead::Quatf* pQuat, sead::Vector3f* pTrans, const char** pName,
                          f32 distance) const;

private:
    sead::RingBuffer<PoseInfo> mHistory;
};

static_assert(sizeof(PoseHistoryPath::PoseInfo) == 0x30);
static_assert(sizeof(PoseHistoryPath) == 0x18);

}  // namespace al
