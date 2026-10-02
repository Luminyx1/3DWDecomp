#pragma once

namespace al {
class AnimInfoTable;
class Resource;

class InitResourceDataAnim {
public:
    static InitResourceDataAnim* tryCreate(Resource* pModelRes, Resource* pAnimRes,
                                           Resource* pOtherRes);

    InitResourceDataAnim(Resource* pResource, const AnimInfoTable* pSklAnimInfoTable,
                         const AnimInfoTable* pMclAnimInfoTable,
                         const AnimInfoTable* pMtsAnimInfoTable,
                         const AnimInfoTable* pMtpAnimInfoTable,
                         const AnimInfoTable* pVisAnimInfoTable);

    AnimInfoTable* getSklAnimInfoTable() const { return mSklAnimInfoTable; }
    AnimInfoTable* getMclAnimInfoTable() const { return mMclAnimInfoTable; }
    AnimInfoTable* getMtsAnimInfoTable() const { return mMtsAnimInfoTable; }
    AnimInfoTable* getMtpAnimInfoTable() const { return mMtpAnimInfoTable; }
    AnimInfoTable* getVisAnimInfoTable() const { return mVisAnimInfoTable; }

private:
    AnimInfoTable* mSklAnimInfoTable;
    AnimInfoTable* mMclAnimInfoTable;
    AnimInfoTable* mMtsAnimInfoTable;
    AnimInfoTable* mMtpAnimInfoTable;
    AnimInfoTable* mVisAnimInfoTable;
};
}  // namespace al
