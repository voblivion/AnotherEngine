#pragma once

#include <aoe/core/standard/Memory.h>
#include <aoe/core/type/Applicator.h>

namespace aoe::type
{
	namespace detail
	{
		template <typename Type>
		struct DoClone
		{
			void operator()(Type const& a_source
				, sta::PolymorphicPtr<void>& a_target
				, std::pmr::memory_resource& a_resource) const
			{
				a_target = sta::allocatePolymorphicWith<
					std::remove_const_t<Type>>(&a_resource, a_source);
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
		sta::PolymorphicPtr<BaseType> clone(
			sta::PolymorphicPtr<SubType> const& a_source) const
		{
			if(a_source == nullptr)
			{
				return nullptr;
			}

			sta::PolymorphicPtr<void> t_target;
			m_applicator.apply(*a_source, t_target, m_cloneResource);
			return sta::staticPolymorphicCast<BaseType>(std::move(t_target));
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
		Applicator<void const, detail::DoClone, sta::PolymorphicPtr<void>&
			, std::pmr::memory_resource&> m_applicator;
		std::reference_wrapper<std::pmr::memory_resource> m_cloneResource;
	};

	template <typename Type>
	class Clone
		: public sta::PolymorphicPtr<Type>
	{
		using Base = sta::PolymorphicPtr<Type>;
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
		explicit Clone(CloneCopier const& a_cloneCopier
			, sta::PolymorphicPtr<OtherType> a_ptr)
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
		}

	private:
		std::reference_wrapper<CloneCopier const> m_cloneCopier;
	};
}
