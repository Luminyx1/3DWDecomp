#include "basis/seadNew.h"
#include "basis/seadRawPrint.h"
#include "thread/seadMessageQueue.h"

namespace sead
{
/**
 * Creates an empty queue; call allocate() before using it.
 */
MessageQueue::MessageQueue() = default;

/**
 * Destroys the queue (free() releases its buffer).
 */
MessageQueue::~MessageQueue() = default;

/**
 * Allocates room for the messages and sets the queue up.
 * @param size number of messages the queue holds
 * @param pHeap heap to allocate the buffer from
 */
void MessageQueue::allocate(s32 size, Heap* pHeap)
{
    if (size <= 0)
    {
        SEAD_ASSERT_MSG(false, "MessageQueue size must not be zero");
        return;
    }

    mBuffer = new (pHeap) Element[size];
    nn::os::InitializeMessageQueue(&mMessageQueueInner, reinterpret_cast<u64*>(mBuffer), size);
}

/**
 * Finalizes the queue and frees its buffer.
 */
void MessageQueue::free()
{
    nn::os::FinalizeMessageQueue(&mMessageQueueInner);
    if (mBuffer)
    {
        delete[] mBuffer;
        mBuffer = nullptr;
    }
}

/**
 * Adds a message at the back.
 * @param message message to send
 * @param blockType whether to wait for room when the queue is full
 * @return whether the message was sent
 */
bool MessageQueue::push(MessageQueue::Element message, MessageQueue::BlockType blockType)
{
    if (blockType == BlockType::Blocking)
    {
        nn::os::SendMessageQueue(&mMessageQueueInner, message);
        return true;
    }

    return nn::os::TrySendMessageQueue(&mMessageQueueInner, message);
}

/**
 * Takes the message at the front.
 * @param blockType whether to wait for a message when the queue is empty
 * @return the message, or 0 if there was none
 */
MessageQueue::Element MessageQueue::pop(MessageQueue::BlockType blockType)
{
    u64 message;

    if (blockType == BlockType::Blocking)
    {
        nn::os::ReceiveMessageQueue(&message, &mMessageQueueInner);
        return message;
    }

    if (nn::os::TryReceiveMessageQueue(&message, &mMessageQueueInner))
    {
        return message;
    }

    return 0;
}

/**
 * Looks at the message at the front without taking it.
 * @param blockType whether to wait for a message when the queue is empty
 * @return the message, or 0 if there was none
 */
MessageQueue::Element MessageQueue::peek(MessageQueue::BlockType blockType) const
{
    u64 message;

    if (blockType == BlockType::Blocking)
    {
        nn::os::PeekMessageQueue(&message, &mMessageQueueInner);
        return message;
    }

    if (nn::os::TryPeekMessageQueue(&message, &mMessageQueueInner))
    {
        return message;
    }

    return 0;
}

/**
 * Adds a message at the front, ahead of the others.
 * @param message message to send
 * @param blockType whether to wait for room when the queue is full
 * @return whether the message was sent
 */
bool MessageQueue::jam(MessageQueue::Element message, MessageQueue::BlockType blockType)
{
    if (blockType == BlockType::Blocking)
    {
        nn::os::JamMessageQueue(&mMessageQueueInner, message);
        return true;
    }

    return nn::os::TryJamMessageQueue(&mMessageQueueInner, message);
}
}  // namespace sead
