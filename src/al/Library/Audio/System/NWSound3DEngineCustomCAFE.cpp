#include "Library/Audio/System/NWSound3DEngineCustomCAFE.hpp"

#include <algorithm>
#include <nn/atk/atk_Sound3DCalculator.h>
#include <nn/atk/atk_Sound3DListener.h>
#include <nn/atk/atk_Sound3DManager.h>

namespace al {

/**
 * Constructs the custom 3D sound engine.
 */
NWSound3DEngineCustomCAFE::NWSound3DEngineCustomCAFE() = default;

/**
 * Calculates the ambient parameters of a 3D sound from all listeners.
 * @param pOutValue Ambient parameters to write the results to.
 * @param pManager 3D sound manager holding the listeners.
 * @param pParam 3D parameters of the sound.
 * @param soundId Id of the sound (unused).
 * @param updateFlag Flags of the parameters to update.
 */
void NWSound3DEngineCustomCAFE::UpdateAmbientParam(nn::atk::SoundAmbientParam* pOutValue,
                                                   const nn::atk::Sound3DManager* pManager,
                                                   const nn::atk::Sound3DParam* pParam,
                                                   u32 soundId, u32 updateFlag) {
    const nn::atk::Sound3DManager::ListenerList& listenerList = pManager->GetListenerList();

    if (updateFlag & UpdatePriority) {
        pOutValue->priority = -pManager->GetMaxPriorityReduction();
    }

    if (updateFlag & UpdateFilter) {
        pOutValue->biquadFilterValue = 0.0f;
    }

    if (updateFlag & UpdateVolume) {
        pOutValue->volume = 0.0f;
    }

    if (listenerList.size() > 1) {
        updateFlag &= ~UpdatePitch;
    }

    s32 tvListenerNum = 0;

    for (auto it = listenerList.begin(); it != listenerList.end(); ++it) {
        if (it->GetOutputTypeFlag() & nn::atk::Sound3DListener::ListenerOutputType_Tv) {
            tvListenerNum++;
        }
    }

    for (auto it = listenerList.begin(); it != listenerList.end(); ++it) {
        const nn::atk::Sound3DListener& listener = *it;
        bool isOutputTv = false;

        if (listener.GetOutputTypeFlag() & nn::atk::Sound3DListener::ListenerOutputType_Tv) {
            isOutputTv = true;
        }

        if (updateFlag & (UpdateVolume | UpdatePriority)) {
            f32 volume;
            s32 priority;
            nn::atk::Sound3DCalculator::CalculateVolumeAndPriority(&volume, &priority, *pManager,
                                                                   listener, *pParam);

            if ((updateFlag & UpdateVolume) && isOutputTv) {
                pOutValue->volume = volume;
            }

            if (updateFlag & UpdatePriority) {
                s32 currentPriority = pOutValue->priority;
                pOutValue->priority = std::max(priority, currentPriority);
            }
        }

        if (updateFlag & (UpdatePan | UpdateSurroundPan)) {
            if (isOutputTv && tvListenerNum == 1) {
                f32 pan;
                f32 surroundPan;
                nn::atk::Sound3DCalculator::CalculatePan(&pan, &surroundPan, *pManager, listener,
                                                         *pParam, GetCalculatePanParam());

                if (updateFlag & UpdatePan) {
                    pOutValue->tvParam.pan = pan;
                }

                if (updateFlag & UpdateSurroundPan) {
                    pOutValue->tvParam.span = surroundPan;
                }
            }
        }

        if (updateFlag & UpdatePitch) {
            f32 pitch;
            nn::atk::Sound3DCalculator::CalculatePitch(&pitch, *pManager, listener, *pParam);
            pOutValue->pitch = pitch;
        }

        if (updateFlag & UpdateFilter) {
            f32 biquadFilterValue;
            nn::atk::Sound3DCalculator::CalculateBiquadFilterValue(&biquadFilterValue, *pManager,
                                                                   listener, *pParam);
            pOutValue->biquadFilterType = pManager->GetBiquadFilterType();
            f32 currentBiquadFilterValue = pOutValue->biquadFilterValue;
            pOutValue->biquadFilterValue = biquadFilterValue < currentBiquadFilterValue ?
                                               currentBiquadFilterValue :
                                               biquadFilterValue;
        }
    }
}

}  // namespace al
