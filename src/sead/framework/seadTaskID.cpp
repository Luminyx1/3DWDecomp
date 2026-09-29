#include <framework/seadTaskID.h>

namespace sead
{
TaskClassID::IntTaskCreator TaskClassID::sIntTaskCreator = nullptr;
TaskClassID::StringTaskCreator TaskClassID::sStringTaskCreator = nullptr;

/**
 * Creates a task of this class through the matching factory.
 * @param rArg construction argument passed to the factory
 * @return the created task, or nullptr when no factory can create it
 */
TaskBase* TaskClassID::create(const TaskConstructArg& rArg) const
{
    switch (mType)
    {
    case Type::cInt:
        if (sIntTaskCreator)
        {
            return sIntTaskCreator(mID.mInt, rArg);
        }
        return nullptr;
    case Type::cFactory:
        return mID.mFactory(rArg);
    case Type::cString:
        if (sStringTaskCreator)
        {
            return sStringTaskCreator(mID.mString, rArg);
        }
        return nullptr;
    default:
        return nullptr;
    }
}

}  // namespace sead
