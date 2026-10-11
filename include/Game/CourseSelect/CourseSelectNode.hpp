#pragma once
#include "Library/LiveActor/LiveActor.hpp"

class CourseSelectMiniature;
class ICourseSelectActorController;
class CourseSelectNodeParts;
// Partial layout used by roads and pipes.
class CourseSelectNode : public al::LiveActor {
public:
    CourseSelectNode(const char* pName, const CourseSelectMiniature* pMiniature);
    void setController(ICourseSelectActorController* pController) { mpController = pController; }
    int getFirstNextNodeId() const { return *mNextNodeIds[0]; }
    void initBasic(const al::ActorInitInfo& rInfo);
    void initRoadList(const al::ActorInitInfo& rInfo, const sead::Vector3f* pOffset);
    void initNextNodePos(const al::ActorInitInfo& rInfo);
    bool isCurve() const;
    int getNextNodeNum() const;
    bool isWait() const;
    void openRoadToNextNode(CourseSelectNode* pNext, bool isImmediately);
private:
    ICourseSelectActorController* mpController;
    unsigned char _150[0x40];
    int** mNextNodeIds;
    unsigned char _198[0x58];
};
static_assert(sizeof(CourseSelectNode) == 0x1f0);

namespace CourseSelectNodeFunction {
void calcRoundOffDirH(sead::Vector3f* pOut, const sead::Vector3f& rStart, const sead::Vector3f& rEnd);
CourseSelectNodeParts* createRoadEdge(const char*, const al::ActorInitInfo&, const sead::Vector3f&, const sead::Vector3f&, bool);
}
