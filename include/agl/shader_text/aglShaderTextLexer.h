#pragma once

#include <prim/seadSafeString.h>
#include "shader_text/aglShaderTextClause.h"

namespace sead {
class Heap;
}

namespace agl::shtxt {

class Lexer {
public:
    Lexer();
    ~Lexer();

    void initialize(sead::Heap* pHeap, const char* pText, Clause* pRoot);
    const char* setupCurrentRange(u64 length);
    Clause::Type findNumberBlock() const;
    Clause* createClause(u32 type) const;
    bool execute(bool skipSpace);

    sead::SafeString getErrorMessage() const { return mErrorMessage; }

private:
    sead::Heap* mHeap = nullptr;
    Clause* mRoot = nullptr;
    const char* mText = nullptr;
    const char* mCurrent = nullptr;
    mutable const char* mBegin = nullptr;
    mutable const char* mEnd = nullptr;
    sead::SafeString mErrorMessage;
};
static_assert(sizeof(Lexer) == 0x40);

}  // namespace agl::shtxt
