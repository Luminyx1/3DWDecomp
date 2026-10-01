#include <nn/atk/atkfnd_Thread.h>
#include <nn/util.h>

namespace nn::fs {
enum Priority : int;
void SetPriorityOnCurrentThread(Priority priority);
}

namespace nn::atk::detail::fnd {
Thread::RunArgs::RunArgs()
    : name(""), stack(nullptr), stackSize(0), core(-1), affinity(AffinityMask_Default),
      priority(16), fsPriority(FsPriority_Default), argument(nullptr), handler(nullptr) {}
bool Thread::RunArgs::IsValid() const {
    if (stack == nullptr) return false;

    if (!stackSize) return false;

    if (priority > 31) return false;
    return handler != nullptr;
}

Thread::~Thread() {}
// args provides the stack, callback and argument, name, priorities, and core selection.
bool Thread::Run(const RunArgs& args) {
    if (!args.IsValid()) return false;
    mArgument = args.argument;
    mHandler = args.handler;

    if (!Create(mThread, mId, args)) return false;
    SetName(args.name);

    if (args.affinity != AffinityMask_Default) SetAffinityMask(args.core, args.affinity);
    mPriority = args.priority;
    Resume();
    return true;
}

void Thread::WaitForExit() { Join(); }
void Thread::Release() {
    if (mState == State_Exited) {
        Detach();
        mState = State_Released;
    }
}

// state replaces the thread's lifecycle marker.
void Thread::SetState(State state) { mState = state; }
int Thread::GetPriority() const { return mPriority; }
Thread::State Thread::GetState() const { return mState; }
void Thread::OnRun() { mState = State_Running; }
void Thread::OnExit() { mState = State_Exited; }
Thread::Thread()
    : mState(State_NotStarted), mId(0xffffffffu), mPriority(16),
      mHandler(nullptr), mTerminated(false) {}
// priority is the new operating-system scheduling priority.
int Thread::SetPriority(int priority) { return os::ChangeThreadPriority(&mThread, priority); }
// priority is one of the three file-system priority values, numbered zero through two.
void Thread::SetFsPriority(FsPriority priority) {
    mFsPriority = priority;

    if (static_cast<u32>(priority) >= 3) NN_UNEXPECTED_DEFAULT;
    fs::SetPriorityOnCurrentThread(static_cast<fs::Priority>(priority));
}

Thread::FsPriority Thread::GetFsPriority() const { return mFsPriority; }
// duration specifies how long the calling thread should sleep.
void Thread::Sleep(const TimeSpan& duration) { os::SleepThread(nn::TimeSpan::FromNanoSeconds(duration.ToNanoSeconds())); }
}
