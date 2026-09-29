#pragma once

#include <container/seadPtrArray.h>
#include <hostio/seadHostIONode.h>
#include "common/aglShader.h"
#include "common/aglShaderEnum.h"

namespace sead {
class Heap;
}

// more information:
// https://github.com/aboood40091/sead/blob/master/packages/agl/include/common/aglShaderCompileInfo.h
namespace agl {

class ShaderCompileInfo : public sead::hostio::Node {
public:
    // this value is used as an index to a table of version lists
    // on SMO 1.2.0, located at 0x7101E80B30
    enum Target {};

    ShaderCompileInfo();

    virtual ~ShaderCompileInfo();

    void destroy();
    void create(s32 macroNum, s32 variationNum, sead::Heap* pHeap);
    void clearVariation();
    void pushBackVariation(const char*, const char*);
    void calcCompileSource(ShaderType, sead::BufferedSafeString*, Target, bool) const;
    static const sead::SafeString& getRegitserUniformBlockName();  // "RegisterUBO"

    void setName(const sead::SafeString& rName) { mName = rName; }
    void setSource(const sead::SafeString* pSource) { mSource = pSource; }

private:
    friend class ShaderProgramEdit;

    sead::SafeString mName;
    const sead::SafeString* mSource;
    void* _20;
    sead::PtrArray<const char> mMacroName;
    sead::PtrArray<const char> mMacroValue;
    sead::PtrArray<const char> mVariationName;
    sead::PtrArray<const char> mVariationValue;
};
static_assert(sizeof(ShaderCompileInfo) == 0x68);

}  // namespace agl
