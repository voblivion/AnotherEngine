#pragma once

#include <cassert>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

#include <vob/sta/ignorable_assert.h>
#include <vob/sta/type_traits.h>

#include <vob/aoe/core/type/ADynamicType.h>

namespace vob::aoe::type
{
	namespace detail
	{
		template <typename PolymorphicBaseType, typename... Args>
		class ATypeApplicator
			: public type::ADynamicType
		{
			static_assert(std::has_virtual_destructor_v<PolymorphicBaseType>);
		public:
			// Methods
			virtual void apply(PolymorphicBaseType* a_object, Args&&... a_args) const = 0;
		};

		template <
			typename PolymorphicBaseType
			, typename Type
			, template <typename> typename Func
			, typename... Args
		>
		class TypeApplicator final
			: public ATypeApplicator<PolymorphicBaseType, Args...>
        {
            static_assert(std::is_base_of_v<PolymorphicBaseType, Type>);
		public:
			// Constructors
			explicit TypeApplicator(Func<Type> a_functor)
				: m_functor{ std::move(a_functor) }
			{}

			// Methods
			void apply(PolymorphicBaseType* a_object, Args&&... a_args) const override
			{
				using _Type = sta::same_const_t<PolymorphicBaseType, Type>;
				auto& t_object = *static_cast<_Type*>(a_object);
				m_functor(t_object, std::forward<Args>(a_args)...);
			}

		private:
			// Attributes
			Func<Type> m_functor;
		};
	}

	template <
		typename PolymorphicBaseType
		, typename AllocatorType
		, template <typename> typename Func
		, typename... Args
	>
	class Applicator
	{
		// Aliases
		using ATypeApplicator = detail::ATypeApplicator<PolymorphicBaseType, Args...>;
			
		template <typename Type>
		using TypeApplicator = detail::TypeApplicator<PolymorphicBaseType, Type, Func, Args...>;

	public:
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
			auto allocator = m_typeApplicators.get_allocator();
			m_typeApplicators.emplace(
				typeid(Type)
				, std::allocate_shared<TypeApplicator<Type>>(allocator, a_functor)
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

	private:
        // Attributes
        using MapAllocator = typename std::allocator_traits<AllocatorType>::template rebind_alloc<
            std::pair<std::type_index const, std::shared_ptr<ATypeApplicator>>
        >;
		std::unordered_map<
			std::type_index
			, std::shared_ptr<ATypeApplicator>
			, std::hash<std::type_index>
			, std::equal_to<std::type_index>
			, MapAllocator
		> m_typeApplicators;
	};
}
