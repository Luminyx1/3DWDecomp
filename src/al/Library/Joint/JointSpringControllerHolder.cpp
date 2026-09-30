#include "Library/Joint/JointSpringControllerHolder.hpp"

#include "Library/Joint/JointControllerKeeper.hpp"
#include "Library/Joint/JointSpringController.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"

namespace al {

/**
 * Constructs an empty holder.
 */
JointSpringControllerHolder::JointSpringControllerHolder() = default;

/**
 * Allocates room for a number of controllers.
 * @param maxControllers Maximum number of controllers.
 */
void JointSpringControllerHolder::init(s32 maxControllers) {
    mMaxNum = maxControllers;
    mEntries = new Entry[maxControllers];
}

/**
 * Creates spring controllers from a model resource yaml, if it exists.
 * @param pActor Actor to control.
 * @param pFileName Name of the resource yaml.
 */
void JointSpringControllerHolder::init(LiveActor* pActor, const char* pFileName) {
    if (!isExistModelResourceYaml(pActor, pFileName, nullptr)) {
        return;
    }
    init(pActor, ByamlIter(getModelResourceYaml(pActor, pFileName, nullptr)));
}

/**
 * Creates a spring controller for each joint entry in a byaml array.
 * @param pActor Actor to control.
 * @param rIter Byaml array of controller settings.
 */
void JointSpringControllerHolder::init(LiveActor* pActor, const ByamlIter& rIter) {
    s32 size = rIter.getSize();
    if (size < 1) {
        return;
    }

    mMaxNum = size;
    mEntries = new Entry[size];

    for (s32 i = 0; i < size; i++) {
        ByamlIter iter;
        rIter.tryGetIterByIndex(&iter, i);

        const char* jointName = nullptr;
        iter.tryGetStringByKey(&jointName, "JointName");
        if (!jointName) {
            continue;
        }

        JointSpringController* controller = initJointSpringController(pActor, jointName);

        sead::Vector3f childLocalPos;
        if (tryGetByamlV3f(&childLocalPos, iter, "ChildLocalPos")) {
            controller->setChildLocalPos(childLocalPos);
        }
        f32 stability = 0.0f;
        if (tryGetByamlF32(&stability, iter, "Stability")) {
            controller->setStability(stability);
        }
        f32 friction = 0.98f;
        if (tryGetByamlF32(&friction, iter, "Friction")) {
            controller->setFriction(friction);
        }
        f32 limitDegree = 0.0f;
        if (tryGetByamlF32(&limitDegree, iter, "LimitDegree")) {
            controller->setLimitDegree(limitDegree);
        }

        addController(controller, jointName);
    }
}

/**
 * Registers a controller if there is room.
 * @param pController Controller to add.
 * @param pJointName Name of the controlled joint.
 */
void JointSpringControllerHolder::addController(JointSpringController* pController,
                                                const char* pJointName) {
    if (mNum >= mMaxNum) {
        return;
    }
    mEntries[mNum].controller = pController;
    mEntries[mNum].jointName = pJointName;
    mNum++;
}

/**
 * Sets the control rate of every controller to zero.
 */
void JointSpringControllerHolder::offControlAll() {
    for (s32 i = 0; i < mNum; i++) {
        mEntries[i].controller->setControlRate(0.0f);
    }
}

/**
 * Sets the control rate of every controller.
 * @param rate Control rate.
 */
void JointSpringControllerHolder::setControlRateAll(f32 rate) {
    for (s32 i = 0; i < mNum; i++) {
        mEntries[i].controller->setControlRate(rate);
    }
}

/**
 * Sets the control rate of every controller to one.
 */
void JointSpringControllerHolder::onControllAll() {
    for (s32 i = 0; i < mNum; i++) {
        mEntries[i].controller->setControlRate(1.0f);
    }
}

/**
 * Resets every controller.
 */
void JointSpringControllerHolder::resetControlAll() {
    for (s32 i = 0; i < mNum; i++) {
        mEntries[i].controller->reset();
    }
}

/**
 * Increases the control rate of every controller.
 * @param rate Amount to add.
 */
void JointSpringControllerHolder::addControlRateAll(f32 rate) {
    for (s32 i = 0; i < mNum; i++) {
        mEntries[i].controller->addControlRate(rate);
    }
}

/**
 * Decreases the control rate of every controller.
 * @param rate Amount to subtract.
 */
void JointSpringControllerHolder::subControlRateAll(f32 rate) {
    for (s32 i = 0; i < mNum; i++) {
        mEntries[i].controller->subControlRate(rate);
    }
}

}  // namespace al
