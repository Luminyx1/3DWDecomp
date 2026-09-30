#include <nn/ui2d/ui2d_Init.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/ui2d/ui2d_LayoutPaneFactory.h>
#include <nn/os.h>
#include <nn/util.h>

namespace nn::ui2d {
LayoutPaneFactory::~LayoutPaneFactory() = default;

void Ui2dInitializationParamaters::SetDefault() {
    paneFactory = &g_DefaultLayoutPaneFactory;
    randomSeed = nn::os::GetSystemTick().GetInt64Value();
}

// seed expands into the four words used by the shared UI random generator.
static inline void InitializeRandom(u32 seed) {
    for (u32 index = 0; index < 4; ++index) {
        seed = 1812433253u * (seed ^ (seed >> 30)) + index + 1;
        Layout::g_Random[index] = seed;
    }
}

// allocate/free are the layout memory callbacks; userData is passed to both.
void Initialize(void* (*allocate)(size_t, size_t, void*), void (*free)(void*, void*), void* userData) {
    Ui2dInitializationParamaters parameters;
    parameters.SetDefault();
    InitializeWithParamaters(allocate, free, userData, parameters);
}

// allocate/free receive userData for memory operations. parameters selects the
// pane factory and the initial random seed used by UI animations.
void InitializeWithParamaters(void* (*allocate)(size_t, size_t, void*),
                              void (*free)(void*, void*), void* userData,
                              const Ui2dInitializationParamaters& parameters) {
    nn::util::ReferSymbol("SDK MW+Nintendo+NintendoWare_Ui2d-10_4_0-Release");
    Layout::SetAllocator(allocate, free, userData);
    Layout::g_pLayoutPaneFactory = parameters.paneFactory;
    InitializeRandom(parameters.randomSeed);
}
}
