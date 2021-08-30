#pragma once

#include <vob/aoe/core/resource/AHolder.h>


namespace vob::aoe::res
{
    template <typename ResourceBaseType, typename ResourceType>
    class DefaultHolder : public AHolder<ResourceBaseType>
    {
    public:
        template <typename... Args>
        explicit DefaultHolder(Args&&... a_args)
            : m_object{ std::forward<Args>(a_args)... }
        {}

        ResourceBaseType* get() override
        {
            return &m_object;
        }

    private:
        ResourceType m_object;
    };
}

