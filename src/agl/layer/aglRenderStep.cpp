#include "layer/aglRenderStep.h"

#include "detail/aglRootNode.h"
#include "layer/aglDrawMethod.h"

namespace agl::lyr {

/**
 * Constructs an enabled render step without draw methods.
 */
RenderStep::RenderStep() : mFlag(1)
{
    detail::RootNode::setNodeMeta(this, "Icon = IN");
}

/**
 * Sorts the draw methods by priority.
 */
void RenderStep::calc()
{
    if (mDrawMethod.size() < 2)
    {
        return;
    }

    DrawMethod** ppMethods = mDrawMethod.data();
    s32 top = 0;
    s32 bottom = mDrawMethod.size() - 1;
    while (top < bottom)
    {
        s32 last = top;
        for (s32 i = top; i < bottom; i++)
        {
            if (DrawMethod::compare(ppMethods[i], ppMethods[i + 1]) > 0)
            {
                DrawMethod* pTmp = ppMethods[i];
                ppMethods[i] = ppMethods[i + 1];
                ppMethods[i + 1] = pTmp;
                last = i;
            }
        }
        bottom = last;
        if (top >= bottom)
        {
            break;
        }

        last = bottom;
        for (s32 i = bottom; i > top; i--)
        {
            if (DrawMethod::compare(ppMethods[i], ppMethods[i - 1]) < 0)
            {
                DrawMethod* pTmp = ppMethods[i - 1];
                ppMethods[i - 1] = ppMethods[i];
                ppMethods[i] = pTmp;
                last = i;
            }
        }
        top = last;
    }
}

/**
 * Adds a draw method if it is not registered yet.
 * @param pMethod draw method to add
 * @return false if the draw method was already registered
 */
bool RenderStep::pushBack(DrawMethod* pMethod)
{
    for (auto& rMethod : mDrawMethod)
    {
        if (&rMethod == pMethod)
        {
            return false;
        }
    }
    mDrawMethod.pushBack(pMethod);
    return true;
}

/**
 * Removes every draw method bound to an object.
 * @param pObject object the draw methods are bound to
 * @return number of removed draw methods
 */
s32 RenderStep::removeByObject(const void* pObject)
{
    s32 count = 0;
    s32 index = 0;
    for (auto it = mDrawMethod.begin(); it != mDrawMethod.end();)
    {
        if (it->getObject() == pObject)
        {
            mDrawMethod.erase(index);
            count++;
        }
        else
        {
            ++index;
            ++it;
        }
    }
    return count;
}

/**
 * Removes a draw method.
 * @param pMethod draw method to remove
 * @return number of removed draw methods
 */
s32 RenderStep::remove(const DrawMethod* pMethod)
{
    s32 count = 0;
    s32 index = 0;
    for (auto it = mDrawMethod.begin(); it != mDrawMethod.end();)
    {
        if (&*it == pMethod)
        {
            mDrawMethod.erase(index);
            count++;
        }
        else
        {
            ++index;
            ++it;
        }
    }
    return count;
}

/**
 * Removes all draw methods.
 */
void RenderStep::clear()
{
    mDrawMethod.clear();
}

/**
 * Generates the host I/O messages of every draw method.
 * @param pContext host I/O context
 */
void RenderStep::genMessage(sead::hostio::Context* pContext)
{
    for (auto& rMethod : mDrawMethod)
    {
        rMethod.genMessage(pContext);
    }
}

/**
 * Handles a host I/O property event (no-op in release builds).
 * @param pEvent property event
 */
void RenderStep::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

/**
 * Handles a host I/O node event (no-op in release builds).
 * @param pEvent node event
 */
void RenderStep::listenNodeEvent(const sead::hostio::NodeEvent* pEvent) {}

}  // namespace agl::lyr
