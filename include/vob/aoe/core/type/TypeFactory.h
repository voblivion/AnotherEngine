#pragma once

#include <type_traits>
#include <typeindex>
#include <utility>

#include <vob/sta/memory.h>
#include <vob/sta/utility.h>

#include <vob/aoe/core/type/ADynamicType.h>
#include <vob/aoe/core/type/TypeRegistry.h>

// TODO : can it be done with Type things ?
namespace vob::aoe::type
{
	namespace detail
	{
		class AFactory
			: public ADynamicType
		{
		public:
			// Methods
			virtual sta::polymorphic_ptr<void> create(
				std::pmr::memory_resource* a_resource) = 0;
			virtual std::shared_ptr<void> createShared(
				std::pmr::memory_resource* a_resource) = 0;
		};

		template <typename Type, typename... Args>
		class Factory final
			: public AFactory
		{
		public:
			// Constructors
			explicit Factory(Args... a_args)
				: m_args{ std::forward<Args>(a_args)... }
			{}

			// Methods
			sta::polymorphic_ptr<void> create(std::pmr::memory_resource* a_resource) override
			{
				return createImpl(a_resource
					, std::index_sequence_for<Args...>{});
			}

			std::shared_ptr<void> createShared(std::pmr::memory_resource* a_resource) override
			{
				return createSharedImpl(a_resource, std::index_sequence_for<Args...>{});
			}

		private:
			// Attributes
			std::tuple<Args...> m_args;

			// Methods
			template <std::size_t... indexes>
			sta::polymorphic_ptr<void> createImpl(
				std::pmr::memory_resource* a_resource
				, std::index_sequence<indexes...>)
			{
				return sta::allocate_polymorphic<Type>(
					std::pmr::polymorphic_allocator<Type>(a_resource)
					, std::forward<Args>(std::get<indexes>(m_args))...
				);
			}

			template <std::size_t... indexes>
			std::shared_ptr<void> createSharedImpl(
				std::pmr::memory_resource* a_resource
				, std::index_sequence<indexes...>)
			{
				return std::allocate_shared<Type>(
					std::pmr::polymorphic_allocator<Type>{ a_resource }
					, std::forward<Args>(std::get<indexes>(m_args))...
				);
			}
		};
	}

	class TypeFactory
	{
		// Aliases
		using Allocator = std::pmr::polymorphic_allocator<std::byte>;
		using AFactory = detail::AFactory;
		template <typename Type, typename... Args>
		using Factory = detail::Factory<Type, Args...>;

	public:
		// Constructors
		explicit TypeFactory(TypeRegistry const& a_typeRegistry)
			: m_typeRegistry{ a_typeRegistry }
		{}

		explicit TypeFactory(
			TypeRegistry const& a_typeRegistry
			, std::pmr::memory_resource* a_resource
		)
			: m_typeRegistry{ a_typeRegistry }
			, m_factories{ Allocator{ a_resource } }
		{}

		// Methods
		template <typename Type, typename... Args>
		void addFactory(Args... a_args)
		{
			auto const t_resource = m_factories.get_allocator().resource();
			m_factories.emplace(
				std::type_index{ typeid(Type) }
				, sta::allocate_polymorphic<Factory<Type, Args...>>(
					std::pmr::polymorphic_allocator<Factory<Type, Args...>>{ t_resource }
					, std::forward<Args>(a_args)...
				)
			);
		}

		template <typename Type>
		sta::polymorphic_ptr<Type> create(std::type_index const a_typeId) const
		{
			if (!m_typeRegistry.isBaseOf<Type>(a_typeId))
			{
				return nullptr;
			}

			auto const t_it = m_factories.find(std::type_index{ a_typeId });
			if (t_it != m_factories.end())
			{
				auto const t_resource = m_factories.get_allocator().resource();
				return sta::static_polymorphic_cast<Type>(t_it->second->create(t_resource));
			}
			return nullptr;
		}

		template <typename Type>
		sta::polymorphic_ptr<Type> create(std::uint64_t const a_id) const
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
				auto const t_resource = m_factories.get_allocator().resource();
				return std::static_pointer_cast<Type>(
					t_it->second->createShared(t_resource));
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
		std::pmr::unordered_map<
			std::type_index
			, sta::polymorphic_ptr<AFactory>
		> m_factories;
	};
}