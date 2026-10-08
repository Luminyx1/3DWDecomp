#pragma once

#include <nn/atk/atk_Global.h>
#include <nn/os.h>
#include <nn/util/util_IntrusiveList.h>
#include <nn/types.h>

namespace nn::atk {
class SoundArchive;
class SoundHandle;
class SoundActor;
class SoundPlayer;
typedef void (*SoundStopCallback)();
struct SoundParamCalculationValues;
struct SoundAmbientParam;
} // namespace nn::atk
namespace nn::atk {
enum PanMode {
    PanMode_Dual,
    PanMode_Balance,
    PanMode_Invalid,
};

enum PanCurve {
    PanCurve_Sqrt,
    PanCurve_Sqrt0Db,
    PanCurve_Sqrt0DbClamp,
    PanCurve_SinCos,
    PanCurve_SinCos0Db,
    PanCurve_SinCos0DbClamp,
    PanCurve_Linear,
    PanCurve_Linear0Db,
    PanCurve_Linear0DbClamp,
    PanCurve_Invalid,
};

enum MixMode {
    MixMode_Pan,
    MixMode_MixParameter,
};

struct MixParameter {
    f32 ch[ChannelIndex_Count];
};
} // namespace nn::atk
namespace nn::atk::detail {
class ExternalSoundPlayer;
struct RuntimeTypeInfo {
    // parent identifies the immediate base type, or null for the root type.
    explicit RuntimeTypeInfo(const RuntimeTypeInfo* parent) : parent(parent) {}
    const RuntimeTypeInfo* parent;
};
class BasicSound {
public:
    static const RuntimeTypeInfo* GetRuntimeTypeInfoStatic();
    virtual const RuntimeTypeInfo* GetRuntimeTypeInfo() const;
    virtual ~BasicSound();
    virtual void Initialize();
    virtual void Finalize();
    virtual bool IsPrepared() const = 0;
    virtual bool IsAttachedTempSpecialHandle() = 0;
    virtual void DetachTempSpecialHandle() = 0;
    class AmbientInfo;

    class AmbientParamUpdateCallback {
    public:
        /**
         * @brief Destroys the ambient-parameter callback.
         */
        virtual ~AmbientParamUpdateCallback() = default;
        virtual void detail_UpdateAmbientParam(const void* pArg, u32 soundId, SoundAmbientParam* pParam) = 0;
        virtual int detail_GetAmbientPriority(const void* pArg, u32 soundId) = 0;
    };
    class AmbientArgAllocator {
    public:
        /**
         * @brief Destroys the ambient-argument allocator.
         */
        virtual ~AmbientArgAllocator() = default;
        virtual void* detail_AllocAmbientArg(size_t size) = 0;
        virtual void detail_FreeAmbientArg(void* pArg, const BasicSound* pSound) = 0;
    };

    class AmbientArgUpdateCallback {
    public:
        virtual ~AmbientArgUpdateCallback() {}
        virtual void detail_UpdateAmbientArg(void* pArg, const BasicSound* pSound) = 0;
    };

