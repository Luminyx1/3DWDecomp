#include "Util/LayoutUtil.hpp"
#include <attributes.h>
#include "Library/Layout/LayoutActor.hpp"
#include "Library/Layout/LayoutActorUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Message/MessageHolder.hpp"
#include "Library/Message/ReplaceTagProcessorBase.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "System/GameDataFunction.hpp"
#include "System/GameDataHolderAccessor.hpp"

// LayoutFontUtil and ProjectReplaceTagProcessor have no header yet; only the members used by
// this unit are declared here.

namespace LayoutFontUtil {
const char16_t* getIconFontCrown();
const char16_t* getSystemFontBtnDecide(s32 port);
} // namespace LayoutFontUtil

/**
 * @brief Tag processor that resolves the project-specific picture font tags.
 */
class ProjectReplaceTagProcessor : public al::ReplaceTagProcessorBase {
  public:
    ProjectReplaceTagProcessor() = default;

    s32 replacePictureGroup(char16_t* pDst, const al::MessageTag& rTag) const override;
};

namespace {

/// Size in characters of the buffers that receive the replaced messages.
constexpr s32 cMessageBufferSize = 128;

/**
 * @brief Write the text representing a world number (special worlds use picture font icons).
 * @param pStr String receiving the world text.
 * @param worldId World number.
 */
NOINLINE void formatWorldString(sead::WBufferedSafeString* pStr, u16 worldId) {
    switch (worldId) {
    case 7:
        pStr->format(u"*");
        break;
    case 8:
        pStr->format(u"<");
        break;
    case 9:
        pStr->format(u"$");
        break;
    case 10:
        pStr->format(u"%d", worldId);
        break;
    case 11:
        pStr->format(u"(");
        break;
    case 12:
        pStr->format(u")");
        break;
    default:
        pStr->format(u"%d", worldId);
        break;
    }
}

} // namespace

