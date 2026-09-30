// This original unit contains shared RTTI and template instantiations; FontMgr's methods are in euiFontMgr_dup2.
#include <eui/euiPartsEx.h>
#include <eui/euiWindowEx.h>
#include <eui/euiPictureEx.h>
#include <eui/euiTextBoxEx.h>
#include <filedevice/seadFileDevice.h>
namespace {
// Keep the same shared runtime type definitions emitted in this translation unit.
const nn::font::detail::RuntimeTypeInfo* (*const typeInfoFunctions[])() __attribute__((used)) = {
    &eui::PartsEx::GetRuntimeTypeInfoStatic, &eui::WindowEx::GetRuntimeTypeInfoStatic,
    &eui::PictureEx::GetRuntimeTypeInfoStatic, &eui::TextBoxEx::GetRuntimeTypeInfoStatic
};
}

template class sead::FixedSafeString<25>;
