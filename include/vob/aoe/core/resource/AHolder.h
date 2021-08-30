#pragma once


namespace vob::aoe::res
{
    template <typename ResourceBaseType>
    class AHolder
    {
    public:
        virtual ~AHolder() {}
        virtual ResourceBaseType const* get() = 0;
    };
}

