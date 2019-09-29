#pragma once

#include <deque>
#include <unordered_map>
#include <algorithm>

#include <aoe/core/Export.h>
#include <aoe/core/ecs/Entity.h>
#include <aoe/core/ecs/EntityId.h>
#include <aoe/core/ecs/SystemEntity.h>
#include <aoe/core/ecs/SystemSpawnManager.h>
#include <aoe/core/ecs/SystemUnspawnManager.h>
#include <aoe/core/standard/Memory.h>

namespace aoe
{
	namespace ecs
	{
		using EntityMap = std::pmr::unordered_map<EntityId, sta::PolymorphicPtr<Entity>>;

		namespace detail
		{
			template <typename SystemType>
			class HasOnEntityAdded
			{
			private:
				template <typename S>
				static auto test(int)
					-> decltype(std::declval<S>().onEntityAdded(std::declval<Entity const&>()), std::true_type{})
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
			void callOnEntityAdded(SystemType& a_system, Entity const& a_entity)
			{
				a_system.onEntityAdded(a_entity);
			}

			template <typename SystemType
				, std::enable_if_t<!hasOnEntityAddedV<SystemType>>* = nullptr>
			void callOnEntityAdded(SystemType& a_system, Entity const& a_entity)
			{

			}

			template <typename SystemType>
			class HasOnEntityRemoved
			{
			private:
				template <typename S>
				static auto test(int)
					-> decltype(std::declval<S>().onEntityRemoved(std::declval<EntityId const>()), std::true_type{})
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
			void callOnEntityRemoved(SystemType& a_system, EntityId const a_id)
			{
				a_system.onEntityRemoved(a_id);
			}

			template <typename SystemType
				, std::enable_if_t<!hasOnEntityRemovedV<SystemType>>* = nullptr>
			void callOnEntityRemoved(SystemType& a_system, EntityId const a_id)
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

		struct ASystemEntityList
			: public sta::ADynamicType
		{
			// Methods
			virtual void onEntityAdded(Entity const& a_entity) = 0;
			virtual void onEntityRemoved(EntityId const a_id) = 0;
		};

		template <typename SystemType, typename... ComponentTypes>
		struct SystemEntityList final
			: public ASystemEntityList
		{
			// Attributes
			SystemType& m_system;
			std::pmr::deque<SystemEntity<ComponentTypes...>> m_list;
			std::pmr::unordered_map<EntityId, std::size_t> m_entityIndexes;

			// Constructor
			explicit SystemEntityList(SystemType& a_system
				, EntityMap const& a_entities)
				: m_system{ a_system }
				, m_list{ a_entities.get_allocator() }
				, m_entityIndexes{ a_entities.get_allocator() }
			{
				for (auto& t_pair : a_entities)
				{
					SystemEntityList::onEntityAdded(*t_pair.second);
				}
			}

			// Methods

			virtual void onEntityAdded(Entity const& a_entity) override
			{
				if (detail::SystemEntityTester<ComponentTypes...>::test(a_entity))
				{
					m_entityIndexes.emplace(a_entity.getId(), m_list.size());
					m_list.emplace_back(a_entity);
					detail::callOnEntityAdded(m_system, a_entity);
				}
			}

			virtual void onEntityRemoved(EntityId const a_id) override
			{
				auto t_it = m_entityIndexes.find(a_id);
				if (t_it != m_entityIndexes.end())
				{
					auto t_listIt = m_list.begin();
					t_listIt += t_it->second;
					detail::callOnEntityRemoved(m_system, a_id);

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

			SystemEntity<ComponentTypes...> const* find(
				EntityId const a_id) const
			{
				auto const t_it = m_entityIndexes.find(a_id);
				if (t_it != m_entityIndexes.end())
				{
					return &m_list[t_it->second];
				}
				return nullptr;
			}
		};

		class EntityManager
		{
		public:
			// Constructors
			AOE_CORE_API explicit EntityManager(
				sta::Allocator<std::byte> const& a_allocator);

			//Methods
			AOE_CORE_API SystemSpawnManager& getSystemSpawnManager();

			AOE_CORE_API SystemUnspawnManager& getSystemUnspawnManager();

			AOE_CORE_API void update();

			template <typename System, typename... ComponentTypes>
			SystemEntityList<System, ComponentTypes...> const& getEntityList(
				System& a_system)
			{
				auto t_listHolder = sta::allocatePolymorphicWith<SystemEntityList<
					System, ComponentTypes...>>(
						m_systemEntityLists.get_allocator().resource()
						, a_system, m_entities);
				auto& t_list = *t_listHolder;
				m_systemEntityLists.emplace_back(std::move(t_listHolder));
				return t_list;
			}

		private:
			// Attributes
			EntityMap m_entities;
			std::pmr::deque<sta::PolymorphicPtr<
				ASystemEntityList>> m_systemEntityLists;

			std::pmr::vector<sta::PolymorphicPtr<Entity>> m_frameSpawns;
			SystemSpawnManager m_systemSpawnManager;

			std::pmr::vector<EntityId> m_frameUnspawns;
			SystemUnspawnManager m_systemUnspawnManager;

			// Methods
			void processSpawns();

			void processUnspawns();
		};
	}
}
