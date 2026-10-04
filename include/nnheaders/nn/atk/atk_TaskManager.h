#pragma once

namespace nn::atk::detail {
class TaskManager {
  public:
    static TaskManager& GetInstance();
    void WaitTask();
    void CancelWaitTask();
    void ExecuteTask();
};
} // namespace nn::atk::detail
