#include <eui/euiAnimator.h>

namespace eui {
Animator::Animator() : mStep(0), mLoopCount(0), mPlayType(0), mFlags(0x20),
                       mLayout(nullptr), mName(nullptr) {}
Animator::~Animator() = default;

// frame is the initial frame, type controls playback, and step is the frame increment.
void Animator::PlayFromFrame(float frame, PlayType type, float step) {
    mFrame = frame;
    PlayFromCurrent(type, step);
}

// rOther supplies the frame, speed, and playback mode to mirror.
void Animator::Synchronize(const Animator& rOther) {
    if (rOther.mStep != 0) {
        PlayFromFrame(rOther.mFrame, static_cast<PlayType>(rOther.mPlayType), rOther.mStep);
    } else {
        Stop(rOther.mFrame);
    }
}
}
