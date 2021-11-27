#pragma once

#include <vob/aoe/common/input/PhysicalSwitchHandle.h>


namespace vob::aoe::common::PhysicalSwitchHandleUtil
{
    bool isActive(PhysicalSwitchHandle const& a_handle);
}

struct WorldInputComponent
{
    struct Keyboard
    {
        enum class Key
        {
            A,
            B, 
            C,
            //...
            Count
        };

        
    };
};