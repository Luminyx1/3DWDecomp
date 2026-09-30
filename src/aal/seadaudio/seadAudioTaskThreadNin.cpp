#include "audio/seadAudioTaskThreadNin.h"

#include "mc/seadCoreInfo.h"

namespace sead {
/**
 * Constructs the audio task thread.
 * @param priority Thread priority.
 * @param pHeap Heap for the thread.
 * @param rName Thread name.
 * @param stackSize Stack size.
 * @param messageQueueSize Size of the task queue.
 */
AudioTaskThreadNin::AudioTaskThreadNin(s32 priority, Heap* pHeap, const SafeString& rName, s32 stackSize,
                                       s32 messageQueueSize)
    : Thread(rName, pHeap, priority, MessageQueue::BlockType::Blocking, 0x7fffffff, stackSize,
             messageQueueSize) {}

/**
 * Stops the thread and destroys it.
 */
AudioTaskThreadNin::~AudioTaskThreadNin() {
    if (!isDone()) {
        quitAndWaitDoneSingleThread(false);
    }
}

/**
 * Starts the thread on the calling core.
 * @return True if the thread was started.
 */
bool AudioTaskThreadNin::start() {
    CoreId core = CoreInfo::getCurrentCoreId();
    setAffinity(CoreIdMask(core));
    return Thread::start();
}

/**
 * Executes one queued audio task.
 * @param msg Task to execute.
 */
void AudioTaskThreadNin::calc_(MessageQueue::Element msg) {
    if (!msg) {
        return;
    }

    if (mListener) {
        mListener->onTaskBegin();
    }

    IAudioTaskNin* task = reinterpret_cast<IAudioTaskNin*>(msg);
    task->execute(mState == State::cQuitting);
    task->mPendingCount.decrement();
    if (mListener) {
        mListener->onTaskEnd();
    }
}

/**
 * Queues an audio task.
 * @param pTask Task.
 * @return True if the task was queued.
 */
bool AudioTaskThreadNin::addTask(IAudioTaskNin* pTask) {
    if (mState == State::cQuitting || mState == State::cTerminated) {
        return false;
    }

    if (sendMessage(reinterpret_cast<MessageQueue::Element>(pTask), MessageQueue::BlockType::NonBlocking)) {
        pTask->mPendingCount.increment();
        return true;
    }

    return false;
}
}  // namespace sead
