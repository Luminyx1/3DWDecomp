#pragma once

#include <container/seadObjArray.h>

namespace agl {
class UniformBlockLocation;
}  // namespace agl

namespace al {
class SimpleModelG3D;
class UniformBlock;
struct UniformBlockLayout;

/**
 * @brief A per-model uniform block together with the location it is bound to.
 */
struct UniformBlockAssign {
    UniformBlock* mUniformBlock;
    agl::UniformBlockLocation* mLocation;
};

static_assert(sizeof(UniformBlockAssign) == 0x10);

class UniformBlockAssignArray : public sead::ObjArray<UniformBlockAssign> {};

static_assert(sizeof(UniformBlockAssignArray) == 0x20);

UniformBlockAssign* tryCreateUniformBlockAssign(SimpleModelG3D* pModel, const char* pName,
                                                const UniformBlockLayout* pLayout, s32 layoutNum);
UniformBlockAssign* findUniformBlockAssign(const UniformBlockAssignArray* pArray,
                                           const char* pName);

}  // namespace al
