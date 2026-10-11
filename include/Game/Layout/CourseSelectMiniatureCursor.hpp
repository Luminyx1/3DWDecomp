#pragma once

#include <math/seadVector.h>
namespace al { class IUseCamera; class LiveActor; }

/**
 * @brief Cursor shown over the selected course miniature on the world map.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class CourseSelectMiniatureCursor {
public:
    static void calcMiniatureLayoutPos(sead::Vector2f* pOut, al::IUseCamera* pCamera,
                                       const al::LiveActor* pActor,
                                       const sead::Vector3f& rOffset, float scale);
    bool isAppearGreenStarLock() const;
};
