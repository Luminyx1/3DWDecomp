#include "layer/aglDrawMethod.h"

#include <prim/seadSafeString.h>

#include "layer/aglRenderer.h"

namespace agl::lyr {

namespace {

const char* const cBindTypeName[] = {"none", "const", "non-const"};

}  // namespace

/**
 * Unregisters the draw method from the renderer.
 */
DrawMethod::~DrawMethod()
{
    if (Renderer::instance())
    {
        Renderer::instance()->removeDrawMethod(this);
    }
}

/**
 * Compares two draw methods by priority.
 * @param pA first draw method
 * @param pB second draw method
 * @return difference of the priorities
 */
s32 DrawMethod::compare(const DrawMethod* pA, const DrawMethod* pB)
{
    return pA->mPriority - pB->mPriority;
}

/**
 * Generates the host I/O message describing the draw method.
 * @param pContext host I/O context
 */
void DrawMethod::genMessage(sead::hostio::Context* pContext)
{
    sead::FormatFixedSafeString<256> str("%s ( priority : %d / bind : %s )", mName.cstr(),
                                          mPriority, cBindTypeName[mBindType]);
}

}  // namespace agl::lyr
