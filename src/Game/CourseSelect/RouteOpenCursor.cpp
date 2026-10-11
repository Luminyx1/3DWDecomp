#include "CourseSelect/RouteOpenCursor.hpp"
#include "CourseSelect/CourseSelectDirector.hpp"
#include "CourseSelect/CourseSelectNode.hpp"

/** @brief Create a cursor with space for 32 route nodes. @param pDirector Course-select graph director. */
RouteOpenCursor::RouteOpenCursor(CourseSelectDirector* pDirector) : mpDirector(pDirector) {
    mpNodes = new CourseSelectNode*[32];
    for (int i = 0; i < 32; ++i) mpNodes[i] = nullptr;
}
/** @brief Find a route and start opening its first road. @param pStart Starting node. @param pEnd Destination node. @return Whether a route with at least two nodes was found. */
bool RouteOpenCursor::initialize(CourseSelectNode* pStart, CourseSelectNode* pEnd) {
    mNodeCount = -1;
    mIndex = 0;
    for (int i = 0; i < 32; ++i) mpNodes[i] = nullptr;
    if (pStart->getNextNodeNum() > 0)
        mNodeCount = mpDirector->tryFindNodeList(mpNodes, 32, pStart, pEnd);
    if (mNodeCount <= 1) {
        CourseSelectNode* pCrossing = mpDirector->tryFindFirstCrossingNode(pStart, 10);
        if (pCrossing) mNodeCount = mpDirector->tryFindNodeList(mpNodes, 32, pCrossing, pEnd);
    }
    if (mNodeCount < 2) return false;
    mpNodes[mIndex]->openRoadToNextNode(mpNodes[mIndex + 1], false);
    return true;
}
/** @brief Advance once the current node finishes opening. @return Whether the cursor moved beyond the final node. */
bool RouteOpenCursor::update() {
    if (mpNodes[mIndex]->isWait()) {
        ++mIndex;
        if (mIndex < mNodeCount - 1) mpNodes[mIndex]->openRoadToNextNode(mpNodes[mIndex + 1], false);
        else return mIndex != mNodeCount - 1;
    }
    return false;
}
/** @brief Check whether the cursor has reached the final node. @return Whether no further road remains. */
bool RouteOpenCursor::isEnd() const { return mNodeCount - 1 <= mIndex; }
