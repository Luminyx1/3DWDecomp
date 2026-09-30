#include "Library/Execute/ExecuteDirector.hpp"

#include "Library/Execute/ActorExecuteInfo.hpp"
#include "Library/Execute/ExecuteRequestKeeper.hpp"
#include "Library/Execute/ExecuteTableHolderDraw.hpp"
#include "Library/Execute/ExecuteTableHolderUpdate.hpp"
#include "Library/Execute/ExecuteTablesImpl.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs the execute director.
 * @param requestCount Maximum number of execute requests.
 * @param isUseMultiCore Whether to calculate actor anims on another core.
 */
ExecuteDirector::ExecuteDirector(s32 requestCount, bool isUseMultiCore)
    : mRequestCount(requestCount), mIsUseMultiCore(isUseMultiCore) {}

ExecuteDirector::~ExecuteDirector() {
    for (s32 i = 0; i < mDrawTableNum; i++) {
        if (mDrawTables[i]) {
            delete mDrawTables[i];
        }
    }

    if (mUpdateTable) {
        delete mUpdateTable;
    }
}

/**
 * Creates the update and draw tables and the request keeper.
 * @param rInfo Execute system init info.
 */
void ExecuteDirector::init(const ExecuteSystemInitInfo& rInfo) {
    mUpdateTable = new ExecuteTableHolderUpdate();
    mUpdateTable->init(rInfo, sUpdateTable, 75, mIsUseMultiCore);

    mDrawTableNum = 28;
    mDrawTables = new ExecuteTableHolderDraw*[mDrawTableNum];

    for (s32 i = 0; i < mDrawTableNum; i++) {
        mDrawTables[i] = new ExecuteTableHolderDraw();
    }

    mDrawTables[0]->init("３Ｄ（デプスシャドウ）", rInfo, sDrawTable0, 2);
    mDrawTables[1]->init("３Ｄ（デプスシャドウプレイヤー）", rInfo, sDrawTable1, 2);
    mDrawTables[2]->init("３Ｄ（デプスシャドウオブジェ）", rInfo, sDrawTable2, 9);
    mDrawTables[3]->init("３Ｄ（空）", rInfo, sDrawTable3, 1);
    mDrawTables[4]->init("３Ｄ（撮影用）", rInfo, sDrawTable4, 2);
    mDrawTables[5]->init("３Ｄ（ディファード地形）", rInfo, sDrawTable5, 15);
    mDrawTables[6]->init("３Ｄ（ディファードオブジェ）", rInfo, sDrawTable6, 30);
    mDrawTables[7]->init("３Ｄ（ディファード遠景）", rInfo, sDrawTable7, 2);
    mDrawTables[8]->init("3D (Deferred Water)", rInfo, sDrawTable8, 1);
    mDrawTables[9]->init("３Ｄ（フォワードプレイヤー）", rInfo, sDrawTable9, 4);
    mDrawTables[10]->init("３Ｄ（フォワード）", rInfo, sDrawTable10, 9);
    mDrawTables[11]->init("３Ｄ（インダイレクト）", rInfo, sDrawTable11, 2);
    mDrawTables[12]->init("ShadowMask2ndPass", rInfo, sDrawTable12, 5);
    mDrawTables[13]->init("3D (PostIndirect)", rInfo, sDrawTable13, 11);
    mDrawTables[14]->init("3D (PreIndirect)", rInfo, sDrawTable14, 1);
    mDrawTables[15]->init("２Ｄベース（メイン画面）", rInfo, sDrawTable15, 12);
    mDrawTables[16]->init("２Ｄベース（サブ画面）", rInfo, sDrawTable16, 13);
    mDrawTables[17]->init("２Ｄミス（メイン画面）", rInfo, sDrawTable17, 6);
    mDrawTables[18]->init("２Ｄミス（サブ画面）", rInfo, sDrawTable18, 6);
    mDrawTables[19]->init("2DDrawAboveBlur1", rInfo, sDrawTable19, 3);
    mDrawTables[20]->init("2DDrawAboveBlur1Pause", rInfo, sDrawTable20, 2);
    mDrawTables[21]->init("2DDrawAboveBlur2", rInfo, sDrawTable21, 1);
    mDrawTables[22]->init("ポストエフェクトマスク", rInfo, sDrawTable22, 1);
    mDrawTables[23]->init("ShadowMaskPreAdd", rInfo, sDrawTable23, 1);
    mDrawTables[24]->init("2DDrawSequence", rInfo, sDrawTable24, 1);

    mRequestKeeper = new ExecuteRequestKeeper(mRequestCount);
}

