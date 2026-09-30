#include <nn/atk/atk_MmlSequenceTrack.h>

namespace nn::atk::detail::driver {
MmlSequenceTrack::MmlSequenceTrack() {}
// playNotes controls whether parsing emits notes or only advances sequence state.
int MmlSequenceTrack::Parse(bool playNotes) { return mParser->Parse(this, playNotes); }
}
