#include "CourseSelect/MiniatureController.hpp"
#include "CourseSelect/CourseSelectActorInfo.hpp"
#include "System/Data/StageDatabaseInfo.hpp"

/** @brief Select a cursor layout for the stage's Green Star count.
 * @return Layout 1, 2, or 3 for stages, or layout 5 for other map destinations.
 */
int MiniatureController::getCursorLayoutType() const {
    if (mpInfo->isUseCourseInfo()) {
        int stars = mpInfo->getStageInfo()->getGreenStarNum();
        int type = stars > 9 ? 2 : 1;
        return stars > 29 ? 3 : type;
    }
    return 5;
}
