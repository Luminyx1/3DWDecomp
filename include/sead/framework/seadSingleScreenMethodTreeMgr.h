#pragma once

#include <framework/seadMethodTree.h>
#include <framework/seadMethodTreeMgr.h>

namespace sead
{
class SingleScreenMethodTreeMgr : public MethodTreeMgr
{
    SEAD_RTTI_OVERRIDE(SingleScreenMethodTreeMgr, MethodTreeMgr)

public:
    SingleScreenMethodTreeMgr();
    ~SingleScreenMethodTreeMgr() override;

    void attachMethod(s32 methodType, MethodTreeNode* pNode) override;
    MethodTreeNode* getRootMethodTreeNode(s32 methodType) override;
    void pauseAll(bool pause) override;
    void pauseAppCalc(bool pause) override;

    void calc();
    void draw();

private:
    MethodTreeNode mRootCalcNode{&mCS};
    MethodTreeNode mSysCalcNode{&mCS};
    MethodTreeNode mAppCalcNode{&mCS};
    MethodTreeNode mRootDrawNode{&mCS};
    MethodTreeNode mSysDrawNode{&mCS};
    MethodTreeNode mAppDrawNode{&mCS};
    MethodTreeNode mAppDrawFinalNode{&mCS};
};

}  // namespace sead