/**
 * Registers an actor to the update lists with the given name.
 * @param pActor The actor.
 * @param pListName List name.
 */
void ExecuteDirector::registerActorUpdate(LiveActor* pActor, const char* pListName) {
    if (!pActor->mActorExecuteInfo) {
        pActor->initExecuteInfo(new ActorExecuteInfo(mRequestKeeper));
    }

    mUpdateTable->registerActor(pActor, pListName);
}

/**
 * Registers an actor and its model to the draw lists with the given name.
 * @param pActor The actor.
 * @param pListName List name.
 */
void ExecuteDirector::registerActorDraw(LiveActor* pActor, const char* pListName) {
    if (!pActor->mActorExecuteInfo) {
        pActor->initExecuteInfo(new ActorExecuteInfo(mRequestKeeper));
    }

    for (s32 i = 0; i < mDrawTableNum; i++) {
        mDrawTables[i]->tryRegisterActor(pActor, pListName);
        mDrawTables[i]->tryRegisterActorModel(pActor, pListName);
    }
}

/**
 * Registers an actor model to the draw lists with the given name.
 * @param pActor The actor.
 * @param pListName List name.
 */
void ExecuteDirector::registerActorModelDraw(LiveActor* pActor, const char* pListName) {
    if (!pActor->mActorExecuteInfo) {
        pActor->initExecuteInfo(new ActorExecuteInfo(mRequestKeeper));
    }

    for (s32 i = 0; i < mDrawTableNum; i++) {
        mDrawTables[i]->tryRegisterActorModel(pActor, pListName);
    }
}

/**
 * Registers a layout to the update lists with the given name.
 * @param pLayout The layout.
 * @param pListName List name.
 */
void ExecuteDirector::registerLayoutUpdate(LayoutActor* pLayout, const char* pListName) {
    mUpdateTable->registerLayout(pLayout, pListName);
}

/**
 * Registers a layout to the draw lists with the given name.
 * @param pLayout The layout.
 * @param pListName List name.
 */
void ExecuteDirector::registerLayoutDraw(LayoutActor* pLayout, const char* pListName) {
    for (s32 i = 0; i < mDrawTableNum; i++) {
        mDrawTables[i]->tryRegisterLayout(pLayout, pListName);
    }
}

/**
 * Registers an executor user to the update and draw lists with the given name.
 * @param pUser The user.
 * @param pListName List name.
 */
void ExecuteDirector::registerUser(IUseExecutor* pUser, const char* pListName) {
    mUpdateTable->tryRegisterUser(pUser, pListName);

    for (s32 i = 0; i < mDrawTableNum; i++) {
        mDrawTables[i]->tryRegisterUser(pUser, pListName);
    }
}

/**
 * Registers a functor to the update and draw lists with the given name.
 * @param rFunctor The functor.
 * @param pListName List name.
 */
void ExecuteDirector::registerFunctor(const FunctorBase& rFunctor, const char* pListName) {
    mUpdateTable->tryRegisterFunctor(rFunctor, pListName);

    for (s32 i = 0; i < mDrawTableNum; i++) {
        mDrawTables[i]->tryRegisterFunctor(rFunctor, pListName);
    }
}

/**
 * Registers a functor to the draw lists with the given name.
 * @param rFunctor The functor.
 * @param pListName List name.
 */
