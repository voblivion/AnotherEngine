#pragma once

#include <vob/aoe/core/type/Traits.h>
#include <vob/aoe/core/type/Applicator.h>

namespace vob::aoe::type
{
	template <typename PolymorphicBaseType>
	class CloneCopier
	{
		template <typename TypeT>
		struct DoClone
		{
			void operator()(
				TypeT const& a_source
				, std::unique_ptr<PolymorphicBaseType>& a_target
				) const
			{
				using Type = std::remove_const_t<TypeT>;

				a_target = std::make_unique<Type>(a_source);
			}
		};
	public:
		// Methods
		template <typename BaseType, typename SubType, enforce((std::is_base_of_v<BaseType, SubType>))>
		std::unique_ptr<BaseType> clone(std::unique_ptr<SubType> const& a_source) const
		{
			static_assert(std::is_base_of_v<PolymorphicBaseType, BaseType>);
			if(a_source == nullptr)
			{
				return nullptr;
			}

			std::unique_ptr<PolymorphicBaseType> t_target;
			m_applicator.apply(*a_source, t_target);
			return std::unique_ptr<BaseType>{ static_cast<BaseType*>(t_target.release()) };
		}

		template <typename Type>
		bool isRegistered()
		{
			static_assert(std::is_base_of_v<PolymorphicBaseType, Type>);
			return m_applicator.template isRegistered<Type>();
		}

		template <typename Type>
		void registerType()
		{
			static_assert(std::is_base_of_v<PolymorphicBaseType, Type>);
			m_applicator.template registerType<Type const>();
		}

	private:
		// Attributes
		Applicator<PolymorphicBaseType const, DoClone, std::unique_ptr<PolymorphicBaseType>&> m_applicator;
	};

	template <typename Type, typename PolymorphicBaseType>
	class Clone
		: public std::unique_ptr<Type>
	{
		using Base = std::unique_ptr<Type>;
	public:
		#pragma region Constructors
		explicit Clone(CloneCopier<PolymorphicBaseType> const& a_cloneCopier)
			: m_cloneCopier{a_cloneCopier}
		{}

		Clone(Clone&&) = default;

		Clone(Clone const& a_other)
			: Base{ a_other.m_cloneCopier.get().template clone<Type>(a_other) }
			, m_cloneCopier{ a_other.m_cloneCopier }
		{}

		template <typename OtherType = Type>
		explicit Clone(
			CloneCopier<PolymorphicBaseType> const& a_cloneCopier
			, std::unique_ptr<OtherType> a_ptr
		)
			: Base{ std::move(a_ptr) }
			, m_cloneCopier{ a_cloneCopier }
		{}

		~Clone() = default;
		#pragma endregion

		#pragma region Operators
		Clone& operator=(Clone&&) = default;
		Clone& operator=(Clone const& a_other)
		{
			m_cloneCopier = a_other.m_cloneCopier;
			static_cast<Base&>(*this) =
				m_cloneCopier.get().template clone<Type>(a_other);
			return *this;
		}
		#pragma endregion

	private:
		#pragma region Attributes
		std::reference_wrapper<CloneCopier<PolymorphicBaseType> const> m_cloneCopier;
		#pragma endregion
	};
}
