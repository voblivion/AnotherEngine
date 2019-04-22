#pragma once
#include <cinttypes>
#include <aoe/standard/Memory.h>
#include <aoe/standard/PropagateConst.h>

namespace aoe
{
	namespace visitor
	{
		template <typename VisitorType, typename ObjectType, typename Enable = void>
		struct IsVisitable : std::false_type {};

		template <typename VisitorType, typename ObjectType>
		struct IsVisitable<VisitorType, ObjectType, std::enable_if_t<
			std::is_invocable_r_v<
			void
			, decltype(&ObjectType::accept)
			, ObjectType*
			, VisitorType&
			>
			>> : std::true_type{};

		template <typename VisitorType, typename ObjectType>
		struct IsVisitable<VisitorType, ObjectType, std::enable_if_t<
			std::is_invocable_r_v<
			void
			, decltype(&ObjectType::template accept<VisitorType>)
			, ObjectType*
			, VisitorType&
			>
			>> : std::true_type{};

		enum class AccessType
		{
			Reader,
			Writer
		};

		template <typename VisitorType, typename ValueType, std::enable_if_t<
			IsVisitable<VisitorType, ValueType>::value
		>* = nullptr>
			void makeVisit(VisitorType& a_visitor, ValueType& a_value)
		{
			a_value.accept(a_visitor);
		}

		struct SizeTag
		{
		public:
			explicit SizeTag(std::size_t const a_size = 0)
				: m_size{ a_size }
			{}

			std::size_t m_size;
		};

		template <typename BaseType>
		struct DynamicValue
		{
		public:
			// ReSharper disable once CppNonExplicitConvertingConstructor
			DynamicValue(BaseType& a_value)
				: m_value{ a_value }
			{}

			BaseType& m_value;
		};

		template <typename BaseType>
		DynamicValue<BaseType> makeDynamicValue(BaseType& a_value)
		{
			return { a_value };
		}

		template <typename VisitorType, typename BaseType>
		void makeVisit(VisitorType& a_visitor, sta::PolymorphicPtr<BaseType>& a_ptr)
		{
			auto& t_typeFactory = a_visitor.getTypeFactory();
			auto& t_typeRegistry = t_typeFactory.getTypeRegistry();

			std::uint64_t t_id;
			a_visitor.visit("type_id", t_id);
			a_ptr = t_typeFactory.template create<BaseType>(t_id);
			if (a_ptr)
			{
				a_visitor.visit("data", makeDynamicValue(*a_ptr));
			}
		}

		template <typename VisitorType, typename BaseType>
		void makeVisit(VisitorType& a_visitor, std::shared_ptr<BaseType>& a_ptr)
		{
			auto& t_typeFactory = a_visitor.getTypeFactory();
			auto& t_typeRegistry = t_typeFactory.getTypeRegistry();

			std::uint64_t t_id;
			a_visitor.visit("type_id", t_id);
			a_ptr = t_typeFactory.template createShared<BaseType>(t_id);
			if (a_ptr)
			{
				a_visitor.visit("data", makeDynamicValue(*a_ptr));
			}
		}

		template <typename VisitorType, typename BaseType>
		void makeVisit(VisitorType& a_visitor, DynamicValue<BaseType> const& a_value)
		{
			auto& t_typeVisitorApplicator = a_visitor.getApplicator();
			t_typeVisitorApplicator.apply(a_visitor, a_value.m_value);
		}
	}
}