#include "Project/Camera/Holder/CameraResourceHolder.hpp"

#include "Library/Camera/CameraTicketId.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * Creates an empty holder.
 * @param pStageName Name of the main stage.
 * @param maxEntries Maximum number of camera resources.
 */
CameraResourceHolder::CameraResourceHolder(const char* pStageName, s32 maxEntries)
    : mStageName(pStageName), mMaxEntries(maxEntries) {
    mEntries = new Entry*[maxEntries];

    for (s32 i = 0; i < mMaxEntries; i++) {
        mEntries[i] = nullptr;
    }
}

static void getStageName(StringTmp<128>* pStageName, const char* pArchiveName) {
    StringTmp<256> archiveName;
    archiveName.format("%s", pArchiveName);
    archiveName.endsWith("Map");
    pStageName->copy(archiveName, archiveName.calcLength());
}

bool CameraResourceHolder::tryInitCameraResource(const Resource* pResource, s32 unused) {
    StringTmp<128> stageName = "";
    getStageName(&stageName, pResource->getArchiveName());

    for (s32 i = 0; i < mNumEntries; i++) {
        if (isEqualString(stageName.cstr(), mEntries[i]->stageName)) {
            return false;
        }
    }

    Entry* entry = new Entry;

    if (pResource->isExistFile(StringTmp<64>{"%s.byml", "CameraParam"})) {
        entry->cameraParam = new ByamlIter(pResource->getByml("CameraParam"));
    }

    if (pResource->isExistFile(StringTmp<64>{"%s.byml", "InterpoleParam"})) {
        entry->interpoleParam = new ByamlIter(pResource->getByml("InterpoleParam"));
    }

    entry->stageName = stageName;

    mEntries[mNumEntries] = entry;
    mNumEntries++;
    return true;
}

/**
 * Assigns a zone index to the resource of a stage.
 * @param pStageName Name of the stage.
 * @param zoneId One-based zone index.
 */
void CameraResourceHolder::tryInitZoneID(const char* pStageName, s32 zoneId) {
    for (s32 i = 0; i < mNumEntries; i++) {
        if (isEqualString(mEntries[i]->stageName, pStageName)) {
            mEntries[i]->zoneId = zoneId - 1;
        }
    }
}

/**
 * Finds the ticket parameters of a camera ticket.
 * @param pTicket Receives the ticket parameters.
 * @param pTicketId Ticket to look up.
 * @param paramType 0 for default tickets, 2 for start tickets, otherwise normal tickets.
 * @return Whether the ticket was found.
 */
bool CameraResourceHolder::tryFindParamResource(ByamlIter* pTicket,
                                                const CameraTicketId* pTicketId,
                                                s32 paramType) const {
    ByamlIter paramList;
    const PlacementId* placementId = pTicketId->getPlacementId();
    const char* paramName;

    if (paramType == 2) {
        paramName = "StartTickets";
    } else if (paramType == 0) {
        paramName = "DefaultTickets";
    } else {
        paramName = "Tickets";
    }

    if (!tryFindCameraParamList(&paramList, placementId, paramName)) {
        return false;
    }

    for (s32 i = 0; i < paramList.getSize(); i++) {
        if (paramList.tryGetIterByIndex(pTicket, i)) {
            ByamlIter id;
            pTicket->tryGetIterByKey(&id, "Id");

            if (pTicketId->isEqual(id)) {
                return true;
            }
        }
    }

    return false;
}

/**
 * Finds a parameter list in the stage resource of a placement.
 * @param pParamList Receives the list.
 * @param pPlacementId Placement whose unit config selects the stage, or nullptr for the main stage.
 * @param pParamName Name of the list.
 * @return Whether the list was found.
 */
bool CameraResourceHolder::tryFindCameraParamList(ByamlIter* pParamList,
                                                  const PlacementId* pPlacementId,
                                                  const char* pParamName) const {
    if (pPlacementId != nullptr && pPlacementId->mUnitConfigName != nullptr && *pPlacementId->mUnitConfigName) {
        return tryFindCameraParamList(pParamList, pPlacementId->mUnitConfigName, pParamName);
    }

    return tryFindCameraParamList(pParamList, mStageName, pParamName);
}

/**
 * Finds the ticket parameters of a camera ticket, using the zone resource when the ticket has no unit config.
 * @param pTicket Receives the ticket parameters.
 * @param pTicketId Ticket to look up.
 * @param paramType 0 for default tickets, 2 for start tickets, otherwise normal tickets.
 * @param zoneId Zone index to fall back to.
 * @return Whether the ticket was found.
 */
bool CameraResourceHolder::tryFindParamResource(ByamlIter* pTicket,
                                                const CameraTicketId* pTicketId, s32 paramType,
                                                s32 zoneId) const {
    ByamlIter paramList;
    const PlacementId* placementId = pTicketId->getPlacementId();
    const char* paramName;

    if (paramType == 2) {
        paramName = "StartTickets";
    } else if (paramType == 0) {
        paramName = "DefaultTickets";
    } else {
        paramName = "Tickets";
    }

    if (!tryFindCameraParamList(&paramList, placementId, zoneId, paramName)) {
        return false;
    }

    for (s32 i = 0; i < paramList.getSize(); i++) {
        if (paramList.tryGetIterByIndex(pTicket, i)) {
            ByamlIter id;
            pTicket->tryGetIterByKey(&id, "Id");

            if (pTicketId->isEqual(id)) {
                return true;
            }
        }
    }

    return false;
}

