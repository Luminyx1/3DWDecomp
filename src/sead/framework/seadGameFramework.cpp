#include <controller/seadControllerMgr.h>
#include <filedevice/seadFileDeviceMgr.h>
#include <framework/seadFramework.h>
#include <framework/seadGameFramework.h>
#include <framework/seadMethodTreeMgr.h>
#include <framework/seadProcessMeter.h>
#include <framework/seadTaskMgr.h>
#include <gfx/seadGraphics.h>
#include <heap/seadExpHeap.h>
#include <heap/seadHeapMgr.h>
#include <resource/seadResourceMgr.h>
#include <time/seadTickSpan.h>

namespace sead
{
GameFramework::GameFramework()
{
    mUnk6 = [](bool lock) {
        if (lock)
        {
            Graphics::instance()->lockDrawContext();
        }
        else
        {
            Graphics::instance()->unlockDrawContext();
        }
    };
}

// NON_MATCHING: missing body
GameFramework::~GameFramework()
{
    // required for RTTI functions to generate
}

void GameFramework::startDisplay()
{
    if (!mDisplayStarted)
    {
        mDisplayStarted = true;
    }
}

void GameFramework::createSystemTasks(TaskBase* pBase,
                                      const Framework::CreateSystemTaskArg& rCreateArg)
{
    CreateSystemTaskArg parentArg;
    Framework::createSystemTasks(pBase, parentArg);

    createControllerMgr(pBase);
    createProcessMeter(pBase);
    createSeadMenuMgr(pBase);
    createHostIOMgr(pBase, rCreateArg.hostio_parameter, rCreateArg.heap);
    createInfLoopChecker(pBase, rCreateArg.infloop_detection_span, rCreateArg.infloop_unk);
    createCuckooClock(pBase);
}

void GameFramework::createControllerMgr(TaskBase* pBase)
{
    TaskBase::SystemMgrTaskArg arg(&TTaskFactory<ControllerMgr>);
    arg.parent = pBase;

    mTaskMgr->createSingletonTaskSync<ControllerMgr>(arg);
}

void GameFramework::createHostIOMgr([[maybe_unused]] TaskBase* pBase,
                                    [[maybe_unused]] HostIOMgr::Parameter* param,
                                    [[maybe_unused]] Heap* pHeap)
{
}

void GameFramework::createProcessMeter(TaskBase* pBase)
{
    ProcessMeter::createInstance(pBase->mHeapArray.getPrimaryHeap());
}

void GameFramework::createSeadMenuMgr([[maybe_unused]] TaskBase* pBase) {}

void GameFramework::createInfLoopChecker([[maybe_unused]] TaskBase* pBase, const TickSpan&, int) {}

void GameFramework::createCuckooClock([[maybe_unused]] TaskBase* pBase) {}

void GameFramework::initialize(const Framework::InitializeArg& rInitArg)
{
    Framework::initialize(rInitArg);

    Heap* firstHeap = HeapMgr::getRootHeap(0);
    ExpHeap* systemManagersHeap =
        ExpHeap::create(firstHeap->getMaxAllocatableSize(8), "sead::SystemManagers", firstHeap, 8,
                        Heap::cHeapDirection_Forward, false);

    {
        ExpHeap* resourceMgrHeap =
            ExpHeap::create(systemManagersHeap->getMaxAllocatableSize(8), "sead::ResourceMgr",
                            systemManagersHeap, 8, Heap::cHeapDirection_Forward, false);
        ScopedCurrentHeapSetter scopedHeap(resourceMgrHeap);

        ResourceMgr::createInstance(resourceMgrHeap);
        resourceMgrHeap->adjust();
    }
    {
        ExpHeap* fileDeviceMgrHeap =
            ExpHeap::create(systemManagersHeap->getMaxAllocatableSize(8), "sead::FileDeviceMgr",
                            systemManagersHeap, 8, Heap::cHeapDirection_Forward, false);
        ScopedCurrentHeapSetter scopedHeap(fileDeviceMgrHeap);

        FileDeviceMgr::createInstance(fileDeviceMgrHeap);
        fileDeviceMgrHeap->adjust();
    }

    systemManagersHeap->adjust();
}

void GameFramework::waitStartDisplayLoop_()
{
    Graphics::instance()->lockDrawContext();
    mTaskMgr->beforeCalc();
    mTaskMgr->afterCalc();
    Graphics::instance()->unlockDrawContext();

    while (!mTaskMgr->mRootTask)
    {
        if (mDisplayStarted)
        {
            break;
        }

        Thread::sleep(TickSpan::makeFromMilliSeconds(10));
        Graphics::instance()->lockDrawContext();
        mTaskMgr->beforeCalc();
        mTaskMgr->afterCalc();
        Graphics::instance()->unlockDrawContext();
    }

    mMethodTreeMgr->pauseAll(false);
    if (!mDisplayStarted)
    {
        mDisplayStarted = 1;
    }
}

void GameFramework::quitRun_([[maybe_unused]] Heap* pHeap) {}

void GameFramework::lockFrameDrawContext()
{
    if (mUnk5)
    {
        mUnk5(true);
    }
}

void GameFramework::unlockFrameDrawContext()
{
    if (mUnk5)
    {
        mUnk5(false);
    }
}

void GameFramework::initHostIO_() {}

}  // namespace sead
