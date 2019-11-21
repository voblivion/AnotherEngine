#pragma once

#include <vob/sta/memory.h>
#include <vob/aoe/core/type/Traits.h>
#include <vob/aoe/core/type/Applicator.h>

namespace vob::aoe::type
{
	namespace detail
	{
		template <typename TypeT>
		struct DoClone
		{
			void operator()(
				TypeT const& a_source
				, sta::polymorphic_ptr<void>& a_target
				, std::pmr::memory_resource& a_resource
			) const
			{
				using Type = std::remove_const_t<TypeT>;
				a_target = sta::allocate_polymorphic<std::remove_const_t<Type>>(
					std::pmr::polymorphic_allocator<Type>{ &a_resource }, a_source
				);
			}
		};
	}

	class CloneCopier
	{
	public:
		// Constructors
		explicit CloneCopier(std::pmr::memory_resource& a_resource)
			: m_applicator{ &a_resource }
			, m_cloneResource{ a_resource }
		{}

		// Methods
		template <typename BaseType, typename SubType
			, enforce((std::is_base_of_v<BaseType, SubType>))>
		sta::polymorphic_ptr<BaseType> clone(
			sta::polymorphic_ptr<SubType> const& a_source) const
		{
			if(a_source == nullptr)
			{
				return nullptr;
			}

			sta::polymorphic_ptr<void> t_target;
			m_applicator.apply(*a_source, t_target, m_cloneResource);
			return sta::static_polymorphic_cast<BaseType>(std::move(t_target));
		}

		template <typename Type>
		bool isRegistered()
		{
			return m_applicator.isRegistered<Type>();
		}

		template <typename Type>
		void registerType()
		{
			m_applicator.registerType<Type const>();
		}

	private:
		// Attributes
		Applicator<
			void const, detail::DoClone
			, sta::polymorphic_ptr<void>&
			, std::pmr::memory_resource&
		> m_applicator;
		std::reference_wrapper<std::pmr::memory_resource> m_cloneResource;
	};

	template <typename Type>
	class Clone
		: public sta::polymorphic_ptr<Type>
	{
		using Base = sta::polymorphic_ptr<Type>;
	public:
		// Constructors
		explicit Clone(CloneCopier const& a_cloneCopier)
			: m_cloneCopier{a_cloneCopier}
		{}
		Clone(Clone&&) = default;
		Clone(Clone const& a_other)
			: Base{ a_other.m_cloneCopier.get().template clone<Type>(a_other) }
			, m_cloneCopier{ a_other.m_cloneCopier }
		{}

		template <typename OtherType = Type>
		explicit Clone(CloneCopier const& a_cloneCopier, sta::polymorphic_ptr<OtherType> a_ptr)
			: Base{ std::move(a_ptr) }
			, m_cloneCopier{ a_cloneCopier }
		{}

		~Clone() = default;

		// Operators
		Clone& operator=(Clone&&) = default;
		Clone& operator=(Clone const& a_other)
		{
			m_cloneCopier = a_other.m_cloneCopier;
			static_cast<Base&>(*this) =
				m_cloneCopier.get().template clone<Type>(a_other);
			return *this;
		}

	private:
		std::reference_wrapper<CloneCopier const> m_cloneCopier;
	};
}
