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
/**
 * Constructs the framework and installs the default frame draw context lock callback.
 */
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

/**
 * Destroys the framework, finalizing and deleting the owned helper object.
 */
GameFramework::~GameFramework()
{
    if (mUnk4)
    {
        mUnk4->unk9(false);
        delete mUnk4;
    }
}

/**
 * Starts displaying frames, once.
 */
void GameFramework::startDisplay()
{
    if (!mDisplayStarted)
    {
        mDisplayStarted = true;
    }
}

/**
 * Creates every system task under the root task.
 * @param pBase the parent task
 * @param rCreateArg the system task creation settings
 */
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

/**
 * Creates the controller manager singleton task.
 * @param pBase the parent task
 */
void GameFramework::createControllerMgr(TaskBase* pBase)
{
    TaskBase::SystemMgrTaskArg arg(&TTaskFactory<ControllerMgr>);
    arg.parent = pBase;

    mTaskMgr->createSingletonTaskSync<ControllerMgr>(arg);
}

/**
 * Creates the host I/O manager (nothing in release builds).
 * @param pBase the parent task
 * @param pParam the host I/O parameters
 * @param pHeap the heap to allocate from
 */
void GameFramework::createHostIOMgr([[maybe_unused]] TaskBase* pBase,
                                    [[maybe_unused]] HostIOMgr::Parameter* pParam,
                                    [[maybe_unused]] Heap* pHeap)
{
}

/**
 * Creates the process meter singleton task.
 * @param pBase the parent task
 */
void GameFramework::createProcessMeter(TaskBase* pBase)
{
    ProcessMeter::createInstance(pBase->mHeapArray.getPrimaryHeap());
}

/**
 * Creates the sead menu manager (nothing in release builds).
 * @param pBase the parent task
 */
void GameFramework::createSeadMenuMgr([[maybe_unused]] TaskBase* pBase) {}

/**
 * Creates the infinite loop checker (nothing in release builds).
 * @param pBase the parent task
 */
void GameFramework::createInfLoopChecker([[maybe_unused]] TaskBase* pBase, const TickSpan&, int) {}

/**
 * Creates the cuckoo clock (nothing in release builds).
 * @param pBase the parent task
 */
void GameFramework::createCuckooClock([[maybe_unused]] TaskBase* pBase) {}

/**
 * Initializes the framework and creates the system manager heaps and singletons.
 * @param rInitArg the initialization settings
 */
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

/**
 * Waits until startDisplay is called.
 */
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

/**
 * Called when the run loop quits (nothing here).
 * @param pHeap the heap the loop ran with
 */
void GameFramework::quitRun_([[maybe_unused]] Heap* pHeap) {}

/**
 * Locks the frame draw context through the installed callback.
 */
void GameFramework::lockFrameDrawContext()
{
    if (mUnk5)
    {
        mUnk5(true);
    }
}

/**
 * Unlocks the frame draw context through the installed callback.
 */
void GameFramework::unlockFrameDrawContext()
{
    if (mUnk5)
    {
        mUnk5(false);
    }
}

/**
 * Initializes host I/O (nothing in release builds).
 */
void GameFramework::initHostIO_() {}

}  // namespace sead
