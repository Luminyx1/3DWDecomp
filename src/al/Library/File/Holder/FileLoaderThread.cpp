#include "Library/File/Holder/FileLoaderThread.hpp"

#include <prim/seadDelegate.h>
#include <thread/seadDelegateThread.h>

#include "Library/File/FileEntryBase.hpp"

namespace al {
/**
 * @brief Creates and starts the thread that loads file entries in the background.
 * @param priority The priority of the loader thread.
 */
FileLoaderThread::FileLoaderThread(s32 priority) {
    mThread = new sead::DelegateThread(
        "FileLoadThread",
        new sead::Delegate2<FileLoaderThread, sead::Thread*, s64>(
            this, &FileLoaderThread::threadFunction),
        nullptr, priority, sead::MessageQueue::BlockType::Blocking, 0x7fffffff, 0x10000, 0x200);
    mThread->start();
}

/**
 * @brief Thread entry point that loads the file entry passed as a message.
 * @param pThread The loader thread.
 * @param fileEntryPtr The address of the file entry to load.
 */
void FileLoaderThread::threadFunction(sead::Thread* pThread, s64 fileEntryPtr) {
    reinterpret_cast<FileEntryBase*>(fileEntryPtr)->load();
}

/**
 * @brief Queues a file entry to be loaded on the loader thread.
 * @param pFileEntry The file entry to load.
 */
void FileLoaderThread::requestLoadFile(FileEntryBase* pFileEntry) {
    mThread->sendMessage(reinterpret_cast<s64>(pFileEntry),
                         sead::MessageQueue::BlockType::NonBlocking);
}
}  // namespace al
