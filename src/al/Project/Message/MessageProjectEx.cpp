#include "Project/Message/MessageProjectEx.hpp"

#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunc.hpp"

namespace al {
/**
 * Creates an empty message project.
 */
MessageProjectEx::MessageProjectEx() = default;

/**
 * Loads the message project data from the project resource.
 */
void MessageProjectEx::init() {
    Resource* resource = findOrCreateResource(sProjectDataPath, nullptr);
    initialize(resource->getOtherFile("ProjectData.msbp", nullptr), nullptr);
}

/**
 * Releases the message project data.
 */
void MessageProjectEx::finalize() {
    sead::MessageProject::finalize();
}

/**
 * Returns the name of a tag group.
 * @param groupIndex tag group index
 * @return tag group name
 */
const char* MessageProjectEx::getTagGroupNameByIndex(s32 groupIndex) const {
    return LMS_GetTagGroupName(mProjFile, groupIndex);
}

/**
 * Returns the name of a tag.
 * @param groupIndex tag group index
 * @param tagIndex tag index in the group
 * @return tag name
 */
const char* MessageProjectEx::getTagNameByIndex(s32 groupIndex, s32 tagIndex) const {
    return LMS_GetTagName(mProjFile, groupIndex, tagIndex);
}

/**
 * Returns the name of a tag parameter.
 * @param groupIndex tag group index
 * @param tagIndex tag index in the group
 * @param paramIndex parameter index in the tag
 * @return parameter name
 */
const char* MessageProjectEx::getTagParamNameByIndex(s32 groupIndex, s32 tagIndex,
                                                     s32 paramIndex) const {
    return LMS_GetTagParamName(mProjFile, groupIndex, tagIndex, paramIndex);
}

/**
 * Returns the number of tag groups.
 * @return tag group count
 */
s32 MessageProjectEx::getTagGroupNum() const {
    return LMS_GetTagGroupNum(mProjFile);
}

/**
 * Returns the number of tags in a group.
 * @param groupIndex tag group index
 * @return tag count
 */
s32 MessageProjectEx::getTagNum(s32 groupIndex) const {
    return LMS_GetTagNum(mProjFile, groupIndex);
}

/**
 * Returns the number of parameters of a tag.
 * @param groupIndex tag group index
 * @param tagIndex tag index in the group
 * @return parameter count
 */
s32 MessageProjectEx::getTagParamNum(s32 groupIndex, s32 tagIndex) const {
    return LMS_GetTagParamNum(mProjFile, groupIndex, tagIndex);
}
}  // namespace al
