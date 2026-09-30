#include "Project/Action/Common/ActionScreenEffectCtrl.hpp"
#include "Project/Yaml/YamlBridge.hpp"

template void YamlWriterBridge::exec<al::ActionScreenEffectCtrlInfo>(al::ActionScreenEffectCtrlInfo*,
                                                                     const char*);
template void YamlReaderBridge::exec<al::ActionScreenEffectCtrlInfo>(al::ActionScreenEffectCtrlInfo*,
                                                                     const char*);
template void YamlWriterBridge::exec<al::ActionRadialBlurData>(al::ActionRadialBlurData*,
                                                               const char*);
template void YamlReaderBridge::exec<al::ActionRadialBlurData>(al::ActionRadialBlurData*,
                                                               const char*);
