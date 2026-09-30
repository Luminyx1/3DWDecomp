#include "Library/Message/MessageSystem.hpp"

#include <heap/seadHeap.h>
#include <heap/seadFrameHeap.h>
#include <heap/seadHeapMgr.h>

#include "Library/File/FileUtil.hpp"
#include "Library/Memory/HeapUtil.hpp"
#include "Library/Message/MessageHolder.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunc.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Message/MessageProjectEx.hpp"

namespace al {
/**
 * Creates the message project, the message heap and the message tree maps, and loads the system
 * and layout messages.
 */
MessageSystem::MessageSystem() {
    mMessageProject = new MessageProjectEx();
    mMessageProject->init();
    mMessageHeap = sead::FrameHeap::create(0x100000, "MessageHeap", getStationedHeap(), 8,
                                           sead::Heap::cHeapDirection_Forward, false);
    addNamedHeap(mMessageHeap, "MessageHeap");
    mTreeMaps.allocBuffer(3, nullptr);
    mTreeMaps.pushBack(new MessageTreeMap());
    mTreeMaps[0]->allocBuffer(0x40, nullptr);
    mTreeMaps.pushBack(new MessageTreeMap());
    mTreeMaps[1]->allocBuffer(0x40, nullptr);
    mTreeMaps.pushBack(new MessageTreeMap());
    mTreeMaps[2]->allocBuffer(0x40, nullptr);
    sead::ScopedCurrentHeapSetter setter(mMessageHeap);
    tryInitMessageHolder(mTreeMaps[0], "SystemMessage");
    tryInitMessageHolder(mTreeMaps[1], "LayoutMessage");
}

/**
 * Loads one message file of an archive into a message tree map.
 * @param pTreeMap tree map to add the holder to
 * @param pFileName file name in the archive
 * @param pArchiveName message archive name
 * @param pExt extension pattern the file name must match
 * @return whether the file was loaded
 */
bool MessageSystem::tryInitMessageHolder(MessageTreeMap* pTreeMap, const char* pFileName,
                                         const char* pArchiveName, const char* pExt) {
    if (!isMatchString(pFileName, MatchStr(pExt))) {
        return false;
    }

    char name[0x100];
    removeExtensionString(name, 0x100, getBaseName(pFileName));
    MessageHolder* holder = new MessageHolder();
    StringTmp<128> archivePath;
    makeLocalizedArchivePath(&archivePath, StringTmp<128>("MessageData/%s", pArchiveName));
    holder->init(archivePath.cstr(), name);
    pTreeMap->insert(name, holder);
    return true;
}

/**
 * Loads every message file of an archive into a message tree map.
 * @param pTreeMap tree map to add the holders to
 * @param pArchiveName message archive name
 * @return whether the archive exists
 */
bool MessageSystem::tryInitMessageHolder(MessageTreeMap* pTreeMap, const char* pArchiveName) {
    StringTmp<128> archivePath;
    makeLocalizedArchivePath(&archivePath, StringTmp<128>("MessageData/%s", pArchiveName));
    Resource* resource = findOrCreateResource(archivePath, nullptr);
    if (!resource) {
        return false;
    }

    s32 entryNum = resource->getEntryNum("/");
    StringTmp<256> entryName;
    for (s32 i = 0; i < entryNum; i++) {
        resource->getEntryName(&entryName, "/", i);
        if (!searchSubString(entryName.cstr(), ".msbt")) {
            continue;
        }

        char name[0x100];
        removeExtensionString(name, 0x100, getBaseName(entryName.cstr()));
        MessageHolder* holder = new MessageHolder();
        holder->init(resource, entryName.cstr());
        pTreeMap->insert(name, holder);
    }

    return true;
}

/**
 * Releases every message holder and the message project data.
 */
void MessageSystem::destroyMessageData() {
    mMessageProject->finalize();
    mMessageHeap->freeAll();
    mTreeMaps[0]->clear();
    mTreeMaps[1]->clear();
    mTreeMaps[2]->clear();
}

/**
 * Reloads the message project and the system and layout messages.
 */
void MessageSystem::initMessageForChangeLanguage() {
    destroyMessageData();
    sead::ScopedCurrentHeapSetter setter(mMessageHeap);
    mMessageProject->init();
    tryInitMessageHolder(mTreeMaps[0], "SystemMessage");
    tryInitMessageHolder(mTreeMaps[1], "LayoutMessage");
}

/**
 * Returns the message project.
 * @return message project
 */
MessageProjectEx* MessageSystem::getMessageProject() const {
    return mMessageProject;
}

/**
 * Returns a system message holder.
 * @param pName message file name
 * @return the holder, or nullptr
 */
MessageHolder* MessageSystem::getSystemMessageHolder(const char* pName) const {
    return getMessageHolderCore(pName, 0);
}

/**
 * Returns a message holder from one of the message tree maps.
 * @param pName message file name
 * @param index tree map index
 * @return the holder, or nullptr
 */
MessageHolder* MessageSystem::getMessageHolderCore(const char* pName, s32 index) const {
    MessageTreeMap::Node* node = mTreeMaps[index]->find(pName);
    if (!node) {
        return nullptr;
    }

    return node->value();
}

/**
 * Returns a system message holder.
 * @param pName message file name
 * @param pLanguage language name
 * @return the holder, or nullptr
 */
MessageHolder* MessageSystem::getSystemMessageHolder(const char* pName,
                                                     const char* pLanguage) const {
    return getMessageHolderCore(pName, 0, pLanguage);
}

/**
 * Returns a message holder from one of the message tree maps.
 * @param pName message file name
 * @param index tree map index
 * @param pLanguage language name
 * @return the holder, or nullptr
 */
MessageHolder* MessageSystem::getMessageHolderCore(const char* pName, s32 index,
                                                   const char* pLanguage) const {
    MessageTreeMap::Node* node = mTreeMaps[index]->find(pName);
    if (!node) {
        return nullptr;
    }

    return node->value();
}

/**
 * Returns a layout message holder.
 * @param pName message file name
 * @return the holder, or nullptr
 */
MessageHolder* MessageSystem::getLayoutMessageHolder(const char* pName) const {
    return getMessageHolderCore(pName, 1);
}

/**
 * Returns a stage message holder.
 * @param pName message file name
 * @return the holder, or nullptr
 */
MessageHolder* MessageSystem::getStageMessageHolder(const char* pName) const {
    return getMessageHolderCore(pName, 2);
}
}  // namespace al
