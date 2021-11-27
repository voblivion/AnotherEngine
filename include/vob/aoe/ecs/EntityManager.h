#pragma once

#include <deque>
#include <unordered_map>
#include <algorithm>
#include <optional>

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/Entity.h>
#include <vob/aoe/ecs/EntityId.h>
#include <vob/aoe/ecs/EntityView.h>
#include <vob/aoe/ecs/SystemSpawnManager.h>
#include <vob/aoe/ecs/SystemUnspawnManager.h>
#include <vob/aoe/ecs/EntityHandle.h>

#include <vob/aoe/core/type/ADynamicType.h>

namespace vob::aoe::aoecs
{
	using EntityMap = std::unordered_map<EntityId, std::unique_ptr<Entity>>;

	namespace detail
	{
		template <typename SystemType>
		class HasOnEntityAdded
		{
		private:
			template <typename S>
			static auto test(int)
				-> decltype(std::declval<S>().onEntityAdded(std::declval<Entity&>()), std::true_type{})
			{
				return std::true_type{};
			}

			template <typename>
			static std::false_type test(...) { return std::false_type{}; };

		public:
			static constexpr bool value = std::is_same_v<decltype(test<SystemType>(0)), std::true_type>;
		};

		template <typename SystemType>
		constexpr auto hasOnEntityAddedV = HasOnEntityAdded<SystemType>::value;

		template <typename SystemType
			, std::enable_if_t<hasOnEntityAddedV<SystemType>>* = nullptr>
		void callOnEntityAdded(SystemType& a_system, Entity& a_entity)
		{
			a_system.onEntityAdded(a_entity);
		}

		template <typename SystemType
			, std::enable_if_t<!hasOnEntityAddedV<SystemType>>* = nullptr>
		void callOnEntityAdded(SystemType& a_system, Entity& a_entity)
		{

		}

		template <typename SystemType>
		class HasOnEntityRemoved
		{
		private:
			template <typename S>
			static auto test(int)
				-> decltype(std::declval<S>().onEntityRemoved(std::declval<Entity&>()), std::true_type{})
			{
				return std::true_type{};
			}

			template <typename>
			static std::false_type test(...) { return std::false_type{}; };

		public:
			static constexpr bool value = std::is_same_v<decltype(test<SystemType>(0)), std::true_type>;
		};

		template <typename SystemType>
		constexpr auto hasOnEntityRemovedV = HasOnEntityRemoved<SystemType>::value;

		template <typename SystemType
			, std::enable_if_t<hasOnEntityRemovedV<SystemType>>* = nullptr>
		void callOnEntityRemoved(SystemType& a_system, Entity& a_entity)
		{
			a_system.onEntityRemoved(a_entity);
		}

		template <typename SystemType
			, std::enable_if_t<!hasOnEntityRemovedV<SystemType>>* = nullptr>
		void callOnEntityRemoved(SystemType& a_system, Entity const& a_entity)
		{

		}

		template <typename... ComponentTypes>
		struct SystemEntityTester;

		template <>
		struct SystemEntityTester<>
		{
			static bool test(Entity const& a_entity)
			{
				return true;
			}
		};

		template <typename ComponentType, typename... ComponentTypes>
		struct SystemEntityTester<ComponentType, ComponentTypes...>
		{
			static bool test(Entity const& a_entity)
			{
				return a_entity.hasComponent<std::remove_const_t<ComponentType>>()
					&& SystemEntityTester<ComponentTypes...>::test(a_entity);
			}
		};

		template <typename ComponentType, typename... ComponentTypes>
		struct SystemEntityTester<ComponentType*, ComponentTypes...>
		{
			static bool test(Entity const& a_entity)
			{
				return SystemEntityTester<ComponentTypes...>::test(a_entity);
			}
		};
	}

	template <typename... ComponentTypes>
	struct EntityViewList
	{
		// Aliases
		using EntityViewType = EntityView<ComponentTypes...>;

		// Attributes
		std::deque<EntityViewType> m_list;
		std::unordered_map<EntityId, std::size_t> m_entityIndexes;

		// Methods
		void add(Entity& a_entity)
		{
			m_entityIndexes.emplace(a_entity.getId(), m_list.size());
			m_list.emplace_back(a_entity);
		}

