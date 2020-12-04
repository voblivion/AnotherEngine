#pragma once

#include <type_traits>
#include <typeindex>
#include <utility>

#include <vob/sta/utility.h>

#include <vob/aoe/core/type/ADynamicType.h>
#include <vob/aoe/core/type/TypeRegistry.h>

// TODO : can it be done with Type things ?
namespace vob::aoe::type
{
	namespace detail
	{
		template <typename PolymorphicBaseType>
		class AFactory
			: public ADynamicType
		{
			static_assert(std::has_virtual_destructor_v<PolymorphicBaseType>);
		public:
			// Methods
			virtual std::unique_ptr<PolymorphicBaseType> create() = 0;
			virtual std::shared_ptr<PolymorphicBaseType> createShared() = 0;
		};

		template <typename PolymorphicBaseType, typename Type, typename... Args>
		class Factory final
			: public AFactory<PolymorphicBaseType>
		{
			static_assert(std::is_base_of_v<PolymorphicBaseType, Type>);
		public:
			// Constructors
			explicit Factory(Args&&... a_args)
				: m_args{ std::forward<Args>(a_args)... }
			{}

			// Methods
			std::unique_ptr<PolymorphicBaseType> create() override
			{
				return createImpl(std::index_sequence_for<Args...>{});
			}

			std::shared_ptr<PolymorphicBaseType> createShared() override
			{
				return createSharedImpl(std::index_sequence_for<Args...>{});
			}

		private:
			// Attributes
			std::tuple<Args...> m_args;

			// Methods
			template <std::size_t... indexes>
			std::unique_ptr<PolymorphicBaseType> createImpl(std::index_sequence<indexes...>)
			{
				return std::make_unique<Type>(std::forward<Args>(std::get<indexes>(m_args))...);
			}

			template <std::size_t... indexes>
			std::shared_ptr<PolymorphicBaseType> createSharedImpl(std::index_sequence<indexes...>)
			{
				return std::make_shared<Type>(std::forward<Args>(std::get<indexes>(m_args))...);
			}
		};
	}

	template <typename PolymorphicBaseType>
	class TypeFactory
	{
		// Aliases
		using AFactory = detail::AFactory<PolymorphicBaseType>;
		template <typename Type, typename... Args>
		using Factory = detail::Factory<PolymorphicBaseType, Type, Args...>;

	public:
		// Constructors
		explicit TypeFactory(TypeRegistry const& a_typeRegistry)
			: m_typeRegistry{ a_typeRegistry }
		{}

		// Methods
		template <typename Type, typename... Args>
		void addFactory(Args&&... a_args)
		{
			m_factories.emplace(
				std::type_index{ typeid(Type) }
				, std::make_unique<Factory<Type, Args...>>(std::forward<Args>(a_args)...)
			);
		}

		template <typename Type>
		std::unique_ptr<Type> create(std::type_index const a_typeId) const
		{
			if (!m_typeRegistry.isBaseOf<Type>(a_typeId))
			{
				return nullptr;
			}

			auto const t_it = m_factories.find(std::type_index{ a_typeId });
			if (t_it != m_factories.end())
			{
				return std::unique_ptr<Type>{ static_cast<Type*>(t_it->second->create().release()) };
			}
			return nullptr;
		}

		template <typename Type>
		std::unique_ptr<Type> create(std::uint64_t const a_id) const
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
				return std::static_pointer_cast<Type>(t_it->second->createShared());
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
		std::unordered_map<std::type_index, std::unique_ptr<AFactory>> m_factories;
	};
}