namespace rc {

/**
 * @brief Get the course name message of a course.
 * @param pMsgSystem Message system used to look up the message.
 * @param pHolder Game-data holder.
 * @param courseId Course to get the name of.
 * @return The course name message.
 */
const char16_t* getMessageStringStageName(al::IUseMessageSystem* pMsgSystem,
                                          const GameDataHolder* pHolder, s32 courseId) {
    const char* stageName = GameDataFunction::findStageName(
        GameDataHolderAccessor(const_cast<GameDataHolder*>(pHolder)), courseId);
    return al::getSystemMessageString(pMsgSystem, "StageName", stageName);
}

/**
 * @brief Set the course name of a course on a text pane.
 * @param pActor Layout actor whose message system is used.
 * @param pLayout Layout containing the pane.
 * @param pPaneName Name of the text pane.
 * @param pHolder Game-data holder.
 * @param courseId Course to show the name of.
 */
void setPaneStageNameString(al::LayoutActor* pActor, al::IUseLayout* pLayout,
                            const char* pPaneName, const GameDataHolder* pHolder, s32 courseId) {
    al::setPaneString(pLayout, pPaneName, getMessageStringStageName(pActor, pHolder, courseId));
}

/**
 * @brief Set a message containing a world number on a text pane.
 * @param pActor Layout actor whose message system is used.
 * @param pLayout Layout containing the panes.
 * @param pPaneName Name of the text pane.
 * @param pFileName Message file of the message.
 * @param pLabel Label of the message.
 * @param worldId World number to insert into the message.
 * @param pSubPaneName Optional second text pane receiving the same text (may be nullptr).
 * @param isSystemMessage Whether the message is a system message instead of a layout message.
 */
void setPaneWorldString(al::LayoutActor* pActor, al::IUseLayout* pLayout, const char* pPaneName,
                        const char* pFileName, const char* pLabel, u16 worldId,
                        const char* pSubPaneName, bool isSystemMessage) {
    sead::WFixedSafeString<2> worldStr;
    formatWorldString(&worldStr, worldId);

    ProjectReplaceTagProcessor processor;
    char16_t buffer[cMessageBufferSize];
    al::IUseMessageSystem* msgSystem = pActor;
    const char16_t* message = isSystemMessage
                                  ? al::getSystemMessageString(msgSystem, pFileName, pLabel)
                                  : al::getLayoutMessageString(msgSystem, pFileName, pLabel);
    processor.replaceArgs(buffer, cMessageBufferSize, msgSystem, message, worldStr.cstr());
    al::setPaneString(pLayout, pPaneName, buffer, 0, 0);

    if (pSubPaneName != nullptr) {
        al::setPaneString(pLayout, pSubPaneName, buffer, 0, 0);
    }
}

/**
 * @brief Set a message containing a world number and a course number on a text pane.
 * @param pActor Layout actor whose message system is used.
 * @param pLayout Layout containing the panes.
 * @param pPaneName Name of the text pane.
 * @param pFileName Message file of the message.
 * @param pLabel Label of the message.
 * @param worldId World number to insert into the message.
 * @param stageId Course number inside the world (special courses use picture font icons).
 * @param pHolder Game-data holder.
 * @param pSubPaneName Optional second text pane receiving the same text (may be nullptr).
 * @param isSystemMessage Whether the message is a system message instead of a layout message.
 */
void setPaneWorldStageString(al::LayoutActor* pActor, al::IUseLayout* pLayout,
                             const char* pPaneName, const char* pFileName, const char* pLabel,
                             u16 worldId, u16 stageId, const GameDataHolder* pHolder,
                             const char* pSubPaneName, bool isSystemMessage) {
    sead::WFixedSafeString<2> worldStr;
    formatWorldString(&worldStr, worldId);

    sead::WFixedSafeString<8> stageStr;
    GameDataHolderAccessor accessor(const_cast<GameDataHolder*>(pHolder));
    s32 courseId = GameDataFunction::calcCourseId(accessor, worldId, stageId);

    if (GameDataFunction::isStageGateKeeper(accessor, courseId)) {
        switch (stageId) {
        case 100:
            stageStr.format(u"A");
            break;
        case 101:
            stageStr.format(u"B");
            break;
        case 102:
            stageStr.format(u"C");
            break;
        default:
            stageStr.format(u"0");
            break;
        }
    } else if (GameDataFunction::isStageKinopioBrigade(accessor, courseId)) {
        stageStr.format(u"=");
    } else if (GameDataFunction::isStageCasinoRoom(accessor, courseId)) {
        stageStr.format(u"]");
    } else if (GameDataFunction::isStageContinuousMysteryBox(accessor, courseId)) {
        stageStr.format(u"\\");
    } else if (GameDataFunction::isStageKinopioHouse(accessor, courseId) ||
               GameDataFunction::isStageKinopioHouseHide(accessor, courseId)) {
        stageStr.format(u">");
    } else if (GameDataFunction::isStageFairyHouse(accessor, courseId)) {
        stageStr.format(u"[");
    } else if (GameDataFunction::isStageGoldenExpress(accessor, courseId) ||
               GameDataFunction::isStageKoopaCastleExpress(accessor, courseId) ||
               GameDataFunction::isStageKoopaCastleExpressNormal(accessor, courseId)) {
        stageStr.format(u"|");
    } else if (GameDataFunction::isStageKoopaCastleTank(accessor, courseId)) {
        stageStr.format(u"}");
    } else if (GameDataFunction::isStageKoopaCastleFortress(accessor, courseId) ||
               GameDataFunction::isStageKoopaCastle(accessor, courseId)) {
        stageStr.format(u"{");
    } else if (GameDataFunction::isStageChampionShip(accessor, courseId)) {
        stageStr.format(u")");
    } else {
        stageStr.format(u"%d", stageId);
    }

    ProjectReplaceTagProcessor processor;
    char16_t buffer[cMessageBufferSize];
    al::IUseMessageSystem* msgSystem = pActor;
    const char16_t* message = isSystemMessage
                                  ? al::getSystemMessageString(msgSystem, pFileName, pLabel)
                                  : al::getLayoutMessageString(msgSystem, pFileName, pLabel);
    processor.replaceArgs(buffer, cMessageBufferSize, msgSystem, message, worldStr.cstr(),
                          stageStr.cstr());
    al::setPaneString(pLayout, pPaneName, buffer);

    if (pSubPaneName != nullptr) {
        al::setPaneString(pLayout, pSubPaneName, buffer, 0, 0);
    }
}

/**
 * @brief Set a layout message with one number argument on a text pane.
 * @param pMsgSystem Message system used to look up and replace the message.
 * @param pLayout Layout containing the pane.
 * @param pPaneName Name of the text pane.
 * @param pFileName Message file of the message.
 * @param pLabel Label of the message.
 * @param number Number to insert into the message.
 */
void replacePaneMsgNumber(al::IUseMessageSystem* pMsgSystem, al::IUseLayout* pLayout,
                          const char* pPaneName, const char* pFileName, const char* pLabel,
                          s32 number) {
    ProjectReplaceTagProcessor processor;
    char16_t buffer[cMessageBufferSize];
    const char16_t* message = al::getLayoutMessageString(pMsgSystem, pFileName, pLabel);
    processor.replaceArgs(buffer, cMessageBufferSize, pMsgSystem, message, number);
    al::setPaneString(pLayout, pPaneName, buffer);
}

/**
 * @brief Set a layout message with two number arguments on a text pane.
 * @param pMsgSystem Message system used to look up and replace the message.
 * @param pLayout Layout containing the pane.
 * @param pPaneName Name of the text pane.
 * @param pFileName Message file of the message.
 * @param pLabel Label of the message.
 * @param number1 First number to insert into the message.
 * @param number2 Second number to insert into the message.
 */
void replacePaneMsgNumber2(al::IUseMessageSystem* pMsgSystem, al::IUseLayout* pLayout,
                           const char* pPaneName, const char* pFileName, const char* pLabel,
                           s32 number1, s32 number2) {
    ProjectReplaceTagProcessor processor;
    char16_t buffer[cMessageBufferSize];
    const char16_t* message = al::getLayoutMessageString(pMsgSystem, pFileName, pLabel);
    processor.replaceArgs(buffer, cMessageBufferSize, pMsgSystem, message, number1, number2);
    al::setPaneString(pLayout, pPaneName, buffer);
}

/**
 * @brief Set a layout message with three number arguments on a text pane.
 * @param pMsgSystem Message system used to look up and replace the message.
 * @param pLayout Layout containing the pane.
 * @param pPaneName Name of the text pane.
 * @param pFileName Message file of the message.
 * @param pLabel Label of the message.
 * @param number1 First number to insert into the message.
 * @param number2 Second number to insert into the message.
 * @param number3 Third number to insert into the message.
 */
void replacePaneMsgNumber3(al::IUseMessageSystem* pMsgSystem, al::IUseLayout* pLayout,
                           const char* pPaneName, const char* pFileName, const char* pLabel,
                           s32 number1, s32 number2, s32 number3) {
    ProjectReplaceTagProcessor processor;
    char16_t buffer[cMessageBufferSize];
    const char16_t* message = al::getLayoutMessageString(pMsgSystem, pFileName, pLabel);
    processor.replaceArgs(buffer, cMessageBufferSize, pMsgSystem, message, number1, number2,
                          number3);
    al::setPaneString(pLayout, pPaneName, buffer);
}

/**
 * @brief Convert a life count to text; each thousand, hundred and ten digit at or above the
 * display limit is shown as a crown icon.
 * @param pOut String receiving the text.
 * @param life Life count to convert.
 */
void convertPlayerLifeToText(sead::WBufferedSafeString* pOut, s32 life) {
    pOut->clear();
    sead::WFixedSafeString<6> numberStr;

    if (life >= 1000) {
        pOut->append(sead::WSafeString(LayoutFontUtil::getIconFontCrown()));
        life %= 1000;

        if (life >= 100) {
            pOut->append(sead::WSafeString(LayoutFontUtil::getIconFontCrown()));
            life %= 100;

            if (life >= 10) {
                pOut->append(sead::WSafeString(LayoutFontUtil::getIconFontCrown()));
            } else {
                numberStr.format(u"%d", life);
            }
        } else {
            numberStr.format(u"%d", life);
        }
    } else {
        numberStr.format(u"%d", life);
    }

    pOut->append(numberStr);
}

/**
 * @brief Set the decide button icon of a controller on a text pane.
 * @param pLayout Layout containing the pane.
 * @param pPaneName Name of the text pane.
 * @param port Controller port whose decide button is shown.
 */
void setPaneDecideIconFont(al::IUseLayout* pLayout, const char* pPaneName, s32 port) {
    al::setPaneString(pLayout, pPaneName, LayoutFontUtil::getSystemFontBtnDecide(port));
}

/**
 * @brief Hide a button of a button group (does nothing).
 * @param pButtonGroup Button group containing the button.
 * @param pButtonName Name of the button.
 */
void hideButton(ButtonGroup* pButtonGroup, const char* pButtonName) {}

/**
 * @brief Move a layout to the layout position of a world position; a position behind the camera
 * moves the layout out of the screen.
 * @param pActor Layout actor to move.
 * @param pCamera Camera used for the projection.
 * @param rWorldPos World position to follow.
 * @param range Range the layout position is clamped to outside the screen.
 */
void updateLayoutPosAtWorldPos(al::LayoutActor* pActor, const al::IUseCamera* pCamera,
                               const sead::Vector3f& rWorldPos, f32 range) {
    sead::Vector3f layoutPos;
    al::calcLayoutPosFromWorldPosWithClampOutRange(&layoutPos, pCamera, rWorldPos, range, 0);
    sead::Vector2f trans(layoutPos.x, layoutPos.y);

    if (layoutPos.z > 0.0f) {
        trans.x = -1000 - al::getLayoutDisplayWidth();
    }

    al::setLocalTrans(pActor, trans);
}

/**
 * @brief Move a layout to the layout position of a world position.
 * @param pActor Layout actor to move.
 * @param pCamera Camera used for the projection.
 * @param rWorldPos World position to follow.
 * @return Whether the new layout position is inside the screen.
 */
bool updateLayoutPosAtWorldPosWithInCheck(al::LayoutActor* pActor, const al::IUseCamera* pCamera,
                                          const sead::Vector3f& rWorldPos) {
    sead::Vector3f layoutPos;
    al::calcLayoutPosFromWorldPos(&layoutPos, pCamera, rWorldPos);
    sead::Vector2f trans(layoutPos.x, layoutPos.y);
    al::setLocalTrans(pActor, trans);

    f32 halfWidth = al::getLayoutDisplayWidth() * 0.5f;
    f32 halfHeight = al::getLayoutDisplayHeight() * 0.5f;

    if (layoutPos.x > -halfWidth && trans.x < halfWidth && layoutPos.y > -halfHeight &&
        trans.y < halfHeight) {
        return true;
    }

    return false;
}

/**
 * @brief Move a layout to the layout position of an actor.
 * @param pActor Layout actor to move.
 * @param pOwner Actor to follow.
 * @param rOffset Offset added to the owner's position.
 * @param range Range the layout position is clamped to outside the screen.
 */
void updateLayoutPosAtOwner(al::LayoutActor* pActor, const al::LiveActor* pOwner,
                            const sead::Vector3f& rOffset, f32 range) {
    sead::Vector3f worldPos = al::getTrans(pOwner) + rOffset;
    updateLayoutPosAtWorldPos(pActor, pOwner, worldPos, range);
}

/**
 * @brief Move a layout to the layout position of an actor's joint.
 * @param pActor Layout actor to move.
 * @param pOwner Actor to follow.
 * @param pJointName Joint of the owner to follow.
 * @param rOffset Offset added to the joint position.
 * @param range Range the layout position is clamped to outside the screen.
 */
void updateLayoutPosAtOwner(al::LayoutActor* pActor, const al::LiveActor* pOwner,
                            const char* pJointName, const sead::Vector3f& rOffset, f32 range) {
    sead::Vector3f jointPos;
    al::calcJointPos(&jointPos, pOwner, pJointName);
    sead::Vector3f worldPos = jointPos + rOffset;
    updateLayoutPosAtWorldPos(pActor, pOwner, worldPos, range);
}

} // namespace rc
