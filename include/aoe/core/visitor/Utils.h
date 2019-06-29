#pragma once
#include <cinttypes>
#include <aoe/core/standard/Memory.h>
#include <aoe/core/standard/PropagateConst.h>
#include <aoe/core/standard/IgnorableAssert.h>

namespace aoe
{
	namespace visitor
	{
		enum class AccessType
		{
			Reader,
			Writer
		};

		template <typename VisitorType>
		using ReaderType = std::enable_if_t<
			VisitorType::accessType == AccessType::Reader>;

		template <typename VisitorType>
		using WriterType = std::enable_if_t<
			VisitorType::accessType == AccessType::Writer>;

		enum class VisitType
		{
			Sequential,
			Random
		};

		template <typename VisitorType>
		using SequentialType = std::enable_if_t<
			VisitorType::visitType == VisitType::Sequential>;

		template <typename VisitorType>
		using RandomType = std::enable_if_t<
			VisitorType::visitType == VisitType::Random>;

		template <typename VisitorType, typename ObjectType>
		using AcceptType = std::enable_if_t<std::is_invocable_r_v<
			void
			, decltype(&ObjectType::accept)
			, ObjectType*
			, VisitorType&>>;

		template <typename VisitorType, typename ObjectType>
		using TemplateAcceptType = std::enable_if_t<
			std::is_invocable_r_v<
			void
			, decltype(&ObjectType::template accept<VisitorType>)
			, ObjectType*
			, VisitorType&>>;

		template <typename VisitorType, typename ObjectType, typename Enable = void>
		struct IsVisitable : std::false_type {};

		template <typename VisitorType, typename ObjectType>
		struct IsVisitable<VisitorType, ObjectType
			, AcceptType<VisitorType, ObjectType>> : std::true_type{};

		template <typename VisitorType, typename ObjectType>
		struct IsVisitable<VisitorType, ObjectType
			, TemplateAcceptType<VisitorType, ObjectType>> : std::true_type{};

		template <typename VisitorType, typename ValueType>
		using VisitableType = std::enable_if_t<IsVisitable<VisitorType, ValueType>::value>;

		template <typename VisitorType, typename ValueType
			, VisitableType<VisitorType, ValueType>* = nullptr>
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

		template <typename VisitorType>
		std::uint64_t visitTypeId(VisitorType& a_visitor)
		{
			std::uint64_t t_id;
			a_visitor.visit("type_id", t_id);
			return t_id;
		}

		template <typename VisitorType, typename PointerType>
		void visitData(VisitorType& a_visitor, PointerType& a_ptr)
		{
			ignorableAssert(a_ptr != nullptr);
			if (a_ptr != nullptr)
			{
				a_visitor.visit("data", makeDynamicValue(*a_ptr));
			}
		}

		template <typename VisitorType, typename BaseType>
		void makeVisit(VisitorType& a_visitor, sta::PolymorphicPtr<BaseType>& a_ptr)
		{

			auto t_id = visitTypeId(a_visitor);

			auto& t_typeFactory = a_visitor.getTypeFactory();
			a_ptr = t_typeFactory.template create<BaseType>(t_id);

			visitData(a_visitor, a_ptr);
		}

		template <typename VisitorType, typename BaseType>
			void makeVisit(VisitorType& a_visitor, std::shared_ptr<BaseType>& a_ptr)
		{
			auto t_id = visitTypeId(a_visitor);

			auto& t_typeFactory = a_visitor.getTypeFactory();
			a_ptr = t_typeFactory.template createShared<BaseType>(t_id);

			visitData(a_visitor, a_ptr);
		}

		template <typename VisitorType, typename BaseType>
		void makeVisit(VisitorType& a_visitor, DynamicValue<BaseType> const& a_value)
		{
			auto& t_typeVisitorApplicator = a_visitor.getApplicator();
			t_typeVisitorApplicator.apply(a_visitor, a_value.m_value);
		}
	}
}