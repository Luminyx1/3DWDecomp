#pragma once

#include <container/seadPtrArray.h>
#include <container/seadStrTreeMap.h>

namespace sead {
class Heap;
}

namespace al {
class MessageProjectEx;
class MessageHolder;

class MessageSystem {
public:
    using MessageTreeMap = sead::StrTreeMap<256, MessageHolder*>;

    MessageSystem();

    void initMessageForChangeLanguage();
    void destroyMessageData();
    bool tryInitMessageHolder(MessageTreeMap* pTreeMap, const char* pArchiveName,
                              const char* pSubDir, const char* pExt);
    bool tryInitMessageHolder(MessageTreeMap* pTreeMap, const char* pArchiveName);

    MessageProjectEx* getMessageProject() const;
    MessageHolder* getSystemMessageHolder(const char* pName) const;
    MessageHolder* getMessageHolderCore(const char* pName, s32 index) const;
    MessageHolder* getSystemMessageHolder(const char* pName, const char* pLanguage) const;
    MessageHolder* getMessageHolderCore(const char* pName, s32 index, const char* pLanguage) const;
    MessageHolder* getLayoutMessageHolder(const char* pName) const;
    MessageHolder* getStageMessageHolder(const char* pName) const;

private:
    MessageProjectEx* mMessageProject = nullptr;
    sead::PtrArray<MessageTreeMap> mTreeMaps;
    sead::Heap* mMessageHeap = nullptr;
};
}  // namespace al
