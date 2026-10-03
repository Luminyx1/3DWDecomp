#pragma once

#include <attributes.h>
#include <container/seadPtrArray.h>

#include "Project/Model/MeshDrawer.hpp"

/**
 * Sorts the mesh drawers by draw order (MeshDrawer::operator<).
 *
 * The original binary emits this instantiation out of line with the bubble sort and the comparer
 * inlined, instead of calling into PtrArrayImpl::sort.
 */
template <>
inline NOINLINE void sead::PtrArray<al::MeshDrawer>::sort() {
    if (mPtrNum < 2) {
        return;
    }

    al::MeshDrawer** ptrs = reinterpret_cast<al::MeshDrawer**>(mPtrs);
    s32 lo = 0;
    s32 hi = mPtrNum - 1;

    while (lo < hi) {
        s32 last = lo;

        for (s32 i = lo; i < hi; i++) {
            if (compareT(ptrs[i], ptrs[i + 1]) > 0) {
                al::MeshDrawer* tmp = ptrs[i + 1];
                ptrs[i + 1] = ptrs[i];
                ptrs[i] = tmp;
                last = i;
            }
        }

        if (last <= lo) {
            break;
        }

        hi = last;

        for (s32 i = hi; i > lo; i--) {
            if (compareT(ptrs[i], ptrs[i - 1]) < 0) {
                al::MeshDrawer* tmp = ptrs[i - 1];
                ptrs[i - 1] = ptrs[i];
                ptrs[i] = tmp;
                last = i;
            }
        }

        if (last == hi) {
            break;
        }

        lo = last;
    }
}