    void AttachExternalSoundPlayer(ExternalSoundPlayer* pPlayer);
    void DetachExternalSoundPlayer(ExternalSoundPlayer* pPlayer);
    void DetachSoundActor(SoundActor* pActor);
    void Pause(bool pause, int fadeFrames, PauseMode mode);
    /**
     * @brief Gets the actor owning this sound.
     * @return Associated actor, or null.
     */
    SoundActor* GetSoundActor() const { return mSoundActor; }
    /**
     * @brief Combines player and ambient priority.
     * @return Priority clamped to [0, 127].
     */
    int GetPlayerPriority() const {
        int priority = mPlayerPriority + mAmbientPriority;
        if (priority < 0) {
            priority = 0;
        }
        return priority < 127 ? priority : 127;
    }
    /**
     * @brief Gets the external player's intrusive linkage.
     * @return Node initialized by the sound constructor.
     */
    util::IntrusiveListNode& GetExternalPlayerNode() {
        return *reinterpret_cast<util::IntrusiveListNode*>(reinterpret_cast<char*>(this) + 0x200);
    }
    /**
     * @brief Gets the owning sound player's sound-list linkage.
     * @return Node initialized by the sound constructor.
     */
    util::IntrusiveListNode& GetSoundPlayerPlayNode() {
        return *reinterpret_cast<util::IntrusiveListNode*>(reinterpret_cast<char*>(this) + 0x1e0);
    }
    /**
     * @brief Gets the owning sound player's priority-list linkage.
     * @return Node initialized by the sound constructor.
     */
    util::IntrusiveListNode& GetSoundPlayerPriorityNode() {
        return *reinterpret_cast<util::IntrusiveListNode*>(reinterpret_cast<char*>(this) + 0x1f0);
    }
    void Update();
    void AttachSoundPlayer(SoundPlayer* pPlayer);
    void DetachSoundPlayer(SoundPlayer* pPlayer);
    void Stop(int fadeFrames);
    void Pause(bool flag, int fadeFrames);
    void SetVolume(f32 volume, int frames);
    void SetPitch(f32 pitch);
    void SetPan(f32 pan);
    bool IsAttachedGeneralHandle();
    void DetachGeneralHandle();
    bool IsAttachedTempGeneralHandle();
    void DetachTempGeneralHandle();
    void CalculateSoundParamCalculationValues(SoundParamCalculationValues* values) const;
    bool IsPause() const;
    f32 GetVolume() const;
    void SetSurroundPan(f32 pan);
    void SetMainSend(f32 send);
    void SetFxSend(AuxBus bus, f32 send);
    void SetLpfFreq(f32 freq);
    void StartPrepared();
    void FadeIn(int frames);
    void SetOutputLine(u32 lineFlag);
    void SetOutputFxSend(OutputDevice device, AuxBus bus, f32 send);
    void SetBiquadFilter(int type, f32 value);
    void SetMixMode(MixMode mode);
    void SetOutputChannelMixParameter(OutputDevice device, u32 channel, MixParameter param);
    static int GetAmbientPriority(const AmbientInfo& rAmbientInfo, u32 soundId);
    void SetInitialVolume(f32 volume);
    void SetPanMode(PanMode mode);
    void SetPanCurve(PanCurve curve);
    void AttachSoundActor(SoundActor* pActor);
    void SetPlayerPriority(int priority);
    void SetSoundArchive(const SoundArchive* pArchive);
    void SetSetupTick(const os::Tick& rTick);
    void ResetOutputLine();

    /** @brief Gets the archive the sound was started from. @return Sound archive, or nullptr. */
    const SoundArchive* GetSoundArchive() const { return m_pSoundArchive; }
    /** @brief Gets when the sound was set up. @return Tick passed to SetSetupTick. */
    const os::Tick& GetSetupTick() const { return m_SetupTick; }
    /**
     * @brief Sets the callback invoked when the sound stops.
     * @param callback Stop callback, or nullptr.
     */
    void SetSoundStopCallback(SoundStopCallback callback) {
        *reinterpret_cast<SoundStopCallback*>(reinterpret_cast<char*>(this) + 0x1d8) = callback;
    }

    u32 GetId() const { return m_Id; }

private:
    friend class nn::atk::SoundHandle;
    u8 _8[8];
    SoundHandle* mGeneralHandle;
    SoundHandle* mTempGeneralHandle;
    SoundPlayer* mSoundPlayer;
    SoundActor* mSoundActor;
    ExternalSoundPlayer* mExternalSoundPlayer;
    const SoundArchive* m_pSoundArchive;
    u8 _40[0x7c - 0x40];
    int mAmbientPriority;
    u8 _80[0xf8 - 0x80];
    u8 mPlayerPriority;
    u8 _f9[0x110 - 0xf9];
    u32 m_Id;
    u8 _114[0x118 - 0x114];
    os::Tick m_SetupTick;
    u8 _120[0x210 - 0x120];
};
static_assert(sizeof(BasicSound) == 0x210);
} // namespace nn::atk::detail
