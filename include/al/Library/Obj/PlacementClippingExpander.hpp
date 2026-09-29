#pragma once

namespace al {
    class LiveActor;
    struct PlacementInfo;

    /// Expands an actor's clipping to cover a linked ClippingExpandObject placement.
    class PlacementClippingExpander {
    public:
        PlacementClippingExpander();

        void init(LiveActor* pActor, const PlacementInfo& rInfo);

        void* _0[4];
    };
};
