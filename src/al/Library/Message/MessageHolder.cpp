#include "Library/Message/MessageHolder.hpp"

#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunc.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Creates a holder with an empty message set.
 */
MessageHolder::MessageHolder() {
    mMessageSet = new sead::MessageSet<char16_t>;
}

/**
 * Destroys the holder and its message set.
 */
MessageHolder::~MessageHolder() {
    delete mMessageSet;
}

/**
 * Loads a message file from an archive.
 * @param pArchiveName archive path
 * @param pFileName message file name without extension
 */
void MessageHolder::init(const char* pArchiveName, const char* pFileName) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    void* data = resource->getOtherFile(StringTmp<128>("%s.msbt", pFileName), nullptr);
    mMessageSet->initialize(data, nullptr);
}

/**
 * Loads a message file from a resource.
 * @param pResource resource containing the file
 * @param pFileName message file name
 */
void MessageHolder::init(Resource* pResource, const char* pFileName) {
    void* data = pResource->getOtherFile(pFileName, nullptr);
    mMessageSet->initialize(data, nullptr);
}

/**
 * Returns a text by index.
 * @param index text index
 * @return the text, or a placeholder if it doesn't exist
 */
const char16_t* MessageHolder::getText(s32 index) const {
    const char16_t* text = mMessageSet->getText(index);
    return text ? text : u"NULL";
}

/**
 * Returns a text by label.
 * @param pLabel text label
 * @return the text, or a placeholder if it doesn't exist
 */
const char16_t* MessageHolder::getText(const char* pLabel) const {
    const char16_t* text = mMessageSet->getTextByLabel(pLabel);
    return text ? text : u"NULL";
}

/**
 * Returns a text by label.
 * @param pLabel text label
 * @return the text, or nullptr if it doesn't exist
 */
const char16_t* MessageHolder::tryGetText(const char* pLabel) const {
    return mMessageSet->getTextByLabel(pLabel);
}

/**
 * Checks whether a label exists.
 * @param pLabel text label
 * @return whether the text exists
 */
bool MessageHolder::isExistText(const char* pLabel) const {
    return mMessageSet->getTextByLabel(pLabel) != nullptr;
}

/**
 * Returns the character count of a text by index.
 * @param index text index
 * @return number of characters
 */
s32 MessageHolder::calcCharacterNum(s32 index) const {
    return (mMessageSet->calcTextSizeByIndex(index) + 1) / 2;
}

/**
 * Returns the character count of a text by label.
 * @param pLabel text label
 * @return number of characters
 */
s32 MessageHolder::calcCharacterNum(const char* pLabel) const {
    s32 index = mMessageSet->getTextIndexByLabel(pLabel);
    return (mMessageSet->calcTextSizeByIndex(index) + 1) / 2;
}

/**
 * Returns the byte size of a text by label.
 * @param pLabel text label
 * @return size in bytes
 */
s32 MessageHolder::calcCharacterByteSize(const char* pLabel) const {
    s32 index = mMessageSet->getTextIndexByLabel(pLabel);
    return mMessageSet->calcTextSizeByIndex(index);
}

/**
 * Returns the number of texts.
 * @return text count
 */
s32 MessageHolder::getTextNum() const {
    return mMessageSet->getTextNum();
}

/**
 * Writes the label of a text into a string.
 * @param pLabel output label
 * @param index text index
 */
void MessageHolder::searchTextLabelByIndex(sead::BufferedSafeString* pLabel, s32 index) const {
    mMessageSet->searchTextLabelByIndex(pLabel, index);
}

/**
 * Returns the style index of a text.
 * @param index text index
 * @return style index, or -1
 */
s32 MessageHolder::getStyleByIndex(s32 index) const {
    return mMessageSet->getTextStyle(index);
}

/**
 * Returns the style index of a text by label.
 * @param pLabel text label
 * @return style index
 */
s32 MessageHolder::trySearchStyleIndexByLabel(const char* pLabel) const {
    return mMessageSet->getTextStyleByLabel(pLabel);
}
}  // namespace al
