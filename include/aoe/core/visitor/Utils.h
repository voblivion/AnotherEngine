#pragma once

#include <cinttypes>
#include <string_view>

namespace aoe
{
	namespace vis
	{
		struct SizeTag
		{
		public:
			// Constructor
			explicit SizeTag(std::size_t const a_size = 0)
				: m_size{ a_size }
			{}

			// Attributes
			std::size_t m_size;
		};

		template <typename ValueType>
		struct IndexValuePair
		{
			using Type = std::conditional_t<std::is_lvalue_reference_v<ValueType>
				, ValueType, std::remove_reference_t<ValueType>>;

		public:
			// Attributes
			std::size_t m_index;
			ValueType& m_value;

			// Constructor
			IndexValuePair(std::size_t const a_index, Type&& a_value)
				: m_index{ a_index }
				, m_value{ std::forward<Type>(a_value) }
			{}
		};

		template <typename ValueType>
		IndexValuePair<ValueType> makeIndexValuePair(std::size_t const a_index
			, ValueType&& a_value)
		{
			return { a_index, std::forward<ValueType>(a_value) };
		}

		template <typename ValueType>
		struct NameValuePair
		{
			using Type = std::conditional_t<std::is_lvalue_reference_v<ValueType>
				, ValueType, std::remove_reference_t<ValueType>>;

		public:
			// Attributes
			std::string_view m_name;
			ValueType& m_value;
			
			// Constructor
            NameValuePair(std::string_view a_name, Type&& a_value)
                : m_name{ a_name }
                , m_value{ std::forward<Type>(a_value) }
            {}
		};

		template <typename ValueType>
		NameValuePair<ValueType> makeNameValuePair(std::string_view const a_name
			, ValueType&& a_value)
		{
			return { a_name, std::forward<ValueType>(a_value) };
		}

		template <typename ValueType>
		NameValuePair<ValueType> nvp(std::string_view const a_name
			, ValueType&& a_value)
		{
			return { a_name, std::forward<ValueType>(a_value) };
		}

		template <typename BaseType>
		struct DynamicValue
		{
			// Constructor
			explicit DynamicValue(BaseType& a_value)
				: m_value{ a_value }
			{}

			// Attributes
			BaseType& m_value;
		};

		template <typename BaseType>
		DynamicValue<BaseType> makeDynamicValue(BaseType& a_value)
		{
			return DynamicValue<BaseType>{ a_value };
		}

		template <typename ContainerType, typename FactoryType>
		struct ContainerHolder
		{
			// Constructor
			explicit ContainerHolder(ContainerType& a_container, FactoryType a_factory)
				: m_container{ a_container }
				, m_factory{ std::move(a_factory) }
			{}

			// Attributes
			ContainerType& m_container;
			FactoryType m_factory;
		};

		template <typename ContainerType, typename FactoryType>
		ContainerHolder<ContainerType, FactoryType> makeContainerHolder(
			ContainerType& a_container, FactoryType a_factory)
		{
			return ContainerHolder<ContainerType, FactoryType>{ a_container
				, std::move(a_factory) };
		}
	}
}