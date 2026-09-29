#include "utility/aglNamedObj.h"

namespace agl::utl {

/**
 * Constructs a named object.
 */
INamedObj::INamedObj() = default;

/**
 * Destroys the named object.
 */
INamedObj::~INamedObj() = default;

/**
 * Gets the name of the group that objects belong to when no group is set.
 * @return default group name
 */
const sead::SafeString& INamedObj::getDefaultGroupName()
{
    static const sead::SafeString cDefaultGroupName("default");
    return cDefaultGroupName;
}

/**
 * Gets the name of the object.
 * @return object name
 */
const sead::SafeString& INamedObj::getObjName() const
{
    return sead::SafeString::cEmptyString;
}

/**
 * Gets the name of the group the object belongs to.
 * @return group name
 */
const sead::SafeString& INamedObj::getGroupName() const
{
    return sead::SafeString::cEmptyString;
}

/**
 * Gets the type of the object.
 * @return object type
 */
s32 INamedObj::getObjType() const
{
    return 0;
}

/**
 * Checks whether the object is exposed to host I/O.
 * @return true
 */
bool INamedObj::isHostIOEnabled() const
{
    return true;
}

}  // namespace agl::utl
