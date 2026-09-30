#include "Project/File/FileEntryBase.hpp"

namespace al {
/**
 * Constructs the entry and allocates its message queue.
 */
FileEntryBase::FileEntryBase() {
    mMessageQueue.allocate(1, nullptr);
}

/**
 * Sets the file name.
 * @param rFileName File name.
 */
void FileEntryBase::setFileName(const sead::SafeString& rFileName) {
    mFileName = rFileName;
}

/**
 * Gets the file name.
 * @return The file name.
 */
const sead::SafeString& FileEntryBase::getFileName() const {
    return mFileName;
}

/**
 * Signals that the load finished.
 */
void FileEntryBase::sendMessageDone() {
    mMessageQueue.push(1, sead::MessageQueue::BlockType::NonBlocking);
    mFileState = FileState::IsSendMessageDone;
}

/**
 * Blocks until the load finished.
 */
void FileEntryBase::waitLoadDone() {
    mMessageQueue.pop(sead::MessageQueue::BlockType::Blocking);
    mFileState = FileState::IsLoadDone;
}

/**
 * Clears the entry.
 */
void FileEntryBase::clear() {
    mFileName.clear();
    mFileState = FileState::None;
    mMessageQueue.pop(sead::MessageQueue::BlockType::NonBlocking);
}

/**
 * Marks the entry as requested.
 */
void FileEntryBase::setLoadStateRequested() {
    mFileState = FileState::IsLoadRequested;
}
}  // namespace al
