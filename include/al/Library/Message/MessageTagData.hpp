#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace al {
class IUseMessageSystem;

class MessageTagDataBase {
public:
    virtual void replaceMessage(sead::WBufferedSafeString* pString,
                                const IUseMessageSystem* pMsgSystem,
                                const char16_t* pMessage) const = 0;
};

class MessageTagDataScore : public MessageTagDataBase {
public:
    MessageTagDataScore(const char* pName, const s32* pScore) : mScore(pScore), mName(pName) {}

    void replaceMessage(sead::WBufferedSafeString* pString,
                        const IUseMessageSystem* pMsgSystem,
                        const char16_t* pMessage) const override;

private:
    const s32* mScore;
    const char* mName;
};

class MessageTagDataCoinNum : public MessageTagDataBase {
public:
    MessageTagDataCoinNum(const char* pName, const s32* pCoinNum) : mCoinNum(pCoinNum), mName(pName) {}

    void replaceMessage(sead::WBufferedSafeString* pString,
                        const IUseMessageSystem* pMsgSystem,
                        const char16_t* pMessage) const override;

private:
    const s32* mCoinNum;
    const char* mName;
};

class MessageTagDataUserName : public MessageTagDataBase {
public:
    MessageTagDataUserName(const char* pName, const char16_t** pUserName)
        : mUserName(pUserName), mName(pName) {}

    void replaceMessage(sead::WBufferedSafeString* pString,
                        const IUseMessageSystem* pMsgSystem,
                        const char16_t* pMessage) const override;

private:
    const char16_t** mUserName;
    const char* mName;
};

class MessageTagDataString : public MessageTagDataBase {
public:
    MessageTagDataString(const char* pName, const char16_t** pString) : mString(pString), mName(pName) {}

    void replaceMessage(sead::WBufferedSafeString* pString,
                        const IUseMessageSystem* pMsgSystem,
                        const char16_t* pMessage) const override;

private:
    const char16_t** mString;
    const char* mName;
};

class MessageTagDataAmiiboName : public MessageTagDataBase {
public:
    MessageTagDataAmiiboName(const char* pName, const char** pAmiiboName)
        : mAmiiboName(pAmiiboName), mName(pName) {}

    void replaceMessage(sead::WBufferedSafeString* pString,
                        const IUseMessageSystem* pMsgSystem,
                        const char16_t* pMessage) const override;

private:
    const char** mAmiiboName;
    const char* mName;
};
}  // namespace al
