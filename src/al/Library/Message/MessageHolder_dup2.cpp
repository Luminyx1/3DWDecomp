#include "Library/Message/MessageHolder.hpp"

#include <prim/seadStringUtil.h>
#include <time/seadCalendarTime.h>
#include <time/seadDateTime.h>

#include "Library/File/FileUtil.hpp"
#include "Library/Layout/LayoutActor.hpp"
#include "Library/Layout/LayoutActorUtil.hpp"
#include "Library/Layout/LayoutSystem.hpp"
#include "Library/Message/IUseMessageSystem.hpp"
#include "Library/Message/LanguageUtil.hpp"
#include "Library/Message/MessageSystem.hpp"
#include "Library/Message/MessageTag.hpp"
#include "Library/Message/MessageTagData.hpp"
#include "Library/Message/MessageTagDataHolder.hpp"
#include "Library/Message/ReplaceTagProcessorBase.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Message/MessageProjectEx.hpp"

namespace al {
namespace {
inline bool isMessageTagNamed(const MessageProjectEx* pProject, const MessageTag& rTag,
                              const char* pGroupName, const char* pTagName) {
    const char* groupName = pProject->getTagGroupNameByIndex(rTag.getGroup());
    if (!groupName || !isEqualString(pGroupName, groupName)) {
        return false;
    }
    const char* tagName = pProject->getTagNameByIndex(rTag.getGroup(), rTag.getType());
    if (!tagName) {
        return false;
    }
    return isEqualString(pTagName, tagName);
}

inline bool isMessageTagNamed(const IUseMessageSystem* pMsgSystem, const MessageTag& rTag,
                              const char* pGroupName, const char* pTagName) {
    return isMessageTagNamed(pMsgSystem->getMessageSystem()->getMessageProject(), rTag,
                             pGroupName, pTagName);
}
}  // namespace

/**
 * Returns the current language name.
 * @return language name
 */
const char* getLanguage() {
    return getLanguageString();
}

/**
 * Returns the localized layout message archive path.
 * @return archive path
 */
const char* getLayoutMessageArcName() {
    StringTmp<128> path;
    makeLocalizedArchivePath(&path, "MessageData/LayoutMessage");
    return path.cstr();
}

/**
 * Checks whether a character starts a message tag.
 * @param c character to check
 * @return whether it is a tag or tag end mark
 */
bool isMessageTagMark(char16_t c) {
    return c == 0xe || c == 0xf;
}

/**
 * Checks whether a character starts a message tag end.
 * @param c character to check
 * @return whether it is a tag end mark
 */
bool isMessageTagEndMark(char16_t c) {
    return c == 0xe;
}

/**
 * Checks whether a message position is a page break tag.
 * @param pMsgSystem message system
 * @param pMessage message position
 * @return whether it is a page break
 */
bool isMessageTagPageBreak(const IUseMessageSystem* pMsgSystem, const char16_t* pMessage) {
    if (!pMessage || !isMessageTagMark(*pMessage)) {
        return false;
    }
    MessageTag tag(pMessage);
    return isMessageTagPageBreak(pMsgSystem, tag);
}

/**
 * Checks whether a tag is a page break.
 * @param pMsgSystem message system
 * @param rTag tag to check
 * @return whether it is a page break
 */
bool isMessageTagPageBreak(const IUseMessageSystem* pMsgSystem, const MessageTag& rTag) {
    return isMessageTagNamed(pMsgSystem, rTag, "System", "PageBreak");
}

/**
 * Checks whether a tag is a page break.
 * @param pProject message project
 * @param rTag tag to check
 * @return whether it is a page break
 */
bool isMessageTagPageBreak(const MessageProjectEx* pProject, const MessageTag& rTag) {
    return isMessageTagNamed(pProject, rTag, "System", "PageBreak");
}

/**
 * Returns the name of a tag group.
 * @param pProject message project
 * @param groupIndex tag group index
 * @return group name
 */
const char* getMessageTagGroupName(const MessageProjectEx* pProject, s32 groupIndex) {
    return pProject->getTagGroupNameByIndex(groupIndex);
}

/**
 * Returns the name of a tag group.
 * @param pMsgSystem message system
 * @param groupIndex tag group index
 * @return group name
 */
const char* getMessageTagGroupName(const IUseMessageSystem* pMsgSystem, s32 groupIndex) {
    return pMsgSystem->getMessageSystem()->getMessageProject()->getTagGroupNameByIndex(groupIndex);
}

/**
 * Returns the name of a tag.
 * @param pProject message project
 * @param groupIndex tag group index
 * @param tagIndex tag index
 * @return tag name
 */
const char* getMessageTagName(const MessageProjectEx* pProject, s32 groupIndex, s32 tagIndex) {
    return pProject->getTagNameByIndex(groupIndex, tagIndex);
}

/**
 * Returns the name of a tag.
 * @param pMsgSystem message system
 * @param groupIndex tag group index
 * @param tagIndex tag index
 * @return tag name
 */
const char* getMessageTagName(const IUseMessageSystem* pMsgSystem, s32 groupIndex, s32 tagIndex) {
    return pMsgSystem->getMessageSystem()->getMessageProject()->getTagNameByIndex(groupIndex,
                                                                                 tagIndex);
}

/**
 * Checks whether a message contains any tag.
 * @param pMessage message
 * @return whether a tag exists
 */
bool isExistMessageTag(const char16_t* pMessage) {
    s32 size = calcMessageSizeWithoutNullCharacter(pMessage, nullptr);
    for (s32 i = 0; i < size; i++) {
        if (isMessageTagMark(pMessage[i])) {
            return true;
        }
    }
    return false;
}

/**
 * Returns the number of characters (tags included) up to the end or terminator of a message.
 * @param pStart message start
 * @param pEnd message end, or nullptr to stop at the terminator
 * @return length in characters
 */
s32 calcMessageSizeWithoutNullCharacter(const char16_t* pStart, const char16_t* pEnd) {
    const char16_t* ptr = pStart;
    while (true) {
        u32 step;
        if (*ptr == 0xe) {
            step = static_cast<u16>(ptr[3] + 8);
        } else if (*ptr == 0xf) {
            step = 6;
        } else if (*ptr == 0) {
            break;
        } else {
            step = 2;
        }
        ptr = reinterpret_cast<const char16_t*>(reinterpret_cast<const u8*>(ptr) + step);
        if (pEnd && ptr == pEnd) {
            break;
        }
    }
    return static_cast<s32>(reinterpret_cast<const u8*>(ptr) - reinterpret_cast<const u8*>(pStart)) >> 1;
}

/**
 * Checks whether a message contains a text pane animation control tag.
 * @param pMsgSystem message system
 * @param pMessage message
 * @return whether such a tag exists
 */
bool isExistMessageTagTextPaneAnim(const IUseMessageSystem* pMsgSystem, const char16_t* pMessage) {
    s32 size = calcMessageSizeWithoutNullCharacter(pMessage, nullptr);
    for (s32 i = 0; i < size;) {
        if (isMessageTagMark(pMessage[i])) {
            MessageTag tag(&pMessage[i]);
            if (isMessageTagNamed(pMsgSystem, tag, "Eui", "Speed") ||
                isMessageTagNamed(pMsgSystem, tag, "Eui", "Wait") ||
                isMessageTagNamed(pMsgSystem, tag, "Eui", "Flush")) {
                return true;
            }
            i += tag.getSkipLength();
        } else {
            i++;
        }
    }
    return false;
}

/**
 * Finds the first text animation tag of the current page and writes its name.
 * @param pOut output animation name
 * @param pMsgSystem message system
 * @param pMessage message
 * @return whether an animation tag was found
 */
bool tryGetMessageTagTextAnim(sead::BufferedSafeString* pOut, const IUseMessageSystem* pMsgSystem,
                              const char16_t* pMessage) {
    s32 size = calcMessageSizeWithoutNullCharacter(pMessage, nullptr);
    for (s32 i = 0; i < size;) {
        if (isMessageTagMark(pMessage[i])) {
            MessageTag tag(&pMessage[i]);
            if (isMessageTagNamed(pMsgSystem, tag, "System", "PageBreak")) {
                return false;
            }
            const char* name;
            if (isMessageTagNamed(pMsgSystem, tag, "TextAnim", "Cold")) {
                name = "Cold";
            } else if (isMessageTagNamed(pMsgSystem, tag, "TextAnim", "Run")) {
                name = "Run";
            } else if (isMessageTagNamed(pMsgSystem, tag, "TextAnim", "Walk")) {
                name = "Walk";
            } else if (isMessageTagNamed(pMsgSystem, tag, "TextAnim", "Shake")) {
                name = "Shake";
            } else if (isMessageTagNamed(pMsgSystem, tag, "TextAnim", "Roll")) {
                name = "Roll";
            } else if (isMessageTagNamed(pMsgSystem, tag, "TextAnim", "Tremble")) {
                name = "Tremble";
            } else if (isMessageTagNamed(pMsgSystem, tag, "TextAnim", "Wave")) {
                name = "Wave";
            } else if (isMessageTagNamed(pMsgSystem, tag, "TextAnim", "Scream")) {
                name = "Scream";
            } else if (isMessageTagNamed(pMsgSystem, tag, "TextAnim", "Beat")) {
                name = "Beat";
            } else {
                i += tag.getSkipLength();
                continue;
            }
            pOut->format(name);
            return true;
        }
        i++;
    }
    return false;
}

/**
 * Checks whether a message position is a voice tag.
 * @param pMsgSystem message system
 * @param pMessage message position
 * @return whether it is a voice tag
 */
bool isMessageTagVoice(const IUseMessageSystem* pMsgSystem, const char16_t* pMessage) {
    MessageTag tag(pMessage);
    return isMessageTagNamed(pMsgSystem, tag, "PlaySe", "Voice");
}

/**
 * Writes the voice name of a voice tag.
 * @param pOut output voice name
 * @param pMsgSystem message system
 * @param pMessage message position of the voice tag
 */
void getMessageTagVoiceName(sead::BufferedSafeString* pOut, const IUseMessageSystem* pMsgSystem,
                            const char16_t* pMessage) {
    MessageTag tag(pMessage);
    const u16* size = reinterpret_cast<const u16*>(tag.getParamPtr(0));
    sead::StringUtil::convertUtf16ToUtf8(pOut->getBuffer(), 0x100,
                                         reinterpret_cast<const char16_t*>(tag.getParamPtr(2)),
                                         *size / 2);
}
/**
 * Finds the first voice tag of the current page and writes its voice name.
 * @param pOut output voice name
 * @param pMsgSystem message system
 * @param pMessage message
 * @return whether a voice tag was found
 */
bool tryGetMessageTagVoiceNameInPage(sead::BufferedSafeString* pOut,
                                     const IUseMessageSystem* pMsgSystem,
                                     const char16_t* pMessage) {
    s32 size = calcMessageSizeWithoutNullCharacter(pMessage, nullptr);
    for (s32 i = 0; i < size;) {
        if (isMessageTagMark(pMessage[i])) {
            MessageTag tag(&pMessage[i]);
            if (isMessageTagNamed(pMsgSystem, tag, "System", "PageBreak")) {
                return false;
            }
            if (isMessageTagVoice(pMsgSystem, &pMessage[i])) {
                getMessageTagVoiceName(pOut, pMsgSystem, &pMessage[i]);
                return true;
            }
            i += tag.getSkipLength();
        } else {
            i++;
        }
    }
    return false;
}

/**
 * Checks whether a tag group is the picture font group.
 * @param pMsgSystem message system
 * @param groupIndex tag group index
 * @return whether it is the picture font group
 */
bool isMessageTagPictFont(const IUseMessageSystem* pMsgSystem, s32 groupIndex) {
    const char* groupName = getMessageTagGroupName(pMsgSystem, groupIndex);
    if (!groupName) {
        return false;
    }
    return isEqualString(groupName, "PictFont");
}

/**
 * Checks whether a tag group is the device font group.
 * @param pMsgSystem message system
 * @param groupIndex tag group index
 * @return whether it is the device font group
 */
bool isMessageTagDeviceFont(const IUseMessageSystem* pMsgSystem, s32 groupIndex) {
    const char* groupName = getMessageTagGroupName(pMsgSystem, groupIndex);
    if (!groupName) {
        return false;
    }
    return isEqualString(groupName, "DeviceFont");
}

/**
 * Checks whether a message contains a pad style or pad pair tag.
 * @param pMsgSystem message system
 * @param pMessage message
 * @return whether such a tag exists
 */
bool isExistMessageTagPadSwitch(const IUseMessageSystem* pMsgSystem, const char16_t* pMessage) {
    s32 size = calcMessageSizeWithoutNullCharacter(pMessage, nullptr);
    for (s32 i = 0; i < size;) {
        if (isMessageTagMark(pMessage[i])) {
            MessageTag tag(&pMessage[i]);
            if (isMessageTagPadStyle(pMsgSystem, tag.getGroup(), tag.getType()) ||
                isMessageTagPadPair(pMsgSystem, tag.getGroup(), tag.getType())) {
                return true;
            }
            i += tag.getSkipLength();
        } else {
            i++;
        }
    }
    return false;
}

namespace {
inline bool isMessageTagGroupAndName(const IUseMessageSystem* pMsgSystem, s32 groupIndex,
                                     s32 tagIndex, const char* pGroupName, const char* pTagName,
                                     bool isStartWith) {
    const char* tagName = getMessageTagName(pMsgSystem, groupIndex, tagIndex);
    const char* groupName = getMessageTagGroupName(pMsgSystem, groupIndex);
    if (!tagName || !groupName) {
        return false;
    }
    if (!isEqualString(groupName, pGroupName)) {
        return false;
    }
    if (isStartWith) {
        return isStartWithString(tagName, pTagName);
    }
    return isEqualString(tagName, pTagName);
}
}  // namespace

/**
 * Checks whether a tag is a pad style tag.
 * @param pMsgSystem message system
 * @param groupIndex tag group index
 * @param tagIndex tag index
 * @return whether it is a pad style tag
 */
bool isMessageTagPadStyle(const IUseMessageSystem* pMsgSystem, s32 groupIndex, s32 tagIndex) {
    return isMessageTagGroupAndName(pMsgSystem, groupIndex, tagIndex, "ProjectTag", "PadStyle",
                                    true);
}

/**
 * Checks whether a tag is a pad pair tag.
 * @param pMsgSystem message system
 * @param groupIndex tag group index
 * @param tagIndex tag index
 * @return whether it is a pad pair tag
 */
bool isMessageTagPadPair(const IUseMessageSystem* pMsgSystem, s32 groupIndex, s32 tagIndex) {
    return isMessageTagGroupAndName(pMsgSystem, groupIndex, tagIndex, "ProjectTag", "PadPair",
                                    true);
}

/**
 * Checks whether a tag is a second player pad style tag.
 * @param pMsgSystem message system
 * @param groupIndex tag group index
 * @param tagIndex tag index
 * @return whether it is a second player pad style tag
 */
bool isMessageTagPadStyle2P(const IUseMessageSystem* pMsgSystem, s32 groupIndex, s32 tagIndex) {
    return isMessageTagGroupAndName(pMsgSystem, groupIndex, tagIndex, "ProjectTag", "PadStyle2P",
                                    true);
}

/**
 * Checks whether a tag is a left alignment tag.
 * @param pMsgSystem message system
 * @param groupIndex tag group index
 * @param tagIndex tag index
 * @return whether it is a left alignment tag
 */
bool isMessageTagAlignLeft(const IUseMessageSystem* pMsgSystem, s32 groupIndex, s32 tagIndex) {
    return isMessageTagGroupAndName(pMsgSystem, groupIndex, tagIndex, "TextAlign", "AlignLeft",
                                    false);
}

/**
 * Checks whether a tag is a center alignment tag.
 * @param pMsgSystem message system
 * @param groupIndex tag group index
 * @param tagIndex tag index
 * @return whether it is a center alignment tag
 */
bool isMessageTagAlignCenter(const IUseMessageSystem* pMsgSystem, s32 groupIndex, s32 tagIndex) {
    return isMessageTagGroupAndName(pMsgSystem, groupIndex, tagIndex, "TextAlign", "AlignCenter",
                                    false);
}
/**
 * Replaces the argument tags of a message with a string.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param pMessage source message
 * @param pString replacement string
 */
void replaceMessageTagString(sead::BufferedSafeStringBase<char16_t>* pOut,
                             const IUseMessageSystem* pMsgSystem, const char16_t* pMessage,
                             const char16_t* pString) {
    ReplaceTagProcessorBase processor;
    processor.replaceArgs(pOut->getBuffer(), pOut->getBufferSize(), pMsgSystem, pMessage, pString);
}

/**
 * Writes a race time using the default race time message.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param rInfo time to write
 */
void replaceMessageTagTimeDirectRaceTime(sead::BufferedSafeStringBase<char16_t>* pOut,
                                         const IUseMessageSystem* pMsgSystem,
                                         ReplaceTimeInfo& rInfo) {
    ReplaceTagProcessorBase processor;
    processor.replaceTime(pOut, pMsgSystem,
                          getSystemMessageString(pMsgSystem, "Time", "RaceTimeDefault"),
                          "Default", rInfo);
}

/**
 * Returns a text of a system message file.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param pLabel text label
 * @return the text, or a placeholder
 */
const char16_t* getSystemMessageString(const IUseMessageSystem* pMsgSystem, const char* pFileName,
                                       const char* pLabel) {
    MessageHolder* holder =
        pMsgSystem->getMessageSystem()->getSystemMessageHolder(pFileName, pLabel);
    if (!holder) {
        return u"NULL";
    }
    return holder->getText(pLabel);
}

/**
 * Writes a date using the default date message.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param rInfo time to write
 */
void replaceMessageTagTimeDirectDate(sead::BufferedSafeStringBase<char16_t>* pOut,
                                     const IUseMessageSystem* pMsgSystem, ReplaceTimeInfo& rInfo) {
    ReplaceTagProcessorBase processor;
    processor.replaceTime(pOut, pMsgSystem,
                          getSystemMessageString(pMsgSystem, "Time", "DateDefault"), "Default",
                          rInfo);
}

/**
 * Writes a detailed date using the default detailed date message.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param rInfo time to write
 */
void replaceMessageTagTimeDirectDateDetail(sead::BufferedSafeStringBase<char16_t>* pOut,
                                           const IUseMessageSystem* pMsgSystem,
                                           ReplaceTimeInfo& rInfo) {
    ReplaceTagProcessorBase processor;
    processor.replaceTime(pOut, pMsgSystem,
                          getSystemMessageString(pMsgSystem, "Time", "DateDetailDefault"),
                          "Default", rInfo);
}

/**
 * Replaces the score tag of a message.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param pMessage source message
 * @param score score to write
 * @param pName tag name
 */
void replaceMessageTagScore(sead::BufferedSafeStringBase<char16_t>* pOut,
                            const IUseMessageSystem* pMsgSystem, const char16_t* pMessage,
                            s32 score, const char* pName) {
    ReplaceTagProcessorBase processor;
    processor.replaceScore(pOut, pMsgSystem, score, pMessage, pName);
}

/**
 * Replaces the coin number tag of a message.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param pMessage source message
 * @param coinNum coin number to write
 * @param pName tag name
 */
void replaceMessageTagCoinNum(sead::BufferedSafeStringBase<char16_t>* pOut,
                              const IUseMessageSystem* pMsgSystem, const char16_t* pMessage,
                              s32 coinNum, const char* pName) {
    ReplaceTagProcessorBase processor;
    processor.replaceCoinNum(pOut, pMsgSystem, coinNum, pMessage, pName);
}

/**
 * Replaces the amiibo name tag of a message.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param pMessage source message
 * @param pAmiiboName amiibo name to write
 * @param pName tag name
 */
void replaceMessageTagAmiiboName(sead::BufferedSafeStringBase<char16_t>* pOut,
                                 const IUseMessageSystem* pMsgSystem, const char16_t* pMessage,
                                 const char* pAmiiboName, const char* pName) {
    ReplaceTagProcessorBase processor;
    processor.replaceAmiiboName(pOut, pMsgSystem, pAmiiboName, pMessage, pName);
}

/**
 * Replaces the user name tag of a message.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param pMessage source message
 * @param pUserName user name to write
 * @param pName tag name
 */
void replaceMessageTagUserName(sead::BufferedSafeStringBase<char16_t>* pOut,
                               const IUseMessageSystem* pMsgSystem, const char16_t* pMessage,
                               const char16_t* pUserName, const char* pName) {
    ReplaceTagProcessorBase processor;
    processor.replaceUserName(pOut, pMsgSystem, pUserName, pMessage, pName);
}

/**
 * Replaces a named string tag of a message.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param pMessage source message
 * @param pString string to write
 * @param pName tag name
 */
void replaceMessageTagNamedString(sead::BufferedSafeStringBase<char16_t>* pOut,
                                  const IUseMessageSystem* pMsgSystem, const char16_t* pMessage,
                                  const char16_t* pString, const char* pName) {
    ReplaceTagProcessorBase processor;
    processor.replaceNamedString(pOut, pMsgSystem, pString, pMessage, pName);
}

/**
 * Replaces the time tag of a message.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param pMessage source message
 * @param rInfo time to write
 * @param pName tag name
 */
void replaceMessageTagTime(sead::BufferedSafeStringBase<char16_t>* pOut,
                           const IUseMessageSystem* pMsgSystem, const char16_t* pMessage,
                           ReplaceTimeInfo& rInfo, const char* pName) {
    ReplaceTagProcessorBase processor;
    processor.replaceTime(pOut, pMsgSystem, pMessage, pName, rInfo);
}

/**
 * Fills a time info with a race time.
 * @param pInfo output time info
 * @param minute minutes
 * @param second seconds
 * @param centiSecond hundredths of a second
 */
void createReplaceTimeInfoForRaceTime(ReplaceTimeInfo* pInfo, s32 minute, s32 second,
                                      s32 centiSecond) {
    pInfo->minute = minute;
    pInfo->second = second;
    pInfo->centiSecond = centiSecond;
}

/**
 * Fills a time info with a date and time.
 * @param pInfo output time info
 * @param time unix time
 */
void createReplaceTimeInfoForDateTime(ReplaceTimeInfo* pInfo, u64 time) {
    sead::DateTime dateTime(time);
    sead::CalendarTime calendarTime;
    dateTime.getCalendarTime(&calendarTime);
    pInfo->year = calendarTime.getYear();
    pInfo->month = calendarTime.getMonth().getValueOneOrigin();
    pInfo->day = calendarTime.getDay();
    pInfo->hour = calendarTime.getHour();
    pInfo->minute = calendarTime.getMinute();
    pInfo->second = calendarTime.getSecond();
}

/**
 * Writes a date into a text pane.
 * @param pActor layout actor
 * @param pPaneName text pane name
 * @param time unix time
 */
void replacePaneDateTime(LayoutActor* pActor, const char* pPaneName, u64 time) {
    ReplaceTimeInfo info;
    WStringTmp<256> string;
    createReplaceTimeInfoForDateTime(&info, time);
    replaceMessageTagTimeDirectDate(&string, pActor, info);
    setPaneString(pActor, pPaneName, string.cstr(), 0);
}

/**
 * Creates a tag data holder.
 * @param maxNum maximum number of tag data entries
 * @return the holder
 */
MessageTagDataHolder* initMessageTagDataHolder(s32 maxNum) {
    return new MessageTagDataHolder(maxNum);
}

/**
 * Registers a score tag data entry.
 * @param pHolder tag data holder
 * @param pName tag name
 * @param pScore score to write
 */
void registerMessageTagDataScore(MessageTagDataHolder* pHolder, const char* pName,
                                 const s32* pScore) {
    pHolder->registerMessageTagData(new MessageTagDataScore(pName, pScore));
}

/**
 * Registers a coin number tag data entry.
 * @param pHolder tag data holder
 * @param pName tag name
 * @param pCoinNum coin number to write
 */
void registerMessageTagDataCoinNum(MessageTagDataHolder* pHolder, const char* pName,
                                   const s32* pCoinNum) {
    pHolder->registerMessageTagData(new MessageTagDataCoinNum(pName, pCoinNum));
}

/**
 * Registers a user name tag data entry.
 * @param pHolder tag data holder
 * @param pName tag name
 * @param pUserName user name to write
 */
void registerMessageTagDataUserName(MessageTagDataHolder* pHolder, const char* pName,
                                    const char16_t** pUserName) {
    pHolder->registerMessageTagData(new MessageTagDataUserName(pName, pUserName));
}

/**
 * Registers an amiibo name tag data entry.
 * @param pHolder tag data holder
 * @param pName tag name
 * @param pAmiiboName amiibo name to write
 */
void registerMessageTagDataAmiiboName(MessageTagDataHolder* pHolder, const char* pName,
                                      const char** pAmiiboName) {
    pHolder->registerMessageTagData(new MessageTagDataAmiiboName(pName, pAmiiboName));
}

/**
 * Registers a named string tag data entry.
 * @param pHolder tag data holder
 * @param pName tag name
 * @param pString string to write
 */
void registerMessageTagDataString(MessageTagDataHolder* pHolder, const char* pName,
                                  const char16_t** pString) {
    pHolder->registerMessageTagData(new MessageTagDataString(pName, pString));
}

/**
 * Applies every registered tag data entry to a message.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param pHolder tag data holder
 * @param pMessage source message
 */
void replaceMessageTagData(sead::BufferedSafeStringBase<char16_t>* pOut,
                           const IUseMessageSystem* pMsgSystem, const MessageTagDataHolder* pHolder,
                           const char16_t* pMessage) {
    pHolder->replaceMessage(pOut, pMsgSystem, pMessage);
}
/**
 * Returns the number of characters of a message without its tags.
 * @param pStart message start
 * @param pEnd message end, or nullptr to stop at the terminator
 * @return length in characters
 */
s32 calcMessageSizeWithoutTag(const char16_t* pStart, const char16_t* pEnd) {
    s32 tagSize = 0;
    const char16_t* ptr = pStart;
    while (true) {
        if (*ptr == 0xe) {
            u16 size = ptr[3] + 8;
            ptr = reinterpret_cast<const char16_t*>(reinterpret_cast<const u8*>(ptr) + size);
            tagSize += size;
        } else if (*ptr == 0xf) {
            ptr += 3;
            tagSize += 6;
        } else if (*ptr == 0) {
            break;
        } else {
            ptr++;
        }
        if (pEnd && ptr == pEnd) {
            break;
        }
    }
    return (static_cast<s32>(reinterpret_cast<const u8*>(ptr) - reinterpret_cast<const u8*>(pStart)) -
            tagSize) >>
           1;
}

/**
 * Checks whether a layout message file exists.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @return whether it exists
 */
bool isExistLayoutMessage(const IUseMessageSystem* pMsgSystem, const char* pFileName) {
    return pMsgSystem->getMessageSystem()->getLayoutMessageHolder(pFileName) != nullptr;
}

/**
 * Checks whether a system message file exists.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @return whether it exists
 */
bool isExistSystemMessage(const IUseMessageSystem* pMsgSystem, const char* pFileName) {
    return pMsgSystem->getMessageSystem()->getSystemMessageHolder(pFileName) != nullptr;
}

/**
 * Checks whether a stage message file exists.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @return whether it exists
 */
bool isExistStageMessage(const IUseMessageSystem* pMsgSystem, const char* pFileName) {
    return pMsgSystem->getMessageSystem()->getStageMessageHolder(pFileName) != nullptr;
}

/**
 * Checks whether a label exists in a layout message file.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param pLabel text label
 * @return whether it exists
 */
bool isExistLabelInLayoutMessage(const IUseMessageSystem* pMsgSystem, const char* pFileName,
                                 const char* pLabel) {
    MessageHolder* holder = pMsgSystem->getMessageSystem()->getLayoutMessageHolder(pFileName);
    if (!holder) {
        return false;
    }
    return holder->isExistText(pLabel);
}

/**
 * Checks whether a label exists in a system message file.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param pLabel text label
 * @return whether it exists
 */
bool isExistLabelInSystemMessage(const IUseMessageSystem* pMsgSystem, const char* pFileName,
                                 const char* pLabel) {
    MessageHolder* holder =
        pMsgSystem->getMessageSystem()->getSystemMessageHolder(pFileName, pLabel);
    if (!holder) {
        return false;
    }
    return holder->isExistText(pLabel);
}

/**
 * Checks whether a label exists in a stage message file.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param pLabel text label
 * @return whether it exists
 */
bool isExistLabelInStageMessage(const IUseMessageSystem* pMsgSystem, const char* pFileName,
                                const char* pLabel) {
    MessageHolder* holder = pMsgSystem->getMessageSystem()->getStageMessageHolder(pFileName);
    if (!holder) {
        return false;
    }
    return holder->isExistText(pLabel);
}

/**
 * Returns the character count of a system message text.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param pLabel text label
 * @return number of characters
 */
s32 calcSystemMessageCharacterNum(const IUseMessageSystem* pMsgSystem, const char* pFileName,
                                  const char* pLabel) {
    return pMsgSystem->getMessageSystem()
        ->getSystemMessageHolder(pFileName, pLabel)
        ->calcCharacterNum(pLabel);
}

/**
 * Returns the character count of a system message text without its tags.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param pLabel text label
 * @return number of characters
 */
s32 calcSystemMessageCharacterNumWithoutTag(const IUseMessageSystem* pMsgSystem,
                                            const char* pFileName, const char* pLabel) {
    return calcMessageSizeWithoutTag(getSystemMessageString(pMsgSystem, pFileName, pLabel),
                                     nullptr);
}

/**
 * Returns a text of a layout message file.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param pLabel text label
 * @return the text, or a placeholder
 */
const char16_t* getLayoutMessageString(const IUseMessageSystem* pMsgSystem, const char* pFileName,
                                       const char* pLabel) {
    MessageHolder* holder = pMsgSystem->getMessageSystem()->getLayoutMessageHolder(pFileName);
    if (!holder) {
        return u"NULL";
    }
    return holder->getText(pLabel);
}

/**
 * Returns a text of a system message file.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param pLabel text label
 * @return the text, nullptr if the label doesn't exist, or a placeholder
 */
const char16_t* tryGetSystemMessageString(const IUseMessageSystem* pMsgSystem,
                                          const char* pFileName, const char* pLabel) {
    MessageHolder* holder =
        pMsgSystem->getMessageSystem()->getSystemMessageHolder(pFileName, pLabel);
    if (!holder) {
        return u"NULL";
    }
    return holder->tryGetText(pLabel);
}

/**
 * Returns a text of a stage message file.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param pLabel text label
 * @return the text, or a placeholder
 */
const char16_t* getStageMessageString(const IUseMessageSystem* pMsgSystem, const char* pFileName,
                                      const char* pLabel) {
    MessageHolder* holder = pMsgSystem->getMessageSystem()->getStageMessageHolder(pFileName);
    const char16_t* text = nullptr;
    if (holder) {
        text = holder->tryGetText(pLabel);
    }
    return text ? text : u"NULL";
}

/**
 * Gets a text of a stage message file.
 * @param pOut output text
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param pLabel text label
 * @return whether the text exists
 */
bool tryGetStageMessageString(const char16_t** pOut, const IUseMessageSystem* pMsgSystem,
                              const char* pFileName, const char* pLabel) {
    MessageHolder* holder = pMsgSystem->getMessageSystem()->getStageMessageHolder(pFileName);
    if (!holder) {
        return false;
    }
    *pOut = holder->tryGetText(pLabel);
    return *pOut != nullptr;
}

/**
 * Returns a text of a layout message file by index.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param index text index
 * @return the text, or a placeholder
 */
const char16_t* getLayoutMessageString(const IUseMessageSystem* pMsgSystem, const char* pFileName,
                                       s32 index) {
    MessageHolder* holder = pMsgSystem->getMessageSystem()->getLayoutMessageHolder(pFileName);
    if (!holder) {
        return u"NULL";
    }
    return holder->getText(index);
}

/**
 * Returns a text of a system message file by index.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param index text index
 * @return the text, or a placeholder
 */
const char16_t* getSystemMessageString(const IUseMessageSystem* pMsgSystem, const char* pFileName,
                                       s32 index) {
    MessageHolder* holder = pMsgSystem->getMessageSystem()->getSystemMessageHolder(pFileName);
    if (!holder) {
        return u"NULL";
    }
    return holder->getText(index);
}

/**
 * Returns the number of texts of a system message file.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @return text count
 */
s32 getSystemMessageLabelNum(const IUseMessageSystem* pMsgSystem, const char* pFileName) {
    MessageHolder* holder = pMsgSystem->getMessageSystem()->getSystemMessageHolder(pFileName);
    if (!holder) {
        return 0;
    }
    return holder->getTextNum();
}

/**
 * Returns the number of texts of a layout message file.
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @return text count
 */
s32 getLayoutMessageLabelNum(const IUseMessageSystem* pMsgSystem, const char* pFileName) {
    MessageHolder* holder = pMsgSystem->getMessageSystem()->getLayoutMessageHolder(pFileName);
    if (!holder) {
        return 0;
    }
    return holder->getTextNum();
}

/**
 * Returns the name of a tag parameter.
 * @param pMsgSystem message system
 * @param rTag tag
 * @param paramIndex parameter index
 * @return parameter name
 */
const char* getMessageTagParamName(const IUseMessageSystem* pMsgSystem, const MessageTag& rTag,
                                   s32 paramIndex) {
    return pMsgSystem->getMessageSystem()->getMessageProject()->getTagParamNameByIndex(
        rTag.getGroup(), rTag.getType(), paramIndex);
}

/**
 * Returns the number of parameters of a tag.
 * @param pMsgSystem message system
 * @param rTag tag
 * @return parameter count
 */
s32 getMessageTagParamNum(const IUseMessageSystem* pMsgSystem, const MessageTag& rTag) {
    return pMsgSystem->getMessageSystem()->getMessageProject()->getTagParamNum(rTag.getGroup(),
                                                                              rTag.getType());
}

/**
 * Copies a string parameter of a tag.
 * @param pOut output string
 * @param pMsgSystem message system
 * @param rTag tag
 * @param paramIndex index of the string parameter
 */
void getMessageTagParamString(sead::BufferedSafeStringBase<char16_t>* pOut,
                              const IUseMessageSystem* pMsgSystem, const MessageTag& rTag,
                              s32 paramIndex) {
    const u8* param = rTag.getParamPtr(0);
    u16 size = *reinterpret_cast<const u16*>(param);
    for (s32 i = 0; i < paramIndex; i++) {
        param += size + 2;
        size = *reinterpret_cast<const u16*>(param);
    }
    pOut->copy(sead::SafeStringBase<char16_t>(reinterpret_cast<const char16_t*>(param + 2)),
               size / 2);
}

/**
 * Writes the label of a system message text.
 * @param pOut output label
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param index text index
 */
void getSystemMessageLabelString(sead::BufferedSafeString* pOut, const IUseMessageSystem* pMsgSystem,
                                 const char* pFileName, s32 index) {
    pMsgSystem->getMessageSystem()->getSystemMessageHolder(pFileName)->searchTextLabelByIndex(pOut,
                                                                                             index);
}

/**
 * Writes the label of a layout message text.
 * @param pOut output label
 * @param pMsgSystem message system
 * @param pFileName message file name
 * @param index text index
 */
void getLayoutMessageLabelString(sead::BufferedSafeString* pOut, const IUseMessageSystem* pMsgSystem,
                                 const char* pFileName, s32 index) {
    pMsgSystem->getMessageSystem()->getLayoutMessageHolder(pFileName)->searchTextLabelByIndex(pOut,
                                                                                             index);
}
/**
 * Copies one line of a message, tags included.
 * @param pDst output buffer
 * @param dstLength output buffer length
 * @param pSrc message
 * @param lineIndex index of the line to copy
 * @return end of the copied line
 */
char16_t* getMessageLine(char16_t* pDst, u32 dstLength, const char16_t* pSrc, u32 lineIndex) {
    u32 line = 0;
    if (lineIndex != 0) {
        while (*pSrc) {
            if (isMessageTagMark(*pSrc)) {
                MessageTag tag(pSrc);
                pSrc += tag.getSkipLength();
            } else {
                if (*pSrc == '\n') {
                    line++;
                }
                pSrc++;
            }
            if (line == lineIndex) {
                break;
            }
        }
    }

    for (u32 i = 0; i < dstLength; i++) {
        while (isMessageTagMark(*pSrc)) {
            *pDst++ = *pSrc;
            MessageTag tag(pSrc);
            for (s32 j = 0; j < tag.getSkipLength(); j++) {
                *pDst++ = *pSrc++;
            }
        }
        if (*pSrc == 0 || *pSrc == '\n') {
            break;
        }
        *pDst = *pSrc;
        if (i < dstLength - 1) {
            pDst++;
            pSrc++;
        }
    }
    *pDst = 0;
    return pDst;
}

/**
 * Counts the lines of a message.
 * @param pMessage message
 * @return number of lines
 */
s32 countMessageLine(const char16_t* pMessage) {
    s32 count = 1;
    while (*pMessage) {
        if (isMessageTagMark(*pMessage)) {
            MessageTag tag(pMessage);
            pMessage += tag.getSkipLength();
        } else {
            if (*pMessage == '\n') {
                count++;
            }
            pMessage++;
        }
    }
    return count;
}
/**
 * Copies a message without its tags.
 * @param pDst output buffer
 * @param dstSize output buffer length
 * @param pSrc message
 * @param srcLength number of characters to read, or a negative value for no limit
 * @return whether the output buffer wasn't filled
 */
bool copyMessageWithoutTag(char16_t* pDst, s32 dstSize, const char16_t* pSrc, s32 srcLength) {
    if (srcLength < 0) {
        srcLength = 0x7fff;
    }
    while (srcLength > 0 && dstSize > 1 && *pSrc) {
        if (isMessageTagMark(*pSrc)) {
            MessageTag tag(pSrc);
            pSrc += tag.getSkipLength();
            srcLength -= tag.getSkipLength();
        } else {
            *pDst++ = *pSrc++;
            dstSize--;
            srcLength--;
        }
    }
    *pDst = 0;
    return dstSize > 1;
}

/**
 * Copies a message with its tags.
 * @param pDst output buffer
 * @param dstSize output buffer length
 * @param pSrc message
 * @return number of copied characters
 */
s32 copyMessageWithTag(char16_t* pDst, s32 dstSize, const char16_t* pSrc) {
    s32 size = calcMessageSizeWithoutNullCharacter(pSrc, nullptr);
    for (s32 i = 0; i < size; i++) {
        *pDst++ = *pSrc++;
    }
    *pDst = 0;
    return size;
}

/**
 * Copies one page of a message with its tags.
 * @param pMsgSystem message system
 * @param pDst output buffer
 * @param dstSize output buffer length
 * @param pSrc message
 * @param page page index
 * @return number of copied characters
 */
s32 copyMessageWithTagOnlyCurrentPage(const IUseMessageSystem* pMsgSystem, char16_t* pDst,
                                      s32 dstSize, const char16_t* pSrc, s32 page) {
    const char16_t* start = getMessageWithPage(pMsgSystem, pSrc, page);
    const char16_t* ptr = start;
    while (*ptr) {
        if (isMessageTagPageBreak(pMsgSystem, ptr)) {
            *pDst = 0;
            return pDst - start;
        }
        if (isMessageTagMark(*ptr)) {
            MessageTag tag(ptr);
            memcpy(pDst, ptr, tag.getSkipLength() * sizeof(char16_t));
            pDst += tag.getSkipLength();
            ptr += tag.getSkipLength();
        } else {
            *pDst++ = *ptr++;
        }
    }
    *pDst = 0;
    return pDst - start;
}

/**
 * Returns the start of a page of a message.
 * @param pMsgSystem message system
 * @param pMessage message
 * @param page page index
 * @return start of the page, or nullptr
 */
const char16_t* getMessageWithPage(const IUseMessageSystem* pMsgSystem, const char16_t* pMessage,
                                   s32 page) {
    if (page == 0) {
        return pMessage;
    }
    s32 pageCount = 0;
    while (*pMessage) {
        if (isMessageTagMark(*pMessage)) {
            MessageTag tag(pMessage);
            pMessage += tag.getSkipLength();
            if (isMessageTagNamed(pMsgSystem, tag, "System", "PageBreak")) {
                if (*pMessage == '\n') {
                    pMessage++;
                }
                pageCount++;
                if (pageCount == page) {
                    return pMessage;
                }
            }
        } else {
            pMessage++;
        }
    }
    return nullptr;
}

/**
 * Counts the pages of a message.
 * @param pMsgSystem message system
 * @param pMessage message
 * @param length number of characters to read, or 0 for no limit
 * @return number of pages
 */
s32 countMessagePage(const IUseMessageSystem* pMsgSystem, const char16_t* pMessage, s32 length) {
    s32 pageCount = 1;
    s32 i = 0;
    while ((length < 1 || i < length) && *pMessage) {
        s32 step;
        if (isMessageTagMark(*pMessage)) {
            MessageTag tag(pMessage);
            pMessage += tag.getSkipLength();
            step = tag.getSkipLength();
            if (isMessageTagNamed(pMsgSystem, tag, "System", "PageBreak")) {
                pageCount++;
            }
        } else {
            pMessage++;
            step = 1;
        }
        i += step;
    }
    return pageCount;
}

/**
 * Returns the start of the page following the current one.
 * @param pMsgSystem message system
 * @param pMessage message
 * @return start of the next page, or nullptr
 */
const char16_t* getNextMessagePage(const IUseMessageSystem* pMsgSystem, const char16_t* pMessage) {
    while (*pMessage) {
        if (!isMessageTagMark(*pMessage)) {
            pMessage++;
            continue;
        }
        MessageTag tag(pMessage);
        pMessage += tag.getSkipLength();
        if (isMessageTagNamed(pMsgSystem, tag, "System", "PageBreak")) {
            if (*pMessage == '\n') {
                return pMessage + 1;
            }
            return pMessage;
        }
    }
    return nullptr;
}

/**
 * Returns the picture font.
 * @param pLayoutSystem layout system
 * @return the picture font
 */
nn::font::Font* getPictureFont(const LayoutSystem* pLayoutSystem) {
    return pLayoutSystem->tryFindFont("PictureFont20.bcfnt");
}
}  // namespace al
