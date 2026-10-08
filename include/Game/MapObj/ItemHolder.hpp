#pragma once

#include <basis/seadTypes.h>

namespace al {
class ActorInitInfo;
}  // namespace al
class AssistLeaf;
class Ball;
class Bomb;
class BoomerangFlower;
class CoinBlow;
class CoinCountUp;
class DoorKey;
class DoubleMario;
class FireFlower;
class GigaBell;
class KinokoBig;
class KinokoGiga;
class KinokoOneUp;
class KinokoSuper;
class KinokoTreasure;
class SuperBell;
class SuperBellSpecial;
class SuperLeaf;
class SuperStar;
class WhiteBell;

/**
 * @brief Pool of the items the item director can spawn at runtime.
 * @note Only the interface used by reconstructed code is declared so far.
 */
class ItemHolder {
public:
    ItemHolder(const al::ActorInitInfo& rInfo, bool isSingleMode);

    void declareItem(const char* pName, const al::ActorInitInfo& rInfo);

    CoinBlow* getCoinBlow() const;
    CoinCountUp* getCoinCountUp() const;
    KinokoOneUp* getKinokoOneUp() const;
    KinokoSuper* getKinokoSuper() const;
    KinokoTreasure* getKinokoTreasure() const;
    SuperBell* getSuperBell() const;
    FireFlower* getFireFlower() const;
    BoomerangFlower* getBoomerangFlower() const;
    SuperLeaf* getSuperLeaf() const;
    SuperStar* getSuperStar() const;
    Ball* getBall() const;
    Bomb* getBomb() const;
    DoubleMario* getDoubleMario() const;
    KinokoBig* getKinokoBig() const;
    AssistLeaf* getAssistLeaf() const;
    SuperBellSpecial* getSuperBellSpecial() const;
    DoorKey* getDoorKey() const;
    GigaBell* getGigaBell() const;
    KinokoGiga* getKinokoGiga() const;
    WhiteBell* getWhiteBell() const;

private:
    u8 mUnreconstructed[0xa8];
};

static_assert(sizeof(ItemHolder) == 0xa8);
