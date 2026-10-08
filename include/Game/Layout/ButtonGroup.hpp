#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutActor;
class LayoutInitInfo;
}
class CursorTarget;

/**
 * @brief Layout button collection and cursor controller.
 * @note Storage between the button list and the selected button remains opaque; its size is
 *       verified from allocation sites.
 */
class alignas(8) ButtonGroup {
public:
    ButtonGroup(const al::LayoutInitInfo& rInfo, al::LayoutActor* pParent,
                const char* pLayoutName, const char* pCursorName, bool flag);
    void registerButton(al::LayoutActor* pParent, CursorTarget* pButton);
    void resetDestination(const char* pButtonName, const char* pUpName, const char* pDownName,
                          const char* pLeftName, const char* pRightName, bool isLoop);
    void validate();
    void invalidate();
    bool isDecideAny() const;
    bool isDecideEndAny() const;
    s32 getDecideEndButtonIndex() const;
    const char* getDecideButton() const;
    CursorTarget* getButton(const char* pButtonName) const;
    bool isSelect(CursorTarget* pButton) const;
    /** @brief Advances the button cursor layout. */
    void updateCursor() { mCursor->movement(); }
    void setPort(s32 port);
    void hideCursor();
    void showCursor();
    void reset();
    void select(const char* pButtonName);
    void select(s32 index);
    void select(CursorTarget* pButton);
    void updateAndCursorDefault(s32 port);
    bool isDecide(const char* pButtonName) const;

    /**
     * @brief Access the cursor layout.
     * @return The cursor layout actor.
     */
    al::LayoutActor* getCursor() const { return mCursor; }

    /**
     * @brief Read the number of registered buttons.
     * @return The button count.
     */
    s32 getButtonNum() const { return mButtons.size(); }

    /**
     * @brief Access a registered button with a bounds check.
     * @param index Registration index of the button.
     * @return The button, or nullptr when the index is out of range.
     */
    CursorTarget* getButton(s32 index) const { return mButtons.at(index); }

    /**
     * @brief Access a registered button without a bounds check.
     * @param index Registration index of the button.
     * @return The button.
     */
    CursorTarget* getButtonUnsafe(s32 index) const { return mButtons.unsafeAt(index); }

    /**
     * @brief Access the button the cursor is currently on.
     * @return The selected button.
     */
    CursorTarget* getSelectedButton() const { return mSelectedButton; }

private:
    al::LayoutActor* mCursor;
    sead::PtrArray<CursorTarget> mButtons;
    u8 mUnreconstructed18[0x210];
    CursorTarget* mSelectedButton;
    u8 mUnreconstructed230[0x20];
};
static_assert(sizeof(ButtonGroup) == 0x250);
