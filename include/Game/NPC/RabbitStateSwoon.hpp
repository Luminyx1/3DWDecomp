#pragma once
#include "Library/Nerve/NerveStateBase.hpp"
class Rabbit;
class RabbitStateSwoon : public al::HostStateBase<Rabbit> {
public:
    explicit RabbitStateSwoon(Rabbit* pRabbit);
    ~RabbitStateSwoon() override;
    void init() override;
    void appear() override;
    void kill() override;
    void exeSwoonStart();
    void exeSwoon();
    void exeSwoonEnd();
    void exeTurn();

private:
    const char* mStartAction;
    bool mSkipSwoon;
};
