#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

namespace al {
class ByamlIter;

class SeCategoryNameList {
  public:
    SeCategoryNameList(const char** pNames, s32 num);

    const char* getCategoryName(s32 index) const;
    s32 findCategoryNoFromName(const char* pName) const;

    /**
     * @brief Gets the number of categories.
     * @return Number of stored category names.
     */
    s32 getNum() const { return mNames.size(); }

  private:
    sead::PtrArray<const char> mNames;
};

static_assert(sizeof(SeCategoryNameList) == 0x10);

class SeCategoryInfoList {
  public:
    SeCategoryInfoList(const SeCategoryNameList* pNameList);

    bool importYaml(ByamlIter& rIter);
    void setCategoryVolume(const char* pName, f32 volume);

    /**
     * @brief Gets the category volumes in category-name order.
     * @return List of owned volume values in decibels.
     */
    const sead::PtrArray<f32>& getVolumes() const { return mVolumes; }

  private:
    sead::PtrArray<f32> mVolumes;
    const SeCategoryNameList* mNameList;
};

static_assert(sizeof(SeCategoryInfoList) == 0x18);

class AudioMixVolume {
  public:
    AudioMixVolume();

    void moveTo(f32 volumeDb, s32 frames);
    void update();
    void linkTo(const AudioMixVolume* pVolume);
    f32 calcLinkedVolumeDecibel() const;

    /** @brief Removes the linked volume without changing this volume's own fade. */
    void resetLink() { mLinkedVolume = nullptr; }
    /** @brief Gets the parent volume used for linked mixing. @return Linked controller, or nullptr. */
    const AudioMixVolume* getLinkedVolume() const { return mLinkedVolume; }

  private:
    f32 mVolumeDb = 0.0f;
    f32 mTargetRatio = 0.0f;
    f32 mCurRatio = 0.0f;
    f32 mStep = 0.0f;
    f32 mRemainFrames = -1.0f;
    const AudioMixVolume* mLinkedVolume = nullptr;
};

static_assert(sizeof(AudioMixVolume) == 0x20);

class SeCategoryParamsController {
  public:
    SeCategoryParamsController(const SeCategoryNameList* pNameList);

    void moveTo(const SeCategoryInfoList* pInfoList, s32 frames);
    void update();
    void linkTo(const SeCategoryParamsController* pController);
    AudioMixVolume* getMixVolume(s32 index) const;

  private:
    sead::PtrArray<AudioMixVolume> mMixVolumes;
    const SeCategoryNameList* mNameList;
};

static_assert(sizeof(SeCategoryParamsController) == 0x18);

f32 calcDecibelToRatio(f32 decibel);
f32 calcRatioToDecibel(f32 ratio);
} // namespace al