void ExecuteDirector::registerFunctorDraw(const FunctorBase& rFunctor, const char* pListName) {
    for (s32 i = 0; i < mDrawTableNum; i++) {
        mDrawTables[i]->tryRegisterFunctor(rFunctor, pListName);
    }
}

/**
 * Creates the executor tables of all tables.
 */
void ExecuteDirector::createExecutorListTable() {
    mUpdateTable->createExecutorListTable();

    for (s32 i = 0; i < mDrawTableNum; i++) {
        mDrawTables[i]->createExecutorListTable();
    }
}

/**
 * Handles the pending execute requests and executes the update table.
 */
void ExecuteDirector::execute() const {
    mRequestKeeper->executeRequestActorMovementAllOn();
    mRequestKeeper->executeRequestActorDrawAllOn();
    mUpdateTable->execute();
    mRequestKeeper->executeRequestActorMovementAllOff();
    mRequestKeeper->executeRequestActorDrawAllOff();
    mRequestKeeper->executeRequestActorDrawAllOnImmediate();
}

/**
 * Waits for the update table to finish.
 */
void ExecuteDirector::finishExecute() const {
    mUpdateTable->finishExecute();
}

/**
 * Handles the pending execute requests and executes an update list.
 * @param pListName List name.
 */
void ExecuteDirector::executeList(const char* pListName) const {
    mRequestKeeper->executeRequestActorMovementAllOn();
    mRequestKeeper->executeRequestActorDrawAllOn();
    mUpdateTable->executeList(pListName);
    mRequestKeeper->executeRequestActorMovementAllOff();
    mRequestKeeper->executeRequestActorDrawAllOff();
    mRequestKeeper->executeRequestActorDrawAllOnImmediate();
}

/**
 * Handles the pending execute requests and executes an update list while paused.
 * @param pListName List name.
 */
void ExecuteDirector::executeListPaused(const char* pListName) const {
    mRequestKeeper->executeRequestActorMovementAllOn();
    mRequestKeeper->executeRequestActorDrawAllOn();
    mUpdateTable->executeListPaused(pListName);
    mRequestKeeper->executeRequestActorMovementAllOff();
    mRequestKeeper->executeRequestActorDrawAllOff();
    mRequestKeeper->executeRequestActorDrawAllOnImmediate();
}

/**
 * Executes an update list without handling execute requests.
 * @param pListName List name.
 */
void ExecuteDirector::executeListStall(const char* pListName) const {
    mUpdateTable->executeList(pListName);
}

/**
 * Executes a draw table.
 * @param pTableName Table name, or nullptr for the first table.
 */
void ExecuteDirector::draw(const char* pTableName) const {
    if (!pTableName) {
        mDrawTables[0]->execute();
        return;
    }

    for (s32 i = 0; i < mDrawTableNum; i++) {
        if (isEqualString(pTableName, mDrawTables[i]->getName())) {
            mDrawTables[i]->execute();
            return;
        }
    }
}

/**
 * Executes a list of a draw table.
 * @param pTableName Table name.
 * @param pListName List name.
 */
void ExecuteDirector::drawList(const char* pTableName, const char* pListName) const {
    for (s32 i = 0; i < mDrawTableNum; i++) {
        if (isEqualString(pTableName, mDrawTables[i]->getName())) {
            mDrawTables[i]->executeList(pListName);
            return;
        }
    }
}

/**
 * Checks whether a draw table is active.
 * @param pTableName Table name, or nullptr for the first table.
 * @return True if active.
 */
bool ExecuteDirector::isActiveDraw(const char* pTableName) const {
    if (!pTableName) {
        return mDrawTables[0]->isActive();
    }

    for (s32 i = 0; i < mDrawTableNum; i++) {
        if (isEqualString(pTableName, mDrawTables[i]->getName())) {
            return mDrawTables[i]->isActive();
        }
    }

    return false;
}
}  // namespace al
