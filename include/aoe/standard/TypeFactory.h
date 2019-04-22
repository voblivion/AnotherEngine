#pragma once

#include <type_traits>
#include <typeindex>
#include <utility>

#include <aoe/standard/ADynamicType.h>
#include <aoe/standard/Allocator.h>
#include <aoe/standard/Memory.h>
#include <aoe/standard/Utility.h>
#include <aoe/standard/TypeRegistry.h>

namespace aoe
{
	namespace sta
	{
		namespace detail
		{
			template <typename Allocator>
			class AFactory
				: public ADynamicType
			{
			public:
				// Methods
				virtual PolymorphicPtr<void> create(Allocator& a_allocator) = 0;
				virtual std::shared_ptr<void> createShared(Allocator& a_allocator) = 0;
			};

			template <typename Allocator, typename Type, typename... Args>
			class Factory final
				: public AFactory<Allocator>
			{
				// Aliases
				using TypeAllocator = RebindAlloc<Allocator, Type>;

			public:
				// Constructors
				explicit Factory(Args... a_args)
					: m_args{ std::forward<Args>(a_args)... }
				{}

				// Methods
				virtual PolymorphicPtr<void> create(Allocator& a_allocator) override
				{
					TypeAllocator t_allocator{ a_allocator };
					return createImpl(a_allocator, std::index_sequence_for<Args...>{});
				}

				virtual std::shared_ptr<void> createShared(
					Allocator& a_allocator) override
				{
					TypeAllocator t_allocator{ a_allocator };
					return createSharedImpl(a_allocator
						, std::index_sequence_for<Args...>{});
				}

			private:
				// Attributes
				std::tuple<Args...> m_args;

				// Methods
				template <std::size_t... indexes>
				PolymorphicPtr<void> createImpl(Allocator& a_allocator
					, std::index_sequence<indexes...>)
				{
					return allocatePolymorphic<Type, Allocator, Args...>(
						a_allocator
						, std::forward<Args>(std::get<indexes>(m_args))...);
				}

				template <std::size_t... indexes>
				std::shared_ptr<void> createSharedImpl(Allocator& a_allocator
					, std::index_sequence<indexes...>)
				{
					return std::allocate_shared<Type, Allocator, Args...>(
						a_allocator
						, std::forward<Args>(std::get<indexes>(m_args))...);
				}
			};
		}

		class TypeFactory
		{
			// Aliases
			using Allocator = Allocator<std::byte>;
			using AFactory = detail::AFactory<Allocator>;
			template <typename Type, typename... Args>
			using Factory = detail::Factory<Allocator, Type, Args...>;
			using MapKey = std::type_index;
			using FactoryAllocator = RebindAlloc<Allocator, AFactory>;
			using MapValue = PolymorphicPtr<AFactory>;
			using MapPair = std::pair<MapKey const, MapValue>;
			using MapAllocator = RebindAlloc<Allocator, MapPair>;

		public:
			// Constructors
			explicit TypeFactory(TypeRegistry const& a_typeRegistry)
				: m_typeRegistry{ a_typeRegistry }
			{}
			template <typename Allocator>
			explicit TypeFactory(TypeRegistry const& a_typeRegistry
				, Allocator const& a_allocator)
				: m_typeRegistry{ a_typeRegistry }
				, m_factories{ a_allocator }
			{}

			// Methods
			template <typename Type, typename... Args>
			void addFactory(Args... a_args)
			{
				m_factories.emplace(std::type_index{ typeid(Type) }
					, allocatePolymorphic<Factory<Type, Args...>>(
						m_factories.get_allocator()
						, std::forward<Args>(a_args)...));
			}

			template <typename Type>
			PolymorphicPtr<Type> create(std::type_index const a_typeId) const
			{
				if (!m_typeRegistry.isBaseOf<Type>(a_typeId))
				{
					return nullptr;
				}

				auto const t_it = m_factories.find(std::type_index{ a_typeId });
				if (t_it != m_factories.end())
				{
					Allocator t_allocator{ m_factories.get_allocator() };
					return staticPolymorphicCast<Type>(
						t_it->second->create(t_allocator));
				}
				return nullptr;
			}

			template <typename Type>
			PolymorphicPtr<Type> create(std::uint64_t const a_id) const
			{
				if (!m_typeRegistry.isUsed(a_id))
				{
					return nullptr;
				}

				return create<Type>(m_typeRegistry.getTypeIndex(a_id));
			}

			template <typename Type>
			std::shared_ptr<Type> createShared(
				std::type_index const a_typeId) const
			{
				if (!m_typeRegistry.isBaseOf<Type>(a_typeId))
				{
					return nullptr;
				}

				auto const t_it = m_factories.find(std::type_index{ a_typeId });
				if (t_it != m_factories.end())
				{
					Allocator t_allocator{ m_factories.get_allocator() };
					return std::static_pointer_cast<Type>(
						t_it->second->createShared(t_allocator));
				}
				return nullptr;
			}

			template <typename Type>
			std::shared_ptr<Type> createShared(std::uint64_t const a_id) const
			{
				if (!m_typeRegistry.isUsed(a_id))
				{
					return nullptr;
				}

				return createShared<Type>(m_typeRegistry.getTypeIndex(a_id));
			}

			TypeRegistry const& getTypeRegistry() const
			{
				return m_typeRegistry;
			}

		private:
			// Attribute
			TypeRegistry const& m_typeRegistry;
			std::unordered_map<MapKey, MapValue, std::hash<MapKey>
				, std::equal_to<>, MapAllocator> m_factories;
		};
	}
}