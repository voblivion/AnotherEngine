#pragma once

#include <type_traits>
#include <typeindex>
#include <utility>

#include <aoe/core/standard/ADynamicType.h>
#include <aoe/core/standard/Allocator.h>
#include <aoe/core/standard/Memory.h>
#include <aoe/core/standard/Utility.h>
#include <aoe/core/standard/TypeRegistry.h>

namespace aoe::sta
{
	namespace detail
	{
		class AFactory
			: public ADynamicType
		{
		public:
			// Methods
			virtual PolymorphicPtr<void> create(
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
			virtual PolymorphicPtr<void> create(
				std::pmr::memory_resource* a_resource) override
			{
				return createImpl(a_resource
					, std::index_sequence_for<Args...>{});
			}

			virtual std::shared_ptr<void> createShared(
				std::pmr::memory_resource* a_resource) override
			{
				return createSharedImpl(a_resource
					, std::index_sequence_for<Args...>{});
			}

		private:
			// Attributes
			std::tuple<Args...> m_args;

			// Methods
			template <std::size_t... indexes>
			PolymorphicPtr<void> createImpl(
				std::pmr::memory_resource* a_resource
				, std::index_sequence<indexes...>)
			{
				return allocatePolymorphicWith<Type, Args...>(
					a_resource
					, std::forward<Args>(std::get<indexes>(m_args))...);
			}

			template <std::size_t... indexes>
			std::shared_ptr<void> createSharedImpl(
				std::pmr::memory_resource* a_resource
				, std::index_sequence<indexes...>)
			{
				return std::allocate_shared<Type, sta::Allocator<Type>, Args...>(
					sta::Allocator<Type>{ a_resource }
					, std::forward<Args>(std::get<indexes>(m_args))...);
			}
		};
	}

	class TypeFactory
	{
		// Aliases
		using Allocator = Allocator<std::byte>;
		using AFactory = detail::AFactory;
		template <typename Type, typename... Args>
		using Factory = detail::Factory<Type, Args...>;

	public:
		// Constructors
		explicit TypeFactory(TypeRegistry const& a_typeRegistry)
			: m_typeRegistry{ a_typeRegistry }
		{}

		explicit TypeFactory(TypeRegistry const& a_typeRegistry
			, std::pmr::memory_resource* a_resource)
			: m_typeRegistry{ a_typeRegistry }
			, m_factories{ Allocator{ a_resource } }
		{}

		// Methods
		template <typename Type, typename... Args>
		void addFactory(Args... a_args)
		{
			auto const t_resource = m_factories.get_allocator().resource();
			m_factories.emplace(std::type_index{ typeid(Type) }
				, allocatePolymorphicWith<Factory<Type, Args...>>(
					t_resource
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
				auto const t_resource = m_factories.get_allocator().resource();
				return staticPolymorphicCast<Type>(
					t_it->second->create(t_resource));
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
		std::pmr::unordered_map<std::type_index
			, PolymorphicPtr<AFactory>> m_factories;
	};
}