bool CameraResourceHolder::tryFindCameraParamList(ByamlIter* pParamList,
                                                  const PlacementId* pPlacementId, s32 zoneId,
                                                  const char* pParamName) const {
    const char* stageName = (pPlacementId != nullptr) ? pPlacementId->mUnitConfigName : nullptr;

    if (stageName == nullptr || !*stageName) {
        for (s32 i = 0; i < mNumEntries; i++) {
            if (mEntries[i]->zoneId == zoneId) {
                return tryFindCameraParamList(pParamList, mEntries[i]->stageName.cstr(),
                                              pParamName);
            }
        }
    }

    return tryFindCameraParamList(pParamList, stageName, pParamName);
}

/**
 * Finds the normal ticket parameters of a placement.
 * @param pTicket Receives the ticket parameters.
 * @param pPlacementId Placement to look up.
 * @return Whether the ticket was found.
 */
bool CameraResourceHolder::tryFindParamResource(ByamlIter* pTicket,
                                                const PlacementId* pPlacementId) const {
    ByamlIter paramList;

    if (!tryFindCameraParamList(&paramList, pPlacementId, "Tickets")) {
        return false;
    }

    for (s32 i = 0; i < paramList.getSize(); i++) {
        if (paramList.tryGetIterByIndex(pTicket, i)) {
            ByamlIter id;

            if (pTicket->tryGetIterByKey(&id, "Id") && CameraTicketId::isEqual(id, pPlacementId)) {
                return true;
            }
        }
    }

    return false;
}

/**
 * Counts the start tickets of the main stage.
 * @return Number of start tickets.
 */
s32 CameraResourceHolder::calcEntranceCameraParamNum() const {
    ByamlIter startTickets;

    if (!tryFindCameraParamList(&startTickets, mStageName, "StartTickets")) {
        return 0;
    }

    return startTickets.getSize();
}

/**
 * Finds a parameter list in the resource of a stage.
 * @param pParamList Receives the list.
 * @param pStageName Name of the stage.
 * @param pParamName Name of the list.
 * @return Whether the list was found.
 */
bool CameraResourceHolder::tryFindCameraParamList(ByamlIter* pParamList, const char* pStageName,
                                                  const char* pParamName) const {
    Entry* entry = findCameraResource(pStageName);

    if (entry == nullptr || entry->cameraParam == nullptr) {
        return false;
    }

    return entry->cameraParam->tryGetIterByKey(pParamList, pParamName);
}

/**
 * Checks for start tickets in a zone.
 * @param zoneId Zone index, or a negative value for the main stage.
 * @return Number of start tickets for the main stage, otherwise 1 if the zone has start tickets and 0 if not.
 */
s32 CameraResourceHolder::calcEntranceCameraParamNum(s32 zoneId) const {
    if (zoneId < 0) {
        return calcEntranceCameraParamNum();
    }

    ByamlIter startTickets;

    for (s32 i = 0; i < mNumEntries; i++) {
        Entry* entry = mEntries[i];

        if (entry->zoneId == zoneId &&
            tryFindCameraParamList(&startTickets, entry->stageName.cstr(), "StartTickets")) {
            return 1;
        }
    }

    return 0;
}

/**
 * Gets a start ticket of the main stage.
 * @param pTicket Receives the ticket parameters.
 * @param index Index of the start ticket.
 */
void CameraResourceHolder::getEntranceCameraParamResource(ByamlIter* pTicket, s32 index) const {
    ByamlIter startTickets;
    tryFindCameraParamList(&startTickets, mStageName, "StartTickets");
    startTickets.tryGetIterByIndex(pTicket, index);
}

/**
 * Gets a start ticket of a zone.
 * @param pTicket Receives the ticket parameters.
 * @param index Index of the start ticket.
 * @param zoneId Zone index, or a negative value for the main stage.
 */
void CameraResourceHolder::getEntranceCameraParamResource(ByamlIter* pTicket, s32 index,
                                                          s32 zoneId) const {
    if (zoneId < 0) {
        getEntranceCameraParamResource(pTicket, index);
        return;
    }

    ByamlIter startTickets;

    for (s32 i = 0; i < mNumEntries; i++) {
        Entry* entry = mEntries[i];

        if (entry->zoneId == zoneId) {
            tryFindCameraParamList(&startTickets, entry->stageName.cstr(), "StartTickets");
            startTickets.tryGetIterByIndex(pTicket, index);
        }
    }
}

/**
 * Finds the resource of a stage.
 * @param pStageName Name of the stage.
 * @return Resource entry, or nullptr.
 */
CameraResourceHolder::Entry* CameraResourceHolder::findCameraResource(const char* pStageName) const {
    for (s32 i = 0; i < mNumEntries; i++) {
        if (isEqualString(pStageName, mEntries[i]->stageName)) {
            return mEntries[i];
        }
    }

    return nullptr;
}

/**
 * Finds the resource of a stage.
 * @param pStageName Name of the stage.
 * @return Resource entry, or nullptr.
 */
CameraResourceHolder::Entry*
CameraResourceHolder::tryFindCameraResource(const char* pStageName) const {
    return findCameraResource(pStageName);
}

/**
 * Finds the resource of the stage a placement belongs to.
 * @param pPlacementId Placement, or nullptr for the main stage.
 * @return Resource entry, or nullptr.
 */
CameraResourceHolder::Entry*
CameraResourceHolder::tryFindCameraResource(const PlacementId* pPlacementId) const {
    if (pPlacementId != nullptr) {
        const char* stageName = pPlacementId->mUnitConfigName;

        if (stageName == nullptr) {
            stageName = mStageName;
        }

        return findCameraResource(stageName);
    }

    return findCameraResource(mStageName);
}

}  // namespace al