		void remove(EntityId const a_id)
		{
			// todo : pass it ?
			auto t_it = m_entityIndexes.find(a_id);
			if (t_it != m_entityIndexes.end())
			{
				auto t_listIt = m_list.begin();
				t_listIt += t_it->second;

				auto t_endListIt = m_list.end();
				--t_endListIt;
				auto t_endIt = m_entityIndexes.find(t_endListIt->getId());

				t_endIt->second = std::distance(m_list.begin(), t_listIt);
				auto t_tmp = std::move(*t_listIt);
				*t_listIt = std::move(*t_endListIt);
				*t_endListIt = std::move(t_tmp);

				m_entityIndexes.erase(t_it);
				m_list.pop_back();
			}
		}

		bool empty() const
		{
			return m_list.empty();
		}

		auto begin()
		{
			return m_list.begin();
		}

		auto begin() const
		{
			return m_list.begin();
		}

		auto const& front() const
		{
			return *begin();
		}

		auto end()
		{
			return m_list.end();
		}

		auto end() const
		{
			return m_list.end();
		}

		EntityViewType const* find(EntityHandle const& a_handle) const
		{
			// todo : return it ?
			return find(a_handle.m_id);
		}

		EntityViewType const* find(EntityId const a_id) const
		{
			// todo : return it ? optional ?
			auto const t_it = m_entityIndexes.find(a_id);
			if (t_it != m_entityIndexes.end())
			{
				return &m_list[t_it->second];
			}
			return nullptr;
		}
	};

	template <typename TEntityList>
	class EntityHandleChecker
	{
	public:
		explicit EntityHandleChecker(TEntityList const& a_entities)
			: m_entities{ a_entities }
		{}

		bool operator()(aoecs::EntityHandle const& a_entityHandle) const
		{
			return m_entities.find(a_entityHandle) != nullptr;
		}

	private:
		TEntityList const& m_entities;
	};

	struct ASystemEntityList
		: public type::ADynamicType
	{
		// Methods
		virtual void onEntityAdded(Entity& a_entity) = 0;
		virtual void onEntityRemoved(Entity& a_entity) = 0;
	};

	template <typename SystemType, typename... ComponentTypes>
	struct SystemEntityList final
		: public ASystemEntityList
	{
		using EntityViewType = EntityView<ComponentTypes...>;

		// Attributes
		SystemType& m_system;
		EntityViewList<ComponentTypes...> m_entityList;

		// Constructor
		explicit SystemEntityList(
			SystemType& a_system
			, EntityMap const& a_entities
		)
			: m_system{ a_system }
		{
			for (auto& t_pair : a_entities)
			{
				onEntityAdded(*t_pair.second);
			}
		}

		// Methods
		virtual void onEntityAdded(Entity& a_entity) override
		{
			if (detail::SystemEntityTester<ComponentTypes...>::test(a_entity))
			{
				m_entityList.add(a_entity);
				detail::callOnEntityAdded(m_system, a_entity);
			}
		}

		virtual void onEntityRemoved(Entity& a_entity) override
		{
			auto t_entity = m_entityList.find(a_entity.getId());
			if(t_entity != nullptr)
			{
				detail::callOnEntityRemoved(m_system, a_entity);
				m_entityList.remove(a_entity.getId());
			}
		}
	};

	class EntityManager
	{
	public:
		// Constructors
		VOB_AOE_API explicit EntityManager();

		VOB_AOE_API ~EntityManager();

		//Methods
		VOB_AOE_API SystemSpawnManager& getSystemSpawnManager();

		VOB_AOE_API SystemUnspawnManager& getSystemUnspawnManager();

		VOB_AOE_API void update();

		template <typename System, typename... ComponentTypes>
		EntityViewList<ComponentTypes...> const& getEntityViewList(System& a_system)
		{
			auto listHolder = std::make_unique<
				SystemEntityList<System, ComponentTypes...>
			>(a_system, m_entities);
			auto& t_entityList = listHolder->m_entityList;
			m_systemEntityLists.emplace_back(std::move(listHolder));
			return t_entityList;
		}

	private:
		// Attributes
		EntityMap m_entities;
		std::deque<std::unique_ptr<ASystemEntityList>> m_systemEntityLists;

		std::vector<std::unique_ptr<Entity>> m_frameSpawns;
		SystemSpawnManager m_systemSpawnManager;

		std::vector<EntityId> m_frameUnspawns;
		SystemUnspawnManager m_systemUnspawnManager;

		// Methods
		void processSpawns();

		void processUnspawns();
	};
}
