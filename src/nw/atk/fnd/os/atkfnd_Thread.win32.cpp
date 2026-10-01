#include <nn/atk/atkfnd_Thread.h>
#include <nn/util.h>

namespace nn::fs {
enum Priority : int;
void SetPriorityOnCurrentThread(Priority priority);
}

namespace nn::os { void SetThreadCoreMask(ThreadType* thread, int core, u64 mask); }
namespace nn::atk::detail::fnd {
// thread receives the operating-system object; args supplies its stack and scheduling settings.
// id is retained by the ABI; the original updates mId directly instead of this reference.
bool Thread::Create(os::ThreadType& thread, long& id, const RunArgs& args) {
    if (os::CreateThread(&thread, ThreadMain::Run, this, args.stack, args.stackSize,
                         args.priority, args.core).IsFailure()) {
        mId = 0xffffffffu;
        return false;
    }

    mFsPriority = args.fsPriority;
    os::StartThread(&thread);
    mId = reinterpret_cast<long>(&thread);
    return true;
}

// argument points to the owning Thread passed to CreateThread.
void Thread::ThreadMain::Run(void* argument) {
    auto* thread = static_cast<Thread*>(argument);
    thread->mTerminated = false;
    thread->OnRun();
    FsPriority priority = thread->mFsPriority;

    if (static_cast<u32>(priority) >= 3) NN_UNEXPECTED_DEFAULT;
    fs::SetPriorityOnCurrentThread(static_cast<fs::Priority>(priority));
    thread->mHandler->Run(thread->mArgument);
    thread->OnExit();
    thread->mTerminated = true;
}

void Thread::Detach() { os::DestroyThread(&mThread); }
// name remains caller-owned; a null pointer selects an empty name.
void Thread::SetName(const char* name) { os::SetThreadNamePointer(&mThread, (name != nullptr) ? name : ""); }
// core selects the ideal core and mask identifies the permitted cores.
void Thread::SetAffinityMask(int core, AffinityMask mask) { os::SetThreadCoreMask(&mThread, core, static_cast<u32>(mask)); }
void Thread::Resume() {}
void Thread::Join() { os::WaitThread(&mThread); }
bool Thread::IsTerminated() const { return mTerminated; }
}
