#pragma once

#include <message/seadMessageSet.h>
#include <prim/seadSafeString.h>

#include "Library/HostIO/IUseHioNode.hpp"

namespace nn::font {
class Font;
}

namespace al {

class IUseMessageSystem;
class LayoutActor;
class LayoutSystem;
class MessageProjectEx;
class MessageTag;
class MessageTagDataHolder;
class Resource;

struct ReplaceTimeInfo {
    u32 year;
    u32 month;
    u32 day;
    u32 hour;
    u32 minute;
    u32 second;
    u32 centiSecond;
};

class MessageHolder : public HioNode {
public:
    MessageHolder();
    virtual ~MessageHolder();

    void init(const char* pArchiveName, const char* pFileName);
    void init(Resource* pResource, const char* pFileName);
    const char16_t* getText(s32 index) const;
    const char16_t* getText(const char* pLabel) const;
    const char16_t* tryGetText(const char* pLabel) const;
    bool isExistText(const char* pLabel) const;
    s32 calcCharacterNum(s32 index) const;
    s32 calcCharacterNum(const char* pLabel) const;
    s32 calcCharacterByteSize(const char* pLabel) const;
    s32 getTextNum() const;
    void searchTextLabelByIndex(sead::BufferedSafeString* pLabel, s32 index) const;
    s32 getStyleByIndex(s32 index) const;
    s32 trySearchStyleIndexByLabel(const char* pLabel) const;

private:
    sead::MessageSet<char16_t>* mMessageSet;
};

const char* getLanguage();
const char* getLayoutMessageArcName();
bool isMessageTagMark(char16_t);
bool isMessageTagEndMark(char16_t);
bool isMessageTagPageBreak(const IUseMessageSystem*, const char16_t*);
bool isMessageTagPageBreak(const IUseMessageSystem*, const MessageTag&);
bool isMessageTagPageBreak(const MessageProjectEx*, const MessageTag&);
const char* getMessageTagGroupName(const MessageProjectEx*, s32);
const char* getMessageTagGroupName(const IUseMessageSystem*, s32);
const char* getMessageTagName(const MessageProjectEx*, s32, s32);
const char* getMessageTagName(const IUseMessageSystem*, s32, s32);
bool isExistMessageTag(const char16_t*);
s32 calcMessageSizeWithoutNullCharacter(const char16_t*, const char16_t*);
bool isExistMessageTagTextPaneAnim(const IUseMessageSystem*, const char16_t*);
bool tryGetMessageTagTextAnim(sead::BufferedSafeString*, const IUseMessageSystem*, const char16_t*);
bool isMessageTagVoice(const IUseMessageSystem*, const char16_t*);
void getMessageTagVoiceName(sead::BufferedSafeString*, const IUseMessageSystem*, const char16_t*);
bool tryGetMessageTagVoiceNameInPage(sead::BufferedSafeString*, const IUseMessageSystem*,
                                     const char16_t*);
bool isMessageTagPictFont(const IUseMessageSystem*, s32);
bool isMessageTagDeviceFont(const IUseMessageSystem*, s32);
bool isExistMessageTagPadSwitch(const IUseMessageSystem*, const char16_t*);
bool isMessageTagPadStyle(const IUseMessageSystem*, s32, s32);
bool isMessageTagPadPair(const IUseMessageSystem*, s32, s32);
bool isMessageTagPadStyle2P(const IUseMessageSystem*, s32, s32);
bool isMessageTagAlignLeft(const IUseMessageSystem*, s32, s32);
bool isMessageTagAlignCenter(const IUseMessageSystem*, s32, s32);
void replaceMessageTagString(sead::WBufferedSafeString*, const IUseMessageSystem*, const char16_t*,
                             const char16_t*);
void replaceMessageTagTimeDirectRaceTime(sead::WBufferedSafeString*, const IUseMessageSystem*,
                                         ReplaceTimeInfo&);
const char16_t* getSystemMessageString(const IUseMessageSystem*, const char*, const char*);
void replaceMessageTagTimeDirectDate(sead::WBufferedSafeString*, const IUseMessageSystem*,
                                     ReplaceTimeInfo&);
void replaceMessageTagTimeDirectDateDetail(sead::WBufferedSafeString*, const IUseMessageSystem*,
                                           ReplaceTimeInfo&);
void replaceMessageTagScore(sead::WBufferedSafeString*, const IUseMessageSystem*, const char16_t*,
                            s32, const char*);
void replaceMessageTagCoinNum(sead::WBufferedSafeString*, const IUseMessageSystem*, const char16_t*,
                              s32, const char*);
void replaceMessageTagAmiiboName(sead::WBufferedSafeString*, const IUseMessageSystem*,
                                 const char16_t*, const char*, const char*);
void replaceMessageTagUserName(sead::WBufferedSafeString*, const IUseMessageSystem*, const char16_t*,
                               const char16_t*, const char*);
void replaceMessageTagNamedString(sead::WBufferedSafeString*, const IUseMessageSystem*,
                                  const char16_t*, const char16_t*, const char*);
void replaceMessageTagTime(sead::WBufferedSafeString*, const IUseMessageSystem*, const char16_t*,
                           ReplaceTimeInfo&, const char*);
void createReplaceTimeInfoForRaceTime(ReplaceTimeInfo*, s32, s32, s32);
void createReplaceTimeInfoForDateTime(ReplaceTimeInfo*, u64);
void replacePaneDateTime(LayoutActor*, const char*, u64);
MessageTagDataHolder* initMessageTagDataHolder(s32);
void registerMessageTagDataScore(MessageTagDataHolder*, const char*, const s32*);
void registerMessageTagDataCoinNum(MessageTagDataHolder*, const char*, const s32*);
void registerMessageTagDataUserName(MessageTagDataHolder*, const char*, const char16_t**);
void registerMessageTagDataAmiiboName(MessageTagDataHolder*, const char*, const char**);
void registerMessageTagDataString(MessageTagDataHolder*, const char*, const char16_t**);
void replaceMessageTagData(sead::WBufferedSafeString*, const IUseMessageSystem*,
                           const MessageTagDataHolder*, const char16_t*);
s32 calcMessageSizeWithoutTag(const char16_t*, const char16_t*);
bool isExistLayoutMessage(const IUseMessageSystem*, const char*);
bool isExistSystemMessage(const IUseMessageSystem*, const char*);
bool isExistStageMessage(const IUseMessageSystem*, const char*);
bool isExistLabelInLayoutMessage(const IUseMessageSystem*, const char*, const char*);
bool isExistLabelInSystemMessage(const IUseMessageSystem*, const char*, const char*);
bool isExistLabelInStageMessage(const IUseMessageSystem*, const char*, const char*);
s32 calcSystemMessageCharacterNum(const IUseMessageSystem*, const char*, const char*);
s32 calcSystemMessageCharacterNumWithoutTag(const IUseMessageSystem*, const char*, const char*);
const char16_t* getLayoutMessageString(const IUseMessageSystem*, const char*, const char*);
const char16_t* tryGetSystemMessageString(const IUseMessageSystem*, const char*, const char*);
const char16_t* getStageMessageString(const IUseMessageSystem*, const char*, const char*);
bool tryGetStageMessageString(const char16_t**, const IUseMessageSystem*, const char*, const char*);
const char16_t* getLayoutMessageString(const IUseMessageSystem*, const char*, s32);
const char16_t* getSystemMessageString(const IUseMessageSystem*, const char*, s32);
s32 getSystemMessageLabelNum(const IUseMessageSystem*, const char*);
s32 getLayoutMessageLabelNum(const IUseMessageSystem*, const char*);
const char* getMessageTagParamName(const IUseMessageSystem*, const MessageTag&, s32);
s32 getMessageTagParamNum(const IUseMessageSystem*, const MessageTag&);
void getMessageTagParamString(sead::WBufferedSafeString*, const IUseMessageSystem*,
                              const MessageTag&, s32);
void getSystemMessageLabelString(sead::BufferedSafeString*, const IUseMessageSystem*, const char*,
                                 s32);
void getLayoutMessageLabelString(sead::BufferedSafeString*, const IUseMessageSystem*, const char*,
                                 s32);
char16_t* getMessageLine(char16_t*, u32, const char16_t*, u32);
s32 countMessageLine(const char16_t*);
bool copyMessageExpandTag(char16_t*, s32, const IUseMessageSystem*, const char16_t*);
bool copyMessageWithoutTag(char16_t*, s32, const char16_t*, s32);
bool copyMessageWithoutRubyTag(char16_t*, s32, const IUseMessageSystem*, const char16_t*);
void copyMessageWithoutTagExpandRuby(char16_t*, s32, const IUseMessageSystem*, const char16_t*);
s32 copyMessageWithTag(char16_t*, s32, const char16_t*);
s32 copyMessageWithTagOnlyCurrentPage(const IUseMessageSystem*, char16_t*, s32, const char16_t*, s32);
const char16_t* getMessageWithPage(const IUseMessageSystem*, const char16_t*, s32);
void copyMessageOnlyRuby(char16_t*, s32, const IUseMessageSystem*, const char16_t*);
s32 countMessagePage(const IUseMessageSystem*, const char16_t*, s32);
const char16_t* getNextMessagePage(const IUseMessageSystem*, const char16_t*);
nn::font::Font* getPictureFont(const LayoutSystem*);

}  // namespace al
