#pragma once

#include <nn/font/font_Util.h>
#include <nn/util/util_IntrusiveList.h>

namespace eui {
class LayoutEx;

class ControlBase {
public:
    virtual const char* getClassName() const;

    NN_RUNTIME_TYPEINFO_BASE();

    ControlBase();
    virtual ~ControlBase();

    virtual void Update(float);

    const char* getName() const { return mName; }
    LayoutEx* getLayout() const { return static_cast<LayoutEx*>(_20); }

    nn::util::IntrusiveListNode m_Link;
    const char* mName;
    void* _20;
};

}  // namespace eui
