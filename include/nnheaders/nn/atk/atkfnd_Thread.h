#pragma once
#include <nn/atk/atkfnd_Time.h>

namespace nn::atk::detail::fnd {
class Thread {
public:
    enum State { State_NotStarted, State_Running, State_Exited, State_Released };
    enum AffinityMask : u32 { AffinityMask_Default = 0 };
    enum FsPriority { FsPriority_Default = 1 };
    // The callback interface has two destructor slots followed by Run.
    class Handler {
    public:
        virtual ~Handler() = default;
        virtual void Run(void* argument) = 0;
    };
    struct RunArgs {
        RunArgs();
        bool IsValid() const;
        const char* name;
        void* stack;
        size_t stackSize;
        int core;
        AffinityMask affinity;
        int priority;
        FsPriority fsPriority;
        void* argument;
        Handler* handler;
    };
    Thread();
    ~Thread();
    bool Run(const RunArgs& args);
    void WaitForExit();
    void Release();
    void SetState(State state);
    int GetPriority() const;
    State GetState() const;
    void OnRun();
    void OnExit();
    int SetPriority(int priority);
    void SetFsPriority(FsPriority priority);
    FsPriority GetFsPriority() const;
    static void Sleep(const TimeSpan& duration);
    bool Create(os::ThreadType& thread, long& id, const RunArgs& args);
    struct ThreadMain { static void Run(void* argument); };
    void Detach();
    void SetName(const char* name);
    void SetAffinityMask(int core, AffinityMask mask);
    void Resume();
    void Join();
    bool IsTerminated() const;
private:
    State mState;
    os::ThreadType mThread;
    long mId;
    int mPriority;
    FsPriority mFsPriority;
    void* mArgument;
    Handler* mHandler;
    bool mTerminated;
};
static_assert(sizeof(Thread::RunArgs) == 0x38, "Thread arguments size");
static_assert(sizeof(Thread) == 0x1f0, "ATK thread size");
}
