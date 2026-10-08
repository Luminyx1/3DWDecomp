#pragma once

#include <container/seadPtrArray.h>

#include "Library/LiveActor/LiveActor.hpp"

class CourseSelectMiniature;
class ICourseSelectActorController;

/**
 * @brief Link from a course-select node to one of its next nodes.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
struct CourseSelectNodeLink {
    s32 nodeIndex;
};

/**
 * @brief Branch point of the course-select map roads; the controller of the object placed on it
 * (miniature, dokan...) is entered from it.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class CourseSelectNode : public al::LiveActor {
public:
    explicit CourseSelectNode(const char* pName, const CourseSelectMiniature* pMiniature = nullptr);

    void initAfterConnect(const al::ActorInitInfo& rInfo);
    void tryAppearPointObj();

    /** @brief Sets the controller of the object placed on the node. @param pController Controller. */
    void setController(ICourseSelectActorController* pController) { mController = pController; }
    /** @brief Gets the controller of the object placed on the node. @return The controller. */
    ICourseSelectActorController* getController() const { return mController; }
    /** @brief Gets the number of next nodes. @return The link count. */
    s32 getLinkNum() const { return mLinks.size(); }
    /** @brief Gets the director index of the first next node. @return The node index. */
    s32 getFrontLinkNodeIndex() const { return mLinks.front()->nodeIndex; }

private:
    u8 _144[0x148 - 0x144];
    ICourseSelectActorController* mController;  // 0x148
    u8 _150[0x188 - 0x150];
    sead::PtrArray<CourseSelectNodeLink> mLinks;  // 0x188
    u8 _198[0x1f0 - 0x198];
};

static_assert(sizeof(CourseSelectNode) == 0x1f0);
