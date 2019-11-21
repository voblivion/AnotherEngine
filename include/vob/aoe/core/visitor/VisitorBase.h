#pragma once

#include <vob/aoe/core/type/Traits.h>
#include <vob/aoe/core/visitor/Traits.h>
#include <vob/aoe/core/visitor/Applicator.h>

namespace vob::aoe::vis
{
	class VisitorBase
	{
	public:
		// Constructors
		explicit VisitorBase(type::TypeRegistry const& a_typeRegistry
			, type::TypeFactory const& a_typeFactory)
			: m_typeRegistry{ a_typeRegistry }
			, m_typeFactory{ a_typeFactory }
		{}

		// Methods
		auto& getTypeRegistry() const
		{
			return m_typeRegistry;
		}

		auto& getTypeFactory() const
		{
			return m_typeFactory;
		}

	private:
		// Attributes
		type::TypeRegistry const& m_typeRegistry;
		type::TypeFactory const& m_typeFactory;
	};

	template <typename VisitorType
		, typename Applicator = Applicator<void const, VisitorType>>
	class OutputVisitorBase
		: public VisitorBase
	{
	public:
		// Constructor
		explicit OutputVisitorBase(
			type::TypeRegistry const& a_typeRegistry
			, type::TypeFactory const& a_typeFactory
			, std::pmr::polymorphic_allocator<std::byte> a_allocator = {}
		)
			: VisitorBase{ a_typeRegistry, a_typeFactory }
			, m_applicator{ a_allocator }
		{}

		// Methods
		template <typename ValueType>
		void visit(ValueType&& a_value)
		{
			static_cast<VisitorType&>(*this).preVisit();
			processVisit(std::forward<ValueType>(a_value));
			static_cast<VisitorType&>(*this).postVisit();
		}

		bool isRegistered(std::type_index const a_typeIndex) const
		{
			return m_applicator.isRegistered(a_typeIndex);
		}

		template <typename Type>
		bool isRegistered() const
		{
			return m_applicator.template isRegistered<Type const>();
		}

		template <typename Type>
		void registerType()
		{
			m_applicator.template registerType<Type const>();
		}

	protected:
		// Methods
		template <typename ValueType
			, enforce((hasMemberAccept<VisitorType, ValueType const>))
		>
		void processVisit(ValueType&& a_value)
		{
			std::forward<ValueType>(a_value).accept(static_cast<VisitorType&>(*this));
		}

		template <typename ValueType
			, enforce((hasNonMemberAccept<VisitorType, ValueType const>))
		>
		void processVisit(ValueType&& a_value)
		{
			accept(static_cast<VisitorType&>(*this), std::forward<ValueType>(a_value));
		}

		template <typename ValueType
			, enforce(std::is_arithmetic_v<std::remove_reference_t<ValueType>>)>
		void processVisit(ValueType const& a_value)
		{
			static_cast<VisitorType&>(*this).processArithmetic(a_value);
		}

		template <typename CharType, typename TraitsType, typename AllocatorType>
		void processVisit(std::basic_string<CharType, TraitsType
			, AllocatorType>& a_value)
		{
			static_cast<VisitorType&>(*this).processString(a_value);
		}

		void processVisit(vis::SizeTag const& a_sizeTag)
		{
			static_cast<VisitorType&>(*this).processSizeTag(a_sizeTag);
		}

		template <typename ValueType>
		void processVisit(
			vis::IndexValuePair<ValueType> const& a_indexValuePair)
		{
			static_cast<VisitorType&>(*this).processKeyValuePair(
				a_indexValuePair);
		}

		template <typename ValueType>
		void processVisit(
			vis::NameValuePair<ValueType> const& a_nameValuePair)
		{
			static_cast<VisitorType&>(*this).processKeyValuePair(
				a_nameValuePair);
		}

		template <typename BaseType>
		void processVisit(DynamicValue<BaseType> a_dynamicValue)
		{
			m_applicator.apply(const_cast<BaseType const&>(a_dynamicValue.m_value)
				, static_cast<VisitorType&>(*this));
		}

	private:
		// Attributes
		Applicator m_applicator;
	};

	template <typename VisitorType
		, typename Applicator = Applicator<void, VisitorType>>
	class InputVisitorBase
		: public VisitorBase
	{
	public:
		// Constructor
		explicit InputVisitorBase(
			type::TypeRegistry const& a_typeRegistry
			, type::TypeFactory const& a_typeFactory
			, std::pmr::polymorphic_allocator<std::byte> a_allocator = {}
		)
			: VisitorBase{ a_typeRegistry, a_typeFactory }
			, m_applicator{ a_allocator }
		{}

		// Methods
		template <typename ValueType>
		void visit(ValueType&& a_value)
		{
			static_cast<VisitorType&>(*this).preVisit();
			processVisit(std::forward<ValueType>(a_value));
			static_cast<VisitorType&>(*this).postVisit();
		}

		bool isRegistered(std::type_index const a_typeIndex) const
		{
			return m_applicator.isRegistered(a_typeIndex);
		}

		template <typename Type>
		bool isRegistered() const
		{
			return m_applicator.template isRegistered<Type>();
		}

		template <typename Type>
		void registerType()
		{
			m_applicator.template registerType<Type>();
		}

	protected:
		// Methods
		template <typename ValueType, enforce((hasMemberAccept<VisitorType, ValueType>))>
		void processVisit(ValueType&& a_value)
		{
			std::forward<ValueType>(a_value).accept(static_cast<VisitorType&>(*this));
		}

		template <typename ValueType, enforce((hasNonMemberAccept<VisitorType, ValueType>))>
		void processVisit(ValueType&& a_value)
		{
			accept(static_cast<VisitorType&>(*this), std::forward<ValueType>(a_value));
		}

		template <
			typename ValueType
			, enforce(std::is_arithmetic_v<std::remove_reference_t<ValueType>>)
		>
		void processVisit(ValueType& a_value)
		{
			static_cast<VisitorType&>(*this).processArithmetic(a_value);
		}

		template <typename CharType, typename TraitsType, typename AllocatorType>
		void processVisit(std::basic_string<CharType, TraitsType, AllocatorType>& a_value)
		{
			static_cast<VisitorType&>(*this).processString(a_value);
		}

		void processVisit(vis::SizeTag& a_sizeTag)
		{
			static_cast<VisitorType&>(*this).processSizeTag(a_sizeTag);
		}

		template <typename ValueType>
		void processVisit(vis::IndexValuePair<ValueType> a_indexValuePair)
		{
			static_cast<VisitorType&>(*this).processKeyValuePair(a_indexValuePair);
		}

		template <typename ValueType>
		void processVisit(vis::NameValuePair<ValueType> a_nameValuePair)
		{
			static_cast<VisitorType&>(*this).processKeyValuePair(a_nameValuePair);
		}

		template <typename BaseType>
		void processVisit(DynamicValue<BaseType> a_dynamicValue)
		{
			m_applicator.apply(const_cast<BaseType&>(a_dynamicValue.m_value)
				, static_cast<VisitorType&>(*this));
		}

	private:
		// Attributes
		Applicator m_applicator;
	};
}
