#pragma once

#include <cassert>
#include <type_traits>
#include <typeindex>

#include <vob/sta/memory.h>
#include <vob/sta/ignorable_assert.h>

#include <vob/aoe/core/type/ADynamicType.h>

namespace vob::aoe::type
{
	namespace detail
	{
		template <typename VoidMaybeConst, typename... Args>
		class ATypeApplicator
			: public type::ADynamicType
		{
			static_assert(std::is_same_v<std::remove_const_t<VoidMaybeConst>, void>);
		public:
			// Methods
			virtual void apply(VoidMaybeConst* a_object, Args&&... a_args) const = 0;
		};

		template <
			typename VoidMaybeConst
			, typename Type
			, template <typename> typename Func
			, typename... Args
		>
		class TypeApplicator final
			: public ATypeApplicator<VoidMaybeConst, Args...>
		{
			static_assert(!(std::is_const_v<VoidMaybeConst> ^ std::is_const_v<Type>));
		public:
			// Constructors
			explicit TypeApplicator(Func<Type> a_functor)
				: m_functor{ std::move(a_functor) }
			{}

			// Methods
			void apply(VoidMaybeConst* a_object, Args&&... a_args) const override
			{
				auto& t_object = *static_cast<Type*>(a_object);
				m_functor(t_object, std::forward<Args>(a_args)...);
			}

		private:
			// Attributes
			Func<Type> m_functor;
		};
	}

	template <
		typename VoidMaybeConst
		, template <typename> typename Func
		, typename... Args
	>
	class Applicator
	{
		// Aliases
		using ATypeApplicator = detail::ATypeApplicator<VoidMaybeConst, Args...>;
			
		template <typename Type>
		using TypeApplicator = detail::TypeApplicator<VoidMaybeConst, Type
			, Func, Args...>;

	public:
		// Constructors
		explicit Applicator(
			std::pmr::polymorphic_allocator<std::byte> a_allocator = {})
			: m_typeApplicators{ a_allocator }
		{}

		// Methods
		bool isRegistered(std::type_index const a_typeIndex) const
		{
			return m_typeApplicators.find(a_typeIndex) != m_typeApplicators.end();
		}

		template <typename Type>
		bool isRegistered() const
		{
			return isRegistered(typeid(Type));
		}

		template <typename Type>
		void registerType(Func<Type> a_functor = {})
		{
			assert(!isRegistered<Type>());
			m_typeApplicators.emplace(
				typeid(Type)
				, sta::allocate_polymorphic<TypeApplicator<Type>>(
					std::pmr::polymorphic_allocator<TypeApplicator<Type>>{ getResource() }
					, a_functor
				)
			);
		}

		template <typename BaseType>
		void apply(BaseType& a_object, Args&&... a_args) const
		{
			ignorable_assert(isRegistered(typeid(a_object)));
			auto const t_it = m_typeApplicators.find(typeid(a_object));
			if(t_it != m_typeApplicators.end())
			{
				t_it->second->apply(&a_object, std::forward<Args>(a_args)...);
			}
		}

		auto getResource()
		{
			return m_typeApplicators.get_allocator().resource();
		}

	private:
		// Attributes
		std::pmr::unordered_map<std::type_index
			, sta::polymorphic_ptr<ATypeApplicator>> m_typeApplicators;
	};
}
