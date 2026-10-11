#pragma once
#include "CourseSelect/ICourseSelectActorController.hpp"
class CourseSelectMiniature;

class MiniatureController : public ICourseSelectActorController {
public:
    al::LiveActor* getActor() override;
    int getCursorLayoutType() const override;
    const CourseSelectActorInfo* getCourseSelectActorInfo() const override;
    CourseSelectNode* getCourseSelectNode() const override;
    int calcOpenNodePriority() const override;
    void startPuppetDemo(CourseSelectPuppeteerGroup* pGroup) override;
    void endPuppetDemo() override;
    void startBind(CourseSelectPuppeteer* pPuppeteer) override;
    bool tryDecide(const CourseSelectDirector* pDirector) override;
    void startOpen() override;
    void startOpenImmediately() override;
    bool isEndOpen() const override;
private:
    CourseSelectMiniature* mpActor;
    const CourseSelectActorInfo* mpInfo;
};
static_assert(sizeof(MiniatureController) == 0x18);
