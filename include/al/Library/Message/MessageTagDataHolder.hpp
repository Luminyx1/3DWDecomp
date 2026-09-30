#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace al {
class IUseMessageSystem;
class MessageTagDataBase;

class MessageTagDataHolder {
public:
    MessageTagDataHolder(s32 maxNum);

    void registerMessageTagData(MessageTagDataBase* pData);
    void replaceMessage(sead::BufferedSafeStringBase<char16_t>* pString,
                        const IUseMessageSystem* pMsgSystem, const char16_t* pMessage) const;

private:
    MessageTagDataBase** mData;
    s32 mNum;
    s32 mMaxNum;
};
}  // namespace al
