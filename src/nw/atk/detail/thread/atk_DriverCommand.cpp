#include <nn/atk/atk_DriverCommand.h>
#include <nn/atk/atk_SoundThread.h>

namespace nn::atk::detail {
DriverCommand& DriverCommand::GetInstance() {
    static DriverCommand instance;
    return instance;
}

DriverCommand& DriverCommand::GetInstanceForTaskThread() {
    static DriverCommand instance;
    return instance;
}

DriverCommand::DriverCommand() {}
// commandBufferSize is the command storage size; commandCount is the request-queue capacity.
size_t DriverCommand::GetRequiredMemSize(size_t commandBufferSize, int commandCount) {
    return CommandManager::GetRequiredMemSize(commandBufferSize, commandCount);
}

// buffer provides bufferSize bytes for commandBufferSize command bytes and queues
// sized for commandCount submissions. Driver callbacks process lists and wake the sound thread.
void DriverCommand::Initialize(void* buffer, size_t bufferSize, size_t commandBufferSize, int commandCount) {
    CommandManager::Initialize(buffer, bufferSize, commandBufferSize, commandCount, ProcessCommandList);
    SetRequestCallback(RequestProcessCommand);
}

void DriverCommand::RequestProcessCommand() { driver::SoundThread::GetInstance().ForceWakeup(); }
}
