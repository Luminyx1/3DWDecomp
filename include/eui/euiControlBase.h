#pragma once

#include <nn/font/font_Util.h>
#include <nn/util/util_IntrusiveList.h>

namespace eui {

class ControlBase {
public:
    virtual const char* getClassName() const;

    NN_RUNTIME_TYPEINFO_BASE();

    ControlBase();
    virtual ~ControlBase();

    virtual void Update(float);

    nn::util::IntrusiveListNode m_Link;
    void* _18;
    void* _20;
};

}  // namespace eui
