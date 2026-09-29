#pragma once

#include "Library/HostIO/IUseHioNode.hpp"

namespace al {
    /// A named list of executors that a table runs in order.
    class ExecutorListBase : public HioNode {
    public:
        ExecutorListBase(const char* pName, const char* pPauseName);

        virtual ~ExecutorListBase() {}

        virtual void executeList() const = 0;
        virtual void executeListPaused() const {}
        virtual bool isActive() const = 0;

        const char* mName;          // _8
        const char* mPauseName;     // _10
    };

    static_assert(sizeof(ExecutorListBase) == 0x18, "ExecutorListBase size");
};
