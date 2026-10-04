#include "System/RootTask.hpp"
#include "System/GameSystem.hpp"
#include "al/Library/Controller/NpadController.hpp"
#include "al/Library/Memory/HeapUtil.hpp"
#include "al/Library/Memory/Util.hpp"
#include "al/Library/Resource/Resource.hpp"
#include "al/Library/Resource/ResourceFunction.hpp"
#include <agl/common/aglInitArg.h>
#include <controller/seadControllerMgr.h>
#include <framework/seadFramework.h>
#include <framework/seadTaskMgr.h>
#include <gfx/seadPrimitiveRenderer.h>
#include <heap/seadHeapMgr.h>
#include <resource/seadArchiveRes.h>
#include <resource/seadResourceMgr.h>

/**
 * @brief Load a nested archive from an already loaded archive.
 * @param pArchive Archive containing the nested archive.
 * @param rPath Path of the nested archive inside pArchive.
 * @return The loaded nested archive, or nullptr if it is not an archive.
 */
static sead::ArchiveRes* loadArchive(sead::ArchiveRes* pArchive, const sead::SafeString& rPath) {
    sead::ResourceMgr::LoadArg loadArg;
    loadArg.path = rPath;
    return sead::DynamicCast<sead::ArchiveRes>(pArchive->load(loadArg));
}

/**
 * @brief Create the root game task.
 * @param rArg Task framework construction parameters.
 */
RootTask::RootTask(const sead::TaskConstructArg& rArg)
    : sead::Task(rArg, "RootTask"), mpGameSystem(nullptr) {}

/**
 * @brief Create the framework system tasks, configure the Npad controllers and load the shared
 * primitive-renderer and agl resources.
 */
void RootTask::prepare() {
    {
        sead::Framework::CreateSystemTaskArg systemTaskArg;
        getTaskMgr()->mParentFramework->createSystemTasks(this, systemTaskArg);
    }

    sead::ControllerMgr* pControllerMgr = sead::ControllerMgr::instance();
    s32 controllerNum = pControllerMgr->getControllerNum();
    for (s32 i = 0; i < controllerNum; i++) {
        al::NpadController* pController =
            sead::DynamicCast<al::NpadController>(pControllerMgr->getController(i));
        if (pController != nullptr) {
            pController->setUnknown184(1);
        }
    }

    sead::PrimitiveRenderer::createInstance(nullptr);
    sead::PrimitiveRenderer::instance()->prepare(al::getCurrentHeap(),
                                                 "SystemData/primitive_drawer_nvn_shader.bin");

    al::Resource* pAglResource = al::findOrCreateResource("SystemData/AglResource", nullptr);
    sead::ArchiveRes* pAglArchive =
        loadArchive(pAglResource->getFileArchive(), "agl_resource.Nin_NX_NVN.release.sarc");
    agl::AppendRootNodeToOR(nullptr);
    agl::LoadResource(pAglArchive);

    al::addNamedHeap(mHeapArray.getPrimaryHeap(), "RootTaskHeap");
    adjustHeapAll();
}

/**
 * @brief Enter the prepared root task; no additional work is needed.
 */
void RootTask::enter() {}

/**
 * @brief Create the game system on the stationed heap on the first frame, then update it.
 */
void RootTask::calc() {
    if (mpGameSystem == nullptr) {
        sead::ScopedCurrentHeapSetter heapSetter(al::getStationedHeap());
        mpGameSystem = new GameSystem();
        mpGameSystem->init();
    }

    mpGameSystem->movement();
}

/**
 * @brief Draw the main game-system view when initialized.
 */
void RootTask::drawTop() {
    if (mpGameSystem != nullptr) {
        mpGameSystem->drawMain();
    }
}

/**
 * @brief Draw the sub game-system view when initialized.
 */
void RootTask::drawBtm() {
    if (mpGameSystem != nullptr) {
        mpGameSystem->drawSub();
    }
}
