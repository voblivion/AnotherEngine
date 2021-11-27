#pragma once

#include <type_traits>
#include <typeindex>
#include <utility>

#include <vob/misc/std/polymorphic_ptr.h>
#include <vob/misc/std/polymorphic_ptr_util.h>

#include <vob/aoe/core/type/ADynamicType.h>
#include <vob/aoe/core/type/TypeRegistry.h>

// TODO : can it be done with Type things ?
namespace vob::aoe::type
{
	namespace detail
	{
		template <
			typename PolymorphicBaseType
			, typename AllocatorType
		>
		class AFactory
			: public ADynamicType
		{
			static_assert(std::has_virtual_destructor_v<PolymorphicBaseType>);
		public:
			// Methods
			virtual mistd::polymorphic_ptr<PolymorphicBaseType> create(AllocatorType const& a_allocator) = 0;
			virtual std::shared_ptr<PolymorphicBaseType> createShared(AllocatorType const& a_allocator) = 0;
		};

		template <
			typename PolymorphicBaseType
			, typename AllocatorType
			, typename Type
			, typename... Args>
		class Factory final
			: public AFactory<PolymorphicBaseType, AllocatorType>
		{
			static_assert(std::is_base_of_v<PolymorphicBaseType, Type>);
		public:
			// Constructors
			explicit Factory(Args&&... a_args)
				: m_args{ std::forward<Args>(a_args)... }
			{}

			// Methods
			mistd::polymorphic_ptr<PolymorphicBaseType> create(AllocatorType const& a_allocator) override
			{
				return createImpl(a_allocator, std::index_sequence_for<Args...>{});
			}

			std::shared_ptr<PolymorphicBaseType> createShared(AllocatorType const& a_allocator) override
			{
				return createSharedImpl(a_allocator, std::index_sequence_for<Args...>{});
			}

		private:
			// Attributes
			std::tuple<Args...> m_args;

			// Methods
			template <std::size_t... indexes>
			mistd::polymorphic_ptr<PolymorphicBaseType> createImpl(
				AllocatorType const& a_allocator
				, std::index_sequence<indexes...>
			)
			{
				return mistd::polymorphic_ptr_util::allocate<Type>(
					a_allocator
					, std::forward<Args>(std::get<indexes>(m_args))...
				);
			}

			template <std::size_t... indexes>
			std::shared_ptr<PolymorphicBaseType> createSharedImpl(
				AllocatorType const& a_allocator
				, std::index_sequence<indexes...>
			)
			{
				return std::allocate_shared<Type>(
					a_allocator
					, std::forward<Args>(std::get<indexes>(m_args))...
				);
			}
		};
	}

	template <
		typename PolymorphicBaseType
		, typename AllocatorType = std::pmr::polymorphic_allocator<PolymorphicBaseType>
		, typename FactoryAllocatorType = AllocatorType
	>
	class TypeFactory
	{
		// Aliases
		using AFactory = detail::AFactory<PolymorphicBaseType, AllocatorType>;
		template <typename Type, typename... Args>
		using Factory = detail::Factory<PolymorphicBaseType, AllocatorType, Type, Args...>;

	public:
		// Constructors
		explicit TypeFactory(
			TypeRegistry const& a_typeRegistry
			, AllocatorType a_allocator = {}
			, FactoryAllocatorType a_factoryAllocator = {}
		)
			: m_typeRegistry{ a_typeRegistry }
			, m_allocator{ a_allocator }
			, m_factories{ a_factoryAllocator }
		{}

		// Methods
		auto getAllocator() const
		{
			return m_allocator;
		}

		template <typename Type, typename... Args>
		void addFactory(Args&&... a_args)
		{
			auto allocator = m_factories.get_allocator();
			m_factories.emplace(
				std::type_index{ typeid(Type) }
				, std::allocate_shared<Factory<Type, Args...>>(allocator, std::forward<Args>(a_args)...)
			);
		}

		template <typename Type>
		mistd::polymorphic_ptr<Type> create(std::type_index const a_typeId) const
		{
			if (!m_typeRegistry.isBaseOf<Type>(a_typeId))
			{
				return nullptr;
			}

			auto const t_it = m_factories.find(std::type_index{ a_typeId });
			if (t_it != m_factories.end())
			{
				return mistd::polymorphic_ptr_util::cast<Type>(t_it->second->create(m_allocator));
			}
			return nullptr;
		}

		template <typename Type>
		mistd::polymorphic_ptr<Type> create(mishs::string_id const a_id) const
		{
			auto typeIndex = m_typeRegistry.findTypeIndex(a_id);
			return typeIndex != nullptr ? create<Type>(*typeIndex) : nullptr;
		}

		template <typename Type>
		std::shared_ptr<Type> createShared(std::type_index const a_typeId) const
		{
			if (!m_typeRegistry.isBaseOf<Type>(a_typeId))
			{
				return nullptr;
			}

			auto const t_it = m_factories.find(std::type_index{ a_typeId });
			if (t_it != m_factories.end())
			{
				return std::static_pointer_cast<Type>(t_it->second->createShared(m_allocator));
			}
			return nullptr;
		}

		template <typename Type>
		std::shared_ptr<Type> createShared(mishs::string_id const a_id) const
		{
			auto typeIndex = m_typeRegistry.findTypeIndex(a_id);
			return typeIndex != nullptr ? createShared<Type>(*typeIndex) : nullptr;
		}

		TypeRegistry const& getTypeRegistry() const
		{
			return m_typeRegistry;
		}

	private:
		// Attribute
		TypeRegistry const& m_typeRegistry;
		AllocatorType m_allocator;
		using MapAllocator = typename std::allocator_traits<FactoryAllocatorType>::template rebind_alloc<
			std::pair<std::type_index const, std::shared_ptr<AFactory>>
		>;
		std::unordered_map<
			std::type_index
			, std::shared_ptr<AFactory>
			, std::hash<std::type_index>
            , std::equal_to<std::type_index>
            , MapAllocator
		> m_factories;
	};
}