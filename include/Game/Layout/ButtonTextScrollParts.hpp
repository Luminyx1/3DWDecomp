#pragma once

#include <basis/seadTypes.h>
#include "Layout/CursorTarget.hpp"

namespace al {
class LayoutInitInfo;
}  // namespace al

/**
 * @brief Cursor-selectable option button that scrolls left/right through a list of message labels.
 * @note Only the members used by already-decompiled callers are declared.
 */
class ButtonTextScrollParts : public CursorTarget {
public:
    ButtonTextScrollParts(const al::LayoutInitInfo& rInfo, const char* pName,
                          const char* pPartsName, al::LayoutActor* pParent,
                          const char* pMessageArchive, const char** pLabels, s32 labelNum);

    void control() override;
    void select() override;
    bool isEnableControl() const;
    void decide() override;
    void wait() override;
    void enable() override;
    void disable() override;
    bool isDisable() const override;
    bool isDecide() const override;
    bool isDecideEnd() const override;
    bool isTouch() const override;
    bool up() override;
    bool down() override;
    bool left() override;
    bool right() override;
    void setLabelIdx(s32 index);
    bool isValid() const override;
    void invalidate() override;
    void validate() override;

    /**
     * @brief Read the index of the currently shown label.
     * @return The label index.
     */
    s32 getLabelIdx() const { return mLabelIdx; }

private:
    u8 mUnreconstructed12c[0x20];
    s32 mLabelIdx;
    u8 mUnreconstructed150[0x8];
};
static_assert(sizeof(ButtonTextScrollParts) == 0x158);
