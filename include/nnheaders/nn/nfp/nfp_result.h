#pragma once

#include <nn/types.h>

namespace nn::nfp {

R_DEFINE_NAMESPACE_RESULT_MODULE(115);

R_DEFINE_ERROR_RANGE(NfcDeviceNotFound, 64, 79);
R_DEFINE_ERROR_RESULT(NfcDisabled, 80);
R_DEFINE_ERROR_RANGE(NeedRetry, 88, 95);
R_DEFINE_ERROR_RANGE(NeedRestart, 96, 103);
R_DEFINE_ERROR_RESULT(NeedRestore, 136);
R_DEFINE_ERROR_RESULT(NeedFormat, 144);
R_DEFINE_ERROR_RANGE(NotSupported, 176, 183);
R_DEFINE_ERROR_RESULT(InvalidFormatVersion, 184);

}  // namespace nn::nfp
