#include "Library/File/Holder/FileLoaderThread.hpp"

#include <prim/seadDelegate.h>
#include <thread/seadDelegateThread.h>

#include "Project/File/FileEntryBase.hpp"

namespace al {
/**
 * Creates and starts the file loader thread.
 * @param priority Thread priority.
 */
FileLoaderThread::FileLoaderThread(s32 priority) {
    mThread = new sead::DelegateThread(
        "FileLoadThread",
        new sead::Delegate2<FileLoaderThread, sead::Thread*, sead::MessageQueue::Element>(
            this, &FileLoaderThread::threadFunction),
        nullptr, priority, sead::MessageQueue::BlockType::Blocking, 0x7fffffff, 0x10000, 0x200);
    mThread->start();
}

/**
 * Loads the file entry passed as message.
 * @param pThread Unused.
 * @param message Pointer to the file entry.
 */
void FileLoaderThread::threadFunction(sead::Thread* pThread, sead::MessageQueue::Element message) {
    reinterpret_cast<FileEntryBase*>(message)->load();
}

/**
 * Queues a file entry for loading.
 * @param pEntry The file entry.
 */
void FileLoaderThread::requestLoadFile(FileEntryBase* pEntry) {
    mThread->sendMessage(reinterpret_cast<sead::MessageQueue::Element>(pEntry),
                         sead::MessageQueue::BlockType::NonBlocking);
}
}  // namespace al
