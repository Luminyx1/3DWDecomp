#pragma once
#include <nn/atk/atk_CommandManager.h>

namespace nn::atk::detail {
struct ReleaseHeapMemoryCommand : Command {
    const void* pMemory;
    size_t size;
};
class DriverCommand : public CommandManager {
public:
    DriverCommand();
    static DriverCommand& GetInstance();
    static DriverCommand& GetInstanceForTaskThread();
    size_t GetRequiredMemSize(size_t commandBufferSize, int commandCount);
    void Initialize(void* buffer, size_t bufferSize, size_t commandBufferSize, int commandCount);
    static void ProcessCommandList(Command* command);
    static void RequestProcessCommand();
};
}
