#pragma once
#include "Library/Nerve/NerveStateBase.hpp"
namespace al { class LayoutActor; }
class CourseSelectInfoDecideButton : public al::NerveStateBase {
public:
    explicit CourseSelectInfoDecideButton(al::LayoutActor* pActor);
    ~CourseSelectInfoDecideButton() override;
    void setDecideButtonTxt(int port);
    void startActive(bool immediately);
    void startDeactive(bool immediately);
    bool isEnableDecide() const;
private:
    al::LayoutActor* mpActor;
};
static_assert(sizeof(CourseSelectInfoDecideButton) == 0x20);
