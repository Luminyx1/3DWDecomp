#include <eui/euiLetterAnimControl.h>
namespace eui {
const char* LetterAnimControl::getClassName() const { return "LetterAnimControl"; }
LetterAnimControl::LetterAnimControl()
    : mTextBox(nullptr), mText(nullptr), mWorkBuffer(nullptr), mSpeed(0), mDefaultSpeed(0),
      mWaitRemaining(0), _4c(0), mCurrentText(nullptr), mVisibleLength(0), _5a(0),
      mTextLength(0), mPlaying(0), mFlags(0), mFlushMode(0), _61(0), _62(0), mAlpha(255),
      _64(0), _68(0), mAppTagCallback(nullptr), mChoiceExcludeCallback(nullptr) {}
// pCallback receives application tags encountered during letter playback.
void LetterAnimControl::setAppTagCallback(AppTagCallback* pCallback) {
    mAppTagCallback = pCallback;
}

// pCallback decides whether a text choice should be excluded.
void LetterAnimControl::setChoiceExcludeCallback(ChoiceExcludeCallback* pCallback) {
    mChoiceExcludeCallback = pCallback;
}

void LetterAnimControl::start() { mPlaying = 1; }
void LetterAnimControl::stop() { mPlaying = 0; }
// speed sets both the current and default letter playback rate.
void LetterAnimControl::setSpeed(float speed) { mSpeed = speed; mDefaultSpeed = speed; }
// speed overrides the current playback rate until the default is restored.
void LetterAnimControl::setSpeedTemporarily(float speed) { mSpeed = speed; }
void LetterAnimControl::flush() { mFlushMode = 2; }
void LetterAnimControl::flushAllowWait() { if (mFlushMode != 2) mFlushMode = 1; }
}
