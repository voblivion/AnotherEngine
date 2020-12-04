#pragma once

#include <vob/aoe/core/type/Traits.h>
#include <vob/aoe/core/visitor/Traits.h>

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ContextType>
	class OutputVisitorBase
	{
	public:
		// Constructor
		explicit OutputVisitorBase(ContextType const& a_context)
			: m_context{ a_context }
		{}

		// Methods
		template <typename ValueType>
		void visit(ValueType&& a_value)
		{
			static_cast<VisitorType&>(*this).preVisit();
			processVisit(std::forward<ValueType>(a_value));
			static_cast<VisitorType&>(*this).postVisit();
		}

		ContextType const& getContext()
		{
			return m_context;
		}

	protected:
		// Methods
		template <typename ValueType, enforce((hasMemberAccept<VisitorType, ValueType const>))>
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
		void processVisit(std::basic_string<CharType, TraitsType, AllocatorType>& a_value)
		{
			static_cast<VisitorType&>(*this).processString(a_value);
		}

		void processVisit(vis::SizeTag const& a_sizeTag)
		{
			static_cast<VisitorType&>(*this).processSizeTag(a_sizeTag);
		}

		template <typename ValueType>
		void processVisit(vis::IndexValuePair<ValueType> const& a_indexValuePair)
		{
			static_cast<VisitorType&>(*this).processKeyValuePair(a_indexValuePair);
		}

		template <typename ValueType>
		void processVisit(vis::NameValuePair<ValueType> const& a_nameValuePair)
		{
			static_cast<VisitorType&>(*this).processKeyValuePair(a_nameValuePair);
		}

		/*template <typename BaseType>
		void processVisit(DynamicValue<BaseType> a_dynamicValue)
		{
			m_applicator.apply(
				const_cast<BaseType const&>(a_dynamicValue.m_value)
				, static_cast<VisitorType&>(*this)
			);
		}*/

	private:
		// Attributes
		ContextType m_context;
	};

	template <typename VisitorType, typename ContextType>
	class InputVisitorBase
	{
	public:
		// Constructor
		explicit InputVisitorBase(ContextType const& a_context)
			: m_context{ a_context }
		{}

		// Methods
		template <typename ValueType>
		void visit(ValueType&& a_value)
		{
			static_cast<VisitorType&>(*this).preVisit();
			processVisit(std::forward<ValueType>(a_value));
			static_cast<VisitorType&>(*this).postVisit();
		}

		ContextType const& getContext()
		{
			return m_context;
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

		template <typename ValueType, enforce(std::is_arithmetic_v<std::remove_reference_t<ValueType>>)>
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

		/*template <typename BaseType>
		void processVisit(DynamicValue<BaseType> a_dynamicValue)
		{
			m_applicator.apply(
				const_cast<BaseType&>(a_dynamicValue.m_value)
				, static_cast<VisitorType&>(*this)
			);
		}*/

	private:
		// Attributes
		ContextType m_context;
	};
}
