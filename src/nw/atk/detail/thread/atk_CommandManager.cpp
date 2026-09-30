#include <nn/atk/atk_CommandManager.h>

namespace nn::atk::detail {
CommandBuffer::CommandBuffer()
    : mBuffer(nullptr), mCapacity(0), mWritePosition(0), mReadPosition(0), mFull(true) {}
CommandBuffer::~CommandBuffer() {}
void CommandBuffer::Finalize() {}
// buffer supplies size bytes; allocation cursors count four-byte words.
void CommandBuffer::Initialize(void* buffer, size_t size) {
    mFull = false;
    mWritePosition = 0;
    mReadPosition = 0;
    mBuffer = static_cast<u32*>(buffer);
    mCapacity = size / 4;
}

// size includes the command header and is rounded up to four-byte alignment.
Command* CommandBuffer::AllocMemory(size_t size) {
    if (mFull) return nullptr;
    size_t words = (size + 3) / 4;
    volatile size_t read = mReadPosition;
    Command* command;
    if (read > mWritePosition) {
        if (mWritePosition + words > read) return nullptr;
        command = reinterpret_cast<Command*>(mBuffer + mWritePosition);
        mWritePosition += words;
    } else if (mWritePosition + words <= mCapacity) {
        command = reinterpret_cast<Command*>(mBuffer + mWritePosition);
        mWritePosition += words;
    } else {
        if (words > read) return nullptr;
        command = reinterpret_cast<Command*>(mBuffer);
        mWritePosition = words;
    }

    if (command) {
        if (mWritePosition == read) mFull = true;
        command->bufferEnd = mWritePosition;
    }

    return command;
}

// command is the final command in the completed range being released.
void CommandBuffer::FreeMemory(Command* command) {
    mReadPosition = command->bufferEnd;
    mFull = false;
}

size_t CommandBuffer::GetCommandBufferSize() const { return mCapacity * 4; }
size_t CommandBuffer::GetAllocatableCommandSize() const {
    if (mFull) return 0;
    volatile size_t write = mWritePosition;
    volatile size_t read = mReadPosition;
    if (read > write) return (read - write) * 4;
    size_t endSpace = mCapacity - write;
    size_t beginSpace = read;
    return (endSpace < beginSpace ? beginSpace : endSpace) * 4;
}

size_t CommandBuffer::GetAllocatedCommandBufferSize() const {
    if (mFull) return mCapacity * 4;
    volatile size_t write = mWritePosition;
    volatile size_t read = mReadPosition;
    if (read > write) return (write + (mCapacity - read)) * 4;
    return (write - read) * 4;
}

CommandManager::CommandManager()
    : mInitialized(false), mProcessCallback(nullptr), mRequestCallback(nullptr) {}
CommandManager::~CommandManager() { Finalize(); }
void CommandManager::Finalize() {
    mCommands.Finalize();
    mReplies.Finalize();
    mInitialized = false;
}

// buffer holds bufferSize bytes: commandBufferSize bytes of commands followed by
// commandCount request slots and commandCount+1 reply slots. callback processes a list.
// As in the original, the caller is responsible for supplying sufficient bufferSize.
void CommandManager::Initialize(void* buffer, size_t bufferSize, size_t commandBufferSize,
                                int commandCount, ProcessCallback callback) {
    mProcessCallback = callback;
    mRequestCallback = nullptr;
    mBuffer.Initialize(buffer, commandBufferSize);
    mSequence = 0;
    mHead = nullptr;
    mTail = nullptr;
    mCommands.buffer = reinterpret_cast<u64*>(static_cast<u8*>(buffer) + commandBufferSize);
    mReplies.buffer = mCommands.buffer + commandCount;
    mCommands.Initialize(commandCount);
    mReplies.Initialize(commandCount + 1);
    mPendingCount.store(0, std::memory_order_release);
    mInitialized = true;
}

// commandBufferSize is the command storage size; commandCount is the request-queue capacity.
size_t CommandManager::GetRequiredMemSize(size_t commandBufferSize, int commandCount) {
    return commandBufferSize + (size_t(commandCount + 1) + size_t(commandCount)) * sizeof(u64);
}

// size includes the command header. option is retained by the ABI but unused here.
Command* CommandManager::AllocMemory(size_t size, bool option) {
    Command* command = TryAllocMemory(size);
    if (command) {
        ++mAllocatedCount;
        return command;
    }

    RecvCommandReply();
    command = TryAllocMemory(size);
    if (command) {
        ++mAllocatedCount;
        return command;
    }

    do {
        if (mHead) WaitCommandReply(FlushCommand(true, false));
        else RecvCommandReplySync();
        command = TryAllocMemory(size);
    } while (!command);
    ++mAllocatedCount;
    return command;
}

// size includes the command header; this attempt neither waits nor updates the command count.
Command* CommandManager::TryAllocMemory(size_t size) { return mBuffer.AllocMemory(size); }
void CommandManager::RecvCommandReply() {
    u64 message;
    while (os::TryReceiveMessageQueue(&message, &mReplies.queue))
        FinalizeCommandList(reinterpret_cast<Command*>(message));
}

// block permits waiting for queue space and inserts a fence when no list is pending.
// option is retained by the ABI but unused by this implementation.
u32 CommandManager::FlushCommand(bool block, bool option) {
    Command* head = mHead;
    if (!head) {
        if (!block) return 0;
        Command* command = AllocMemory(sizeof(Command), false);
        command->type = 0xffffffffu;
        PushCommand(command);
        head = mHead;
    }

    u32 sequence = mSequence;
    mPendingCount.fetch_add(1, std::memory_order_acq_rel);
    u64 message = reinterpret_cast<u64>(head);
    if (!os::TrySendMessageQueue(&mCommands.queue, message)) {
        if (!block) return 0;
        if (mRequestCallback) mRequestCallback();
        os::SendMessageQueue(&mCommands.queue, message);
    }

    mHead->sequence = sequence;
    ++mSequence;
    mHead = nullptr;
    mTail = nullptr;
    return sequence;
}

// sequence identifies the submitted list whose reply must be received before returning.
void CommandManager::WaitCommandReply(u32 sequence) {
    while (RecvCommandReplySync()->sequence != sequence) {}
}

Command* CommandManager::RecvCommandReplySync() {
    u64 message;
    if (!os::TryReceiveMessageQueue(&message, &mReplies.queue)) {
        if (mRequestCallback) mRequestCallback();
        os::ReceiveMessageQueue(&message, &mReplies.queue);
    }

    Command* command = reinterpret_cast<Command*>(message);
    FinalizeCommandList(command);
    return command;
}

// command is appended to the current batch; the return value identifies that batch.
u32 CommandManager::PushCommand(Command* command) {
    if (mTail) mTail->next = command;
    else mHead = command;
    mTail = command;
    command->next = nullptr;
    return mSequence;
}

// block permits waiting for queue space and submitting an empty-list fence.
u32 CommandManager::FlushCommand(bool block) { return FlushCommand(block, false); }
// command begins a completed list; release its allocations through its final entry.
void CommandManager::FinalizeCommandList(Command* command) {
    mFinishedSequence = command->sequence;
    Command* last;
    do {
        --mAllocatedCount;
        last = command;
        command = command->next;
    } while (command);
    mBuffer.FreeMemory(last);
}

// sequence is compared with the last reply using the original wraparound window.
bool CommandManager::IsFinishCommand(u32 sequence) const {
    if (mFinishedSequence < sequence) return sequence - mFinishedSequence >= 0x40000000u;
    return mFinishedSequence - sequence < 0x40000000u;
}

size_t CommandManager::GetCommandBufferSize() const { return mBuffer.GetCommandBufferSize(); }
size_t CommandManager::GetAllocatableCommandSize() const { return mBuffer.GetAllocatableCommandSize(); }
size_t CommandManager::GetAllocatedCommandBufferSize() const { return mBuffer.GetAllocatedCommandBufferSize(); }
int CommandManager::GetAllocatedCommandCount() const { return mAllocatedCount; }
bool CommandManager::ProcessCommand() {
    u64 message;
    if (!os::TryReceiveMessageQueue(&message, &mCommands.queue)) return false;
    mPendingCount.fetch_sub(1, std::memory_order_acq_rel);
    Command* command = reinterpret_cast<Command*>(message);
    if (command->type != 0xffffffffu) mProcessCallback(command);
    os::SendMessageQueue(&mReplies.queue, message);
    return true;
}
}
