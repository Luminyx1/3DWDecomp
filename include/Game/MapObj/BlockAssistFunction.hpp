#pragma once
class BlockAssistLeaf;

namespace BlockAssistFunction {
void setBlockAssist(BlockAssistLeaf* pBlock);
bool isBlockAssistSetCheckPoint(const BlockAssistLeaf* pBlock);
bool isBlockAssistSetStartPoint(const BlockAssistLeaf* pBlock);
bool isEnableAppearBlockAssist(const BlockAssistLeaf* pBlock);
}
