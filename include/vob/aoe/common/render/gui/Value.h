#pragma once

#include <array>


namespace vob::aoe::common // TODO : change to ::gui
{
    struct PxValue
    {
        float m_value = 0.0f;

        PxValue operator-() const
        {
            return { -m_value };
        }
    };

    struct EmValue
    {
        float m_value = 0.0f;

        EmValue operator-() const
        {
            return { -m_value };
        }
    };

    struct PeValue
    {
        float m_value = 0.0f;

        PeValue operator-() const
        {
            return { -m_value };
        }
    };

    namespace literals
    {
        constexpr PxValue operator"" _px(long double a_value)
        {
            return { static_cast<float>(a_value) };
        }
        constexpr PxValue operator"" _px(unsigned long long a_value)
        {
            return { static_cast<float>(a_value) };
        }
        constexpr EmValue operator"" _em(long double a_value)
        {
            return { static_cast<float>(a_value) };
        }
        constexpr EmValue operator"" _em(unsigned long long a_value)
        {
            return { static_cast<float>(a_value) };
        }
        constexpr PeValue operator"" _pe(long double a_value)
        {
            return { static_cast<float>(a_value) };
        }
        constexpr PeValue operator"" _pe(unsigned long long a_value)
        {
            return { static_cast<float>(a_value) };
        }
    }

    class Value
    {
    public:
        enum class Type : std::uint8_t
        {
            Px,
            Em,
            Pe
        };

        Value() = default;

        Value(float a_value, Type a_type)
            : m_value{ a_value }
            , m_type{ a_type }
        {}

        Value(PxValue a_pxValue)
            : m_value{ a_pxValue.m_value }
            , m_type{ Type::Px }
        {}

        Value(EmValue a_emValue)
            : m_value{ a_emValue.m_value }
            , m_type{ Type::Em }
        {}

        Value(PeValue a_peValue)
            : m_value{ a_peValue.m_value }
            , m_type{ Type::Pe }
        {}

        float get(float a_transformSize, float a_fontSize) const
        {
            switch (m_type)
            {
            case Type::Em:
                return m_value * a_fontSize;
            case Type::Pe:
                return m_value * a_transformSize;
            case Type::Px:
            default:
                return m_value;
            }
        }

        void set(float a_value, Type a_type)
        {
            m_value = a_value;
            m_type = a_type;
        }

        void operator=(float a_value)
        {
            m_value = a_value;
            m_type = Type::Px;
        }

        template <typename VisitorType, typename Self>
        static bool accept(VisitorType& a_visitor, Self& a_this)
        {
            a_visitor.visit(makeNameValuePair("Value", a_this.m_value));
            a_visitor.visit(makeNameValuePair("Type", a_this.m_type));
            return true;
        }

    private:
        float m_value = 0.0f;
        Type m_type = Type::Px;
    };

    class QuadValue
    {
    public:
        QuadValue(Value a_all = {})
            : m_values{ a_all, a_all, a_all, a_all }
        {}
        QuadValue(Value a_width, Value a_height)
            : m_values{ a_width, a_height, a_width, a_height }
        {}
        QuadValue(Value a_left, Value a_top, Value a_right, Value a_bottom)
            : m_values{ a_left, a_top, a_right, a_bottom }
        {}

        glm::vec4 get(float a_transformSize, float a_fontSize) const
        {
            return {
                m_values[0].get(a_transformSize, a_fontSize)
                , m_values[1].get(a_transformSize, a_fontSize)
                , m_values[2].get(a_transformSize, a_fontSize)
                , m_values[3].get(a_transformSize, a_fontSize)
            };
        }

        Value const& operator[](int i) const { return m_values[i]; }
        Value& operator[](int i) { return m_values[i]; }

        template <typename VisitorType, typename Self>
        static bool accept(VisitorType& a_visitor, Self& a_this)
        {
            accept(a_visitor, a_this.m_values);
            return true;
        }

    private:
        std::array<Value, 4> m_values;
    };
}

