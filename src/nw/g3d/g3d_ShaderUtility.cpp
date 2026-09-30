#include <nn/g3d/g3d_ShaderUtility.h>
#include <cstring>

namespace nn::g3d {
// key receives model defaults and assignment overrides; capacity/check are unchecked in this build.
void ShaderUtility::InitializeShaderKey(u32* key, int capacity, const ResShadingModel* model,
                                        const ResShaderAssign* assignment, bool check) {
    int staticLength = model->GetStaticKeyLength();
    model->WriteDefaultStaticKey(key);
    model->WriteDefaultDynamicKey(key + staticLength);
    int count = assignment->ToData().optionCount;
    for (int i = 0; i < count; ++i) {
        const ResShaderOption* option = model->FindStaticOption(assignment->GetOptionName(i));
        if (option == nullptr) continue;
        int choice = option->FindChoiceIndex(assignment->ToData().pOptionArray.Get()[i].Get()->GetData());
        if (choice >= 0) option->WriteStaticKey(key, choice);
    }
}

// object receives assignment's static option choices; check is unchecked in this build.
void ShaderUtility::InitializeShaderKey(ShadingModelObj* object, const ResShaderAssign* assignment, bool check) {
    object->ClearStaticKey();
    int count = assignment->ToData().optionCount;
    for (int i = 0; i < count; ++i) {
        const char* name = assignment->GetOptionName(i);
        const ResShadingModel* model = object->GetResource();
        int index = model->FindStaticOptionIndex(name);
        if (index < 0) continue;
        const ResShaderOption* option = model->GetStaticOption(index);
        int choice = option->FindChoiceIndex(assignment->ToData().pOptionArray.Get()[i].Get()->GetData());
        if (choice >= 0) object->WriteStaticKey(index, choice);
    }
}

// material receives offsets for parameters found in model's material uniform block.
void ShaderUtility::BindShaderParam(ResMaterial* material, const ResShadingModel* model) {
    int blockIndex = model->GetMaterialBlockIndex();
    if (blockIndex == -1) { material->ToData().materialBlockSize = 0; return; }
    const ResUniformBlock* block = model->GetUniformBlock(blockIndex);
    material->ToData().materialBlockSize = block->size;
    int count = material->ToData().shaderParamCount;
    for (int i = 0; i < count; ++i) {
        ResShaderParamData* param = &material->ToData().pShaderParamArray.Get()[i];
        const ResUniformVar* uniform = block->FindUniform(param->name.Get()->GetData());
        param->offset = (uniform != nullptr) ? uniform->offset - 1 : -1;
    }
}

// material receives model's default block contents in each buffered copy, then flushes the writes.
void ShaderUtility::InitializeShaderParam(MaterialObj* material, const ResShadingModel* model) {
    int blockIndex = model->GetMaterialBlockIndex();
    if (blockIndex == -1) return;
    const ResUniformBlock* block = model->GetUniformBlock(blockIndex);
    int count = material->GetBufferingCount();
    for (int i = 0; i < count; ++i) {
        void* output = material->GetMaterialBlock(i)->Map();
        std::memcpy(output, block->defaultValues, block->size);
        material->GetMaterialBlock(i)->FlushMappedRange(0, material->GetMaterialBlockSize());
        material->GetMaterialBlock(i)->Unmap();
    }
}
}
