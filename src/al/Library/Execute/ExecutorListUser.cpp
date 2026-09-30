#include "Library/Execute/ExecutorListUser.hpp"

#include <nvn/nvn_FuncPtrInline.h>

#include "Library/Execute/IUseExecutor.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"

namespace al {
static inline void pushDebugGroup(NVNcommandBuffer* pCmdBuf, const char* pName) {
    reinterpret_cast<void (*)(NVNcommandBuffer*, const char*)>(
        pfnc_nvnCommandBufferPushDebugGroup)(pCmdBuf, pName);
}

/**
 * Constructs an executor list of executor users.
 * @param pListName List name.
 * @param capacity Maximum number of users.
 * @param pGroupName Group name.
 */
ExecutorListIUseExecutorBase::ExecutorListIUseExecutorBase(const char* pListName, s32 capacity,
                                                           const char* pGroupName)
    : ExecutorListBase(pListName, pGroupName), mUserNumMax(capacity) {
    mUsers = new IUseExecutor*[capacity];

    for (s32 i = 0; i < mUserNumMax; i++) {
        mUsers[i] = nullptr;
    }
}

/**
 * Adds a user.
 * @param pUser The user.
 */
void ExecutorListIUseExecutorBase::registerUser(IUseExecutor* pUser) {
    mUsers[mUserNum] = pUser;
    mUserNum++;
}

/**
 * Constructs an update executor list of executor users.
 * @param pListName List name.
 * @param capacity Maximum number of users.
 * @param pGroupName Group name.
 */
ExecutorListIUseExecutorUpdate::ExecutorListIUseExecutorUpdate(const char* pListName,
                                                               s32 capacity,
                                                               const char* pGroupName)
    : ExecutorListIUseExecutorBase(pListName, capacity, pGroupName) {}

/**
 * Executes all users.
 */
void ExecutorListIUseExecutorUpdate::executeList() const {
    for (s32 i = 0; i < mUserNum; i++) {
        mUsers[i]->execute();
    }
}

/**
 * Constructs a draw executor list of executor users.
 * @param pListName List name.
 * @param capacity Maximum number of users.
 * @param pGroupName Group name.
 */
ExecutorListIUseExecutorDraw::ExecutorListIUseExecutorDraw(const char* pListName, s32 capacity,
                                                           const char* pGroupName)
    : ExecutorListIUseExecutorBase(pListName, capacity, pGroupName) {}

/**
 * Draws all users inside a debug group named after the list group.
 */
void ExecutorListIUseExecutorDraw::executeList() const {
    pushDebugGroup(GameFrameworkNx::sInstance->mDrawContext->getNvnCommandBuffer(), mGroupName);

    for (s32 i = 0; i < mUserNum; i++) {
        mUsers[i]->draw();
    }

    nvnCommandBufferPopDebugGroup(GameFrameworkNx::sInstance->mDrawContext->getNvnCommandBuffer());
}
}  // namespace al
