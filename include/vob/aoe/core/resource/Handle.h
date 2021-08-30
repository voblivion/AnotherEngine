#pragma once

#include <memory>

#include <vob/aoe/core/resource/AHolder.h>


namespace vob::aoe::res
{
    template <typename ResourceBaseType>
    class Handle
    {
    public:
        Handle(std::nullptr_t = nullptr)
        {}

        template <typename HolderType>
        Handle(std::shared_ptr<HolderType> a_resource)
            : m_resource{ std::move(a_resource) }
        {}

        ResourceBaseType* get()
        {
            return m_resource != nullptr ? m_resource->get() : nullptr;
        }

        ResourceBaseType* operator->()
        {
            return get();
        }

        ResourceBaseType& operator*()
        {
            return *get();
        }

        bool operator==(std::nullptr_t)
        {
            return m_resource == nullptr || m_resource->get() == nullptr;
        }

        bool operator!=(std::nullptr_t)
        {
            return !(*this == nullptr);
        }

        operator bool()
        {
            return *this != nullptr;
        }

        Handle& operator=(std::nullptr_t)
        {
            m_resource.reset();
            return *this;
        }

        template <typename HolderType>
        Handle& operator=(std::shared_ptr<HolderType> a_resource)
        {
            m_resource = std::move(a_resource);
            return *this;
        }

    private:
        std::shared_ptr<AHolder<ResourceBaseType>> m_resource;
    };

}

