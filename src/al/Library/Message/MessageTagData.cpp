#include "Library/Message/MessageTagData.hpp"

#include "Library/Message/MessageHolder.hpp"

namespace al {
/**
 * Replaces the score tag of a message with the registered score.
 * @param pString output string
 * @param pMsgSystem message system
 * @param pMessage source message
 */
void MessageTagDataScore::replaceMessage(sead::BufferedSafeStringBase<char16_t>* pString,
                                         const IUseMessageSystem* pMsgSystem,
                                         const char16_t* pMessage) const {
    replaceMessageTagScore(pString, pMsgSystem, pMessage, *mScore, mName);
}

/**
 * Replaces the coin number tag of a message with the registered coin number.
 * @param pString output string
 * @param pMsgSystem message system
 * @param pMessage source message
 */
void MessageTagDataCoinNum::replaceMessage(sead::BufferedSafeStringBase<char16_t>* pString,
                                           const IUseMessageSystem* pMsgSystem,
                                           const char16_t* pMessage) const {
    replaceMessageTagCoinNum(pString, pMsgSystem, pMessage, *mCoinNum, mName);
}

/**
 * Replaces the user name tag of a message with the registered user name.
 * @param pString output string
 * @param pMsgSystem message system
 * @param pMessage source message
 */
void MessageTagDataUserName::replaceMessage(sead::BufferedSafeStringBase<char16_t>* pString,
                                            const IUseMessageSystem* pMsgSystem,
                                            const char16_t* pMessage) const {
    replaceMessageTagUserName(pString, pMsgSystem, pMessage, *mUserName, mName);
}

/**
 * Replaces the named string tag of a message with the registered string.
 * @param pString output string
 * @param pMsgSystem message system
 * @param pMessage source message
 */
void MessageTagDataString::replaceMessage(sead::BufferedSafeStringBase<char16_t>* pString,
                                          const IUseMessageSystem* pMsgSystem,
                                          const char16_t* pMessage) const {
    replaceMessageTagNamedString(pString, pMsgSystem, pMessage, *mString, mName);
}

/**
 * Replaces the amiibo name tag of a message with the registered amiibo name.
 * @param pString output string
 * @param pMsgSystem message system
 * @param pMessage source message
 */
void MessageTagDataAmiiboName::replaceMessage(sead::BufferedSafeStringBase<char16_t>* pString,
                                              const IUseMessageSystem* pMsgSystem,
                                              const char16_t* pMessage) const {
    replaceMessageTagAmiiboName(pString, pMsgSystem, pMessage, *mAmiiboName, mName);
}
}  // namespace al
