#pragma once

namespace al {
class FunctorBase;

class StageSwitchListener {
public:
    virtual void listenOn() = 0;
    virtual void listenOff() = 0;
};

class StageSwitchFunctorListener : public StageSwitchListener {
public:
    StageSwitchFunctorListener();

    void setOnFunctor(const FunctorBase& rFunctor);
    void setOffFunctor(const FunctorBase& rFunctor);
    void listenOn() override;
    void listenOff() override;

    FunctorBase* mOnFunctor = nullptr;
    FunctorBase* mOffFunctor = nullptr;
};
}  // namespace al
