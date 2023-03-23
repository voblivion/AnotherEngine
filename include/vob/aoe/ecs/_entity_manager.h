#pragma once

#include <deque>
#include <unordered_map>
#include <algorithm>
#include <optional>

#include <vob/aoe/api.h>
#include <vob/aoe/ecs/_entity.h>
#include <vob/aoe/ecs/_entity_id.h>
#include <vob/aoe/ecs/_entity_view.h>
#include <vob/aoe/ecs/_spawn_manager.h>
#include <vob/aoe/ecs/_unspawn_manager.h>

#include <vob/aoe/core/type/ADynamicType.h>

namespace vob::_aoecs
{
	using EntityMap = std::unordered_map<entity_id, std::unique_ptr<entity>, entity_id_hash>;

	template <typename... ComponentTypes>
	struct entity_view_list
	{
		// Aliases
		using entity_view_type = entity_view<ComponentTypes...>;

		// Attributes
		std::deque<entity_view_type> m_list;
		std::unordered_map<entity_id, std::size_t, entity_id_hash> m_entityIndexes;

		// Methods
		void add(entity& a_entity)
		{
			m_entityIndexes.emplace(a_entity.get_id(), m_list.size());
			m_list.emplace_back(a_entity);
		}

		void remove(entity_id const a_id)
		{
			// todo : pass it ?
			auto t_it = m_entityIndexes.find(a_id);
			if (t_it != m_entityIndexes.end())
			{
				auto t_listIt = m_list.begin();
				t_listIt += t_it->second;

				auto t_endListIt = m_list.end();
				--t_endListIt;
				auto t_endIt = m_entityIndexes.find(t_endListIt->get_id());

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

		entity_view_type const* find(entity_id const a_id) const
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

	namespace detail
	{
		template <typename TSystem>
		class has_on_entity_added
		{
		private:
			template <typename S>
			static auto test(int)
				-> decltype(std::declval<S>().on_entity_added(std::declval<entity&>()), std::true_type{})
			{
				return std::true_type{};
			}

			template <typename>
			static std::false_type test(...) { return std::false_type{}; };

		public:
			static constexpr bool value = std::is_same_v<decltype(test<TSystem>(0)), std::true_type>;
		};

		template <typename TSystem>
		constexpr auto has_on_entity_added_v = has_on_entity_added<TSystem>::value;

		template <typename TSystem>
		requires has_on_entity_added_v<TSystem>
		void call_on_entity_added(TSystem& a_system, entity& a_entity)
		{
			a_system.on_entity_added(a_entity);
		}

		template <typename TSystem>
		requires (!has_on_entity_added_v<TSystem>)
		void call_on_entity_added(TSystem& a_system, entity& a_entity)
		{

		}

		template <typename TSystem>
		class has_on_entity_removed
		{
		private:
			template <typename S>
			static auto test(int)
				-> decltype(std::declval<S>().on_entity_removed(std::declval<entity&>()), std::true_type{})
			{
				return std::true_type{};
			}

			template <typename>
			static std::false_type test(...) { return std::false_type{}; };

		public:
			static constexpr bool value = std::is_same_v<decltype(test<TSystem>(0)), std::true_type>;
		};

		template <typename TSystem>
		constexpr auto has_on_entity_removed_v = has_on_entity_removed<TSystem>::value;

		template <typename TSystem>
		requires has_on_entity_removed_v<TSystem>
		void call_on_entity_removed(TSystem& a_system, entity& a_entity)
		{
			a_system.on_entity_removed(a_entity);
		}

		template <typename TSystem>
		requires (!has_on_entity_removed_v<TSystem>)
		void call_on_entity_removed(TSystem& a_system, entity const& a_entity)
		{

		}

		template <typename... ComponentTypes>
		struct entity_components_tester;

		template <>
		struct entity_components_tester<>
		{
			static bool test(entity const& a_entity)
			{
				return true;
			}
		};

		template <typename ComponentType, typename... ComponentTypes>
		struct entity_components_tester<ComponentType, ComponentTypes...>
		{
			static bool test(entity const& a_entity)
			{
				return a_entity.has_component<std::remove_const_t<ComponentType>>()
					&& entity_components_tester<ComponentTypes...>::test(a_entity);
			}
		};

		template <typename ComponentType, typename... ComponentTypes>
		struct entity_components_tester<ComponentType*, ComponentTypes...>
		{
			static bool test(entity const& a_entity)
			{
				return entity_components_tester<ComponentTypes...>::test(a_entity);
			}
		};

		struct basic_entity_view_list_holder
			: public aoe::type::ADynamicType
		{
			// Methods
			virtual void on_entity_added(entity& a_entity) = 0;
			virtual void on_entity_removed(entity& a_entity) = 0;
		};

		template <typename TSystem, typename... ComponentTypes>
		struct entity_view_list_holder final
			: public basic_entity_view_list_holder
		{
			using entity_view_type = entity_view<ComponentTypes...>;

			// Attributes
			TSystem& m_system;
			entity_view_list<ComponentTypes...> m_entityList;

			// Constructor
			explicit entity_view_list_holder(
				TSystem& a_system
				, EntityMap const& a_entities
			)
				: m_system{ a_system }
			{
				for (auto& t_pair : a_entities)
				{
					on_entity_added(*t_pair.second);
				}
			}

			// Methods
			virtual void on_entity_added(entity& a_entity) override
			{
				if (detail::entity_components_tester<ComponentTypes...>::test(a_entity))
				{
					m_entityList.add(a_entity);
					detail::call_on_entity_added(m_system, a_entity);
				}
			}

			virtual void on_entity_removed(entity& a_entity) override
			{
				auto t_entity = m_entityList.find(a_entity.get_id());
				if (t_entity != nullptr)
				{
					detail::call_on_entity_removed(m_system, a_entity);
					m_entityList.remove(a_entity.get_id());
				}
			}
		};

	}

	class entity_manager
	{
	public:
		// Constructors
		VOB_AOE_API explicit entity_manager();

		VOB_AOE_API ~entity_manager();

		//Methods
		VOB_AOE_API spawn_manager& get_spawn_manager();

		VOB_AOE_API unspawn_manager& get_unspawn_manager();

		VOB_AOE_API void update();

		template <typename System, typename... ComponentTypes>
		entity_view_list<ComponentTypes...> const& get_entity_view_list(System& a_system)
		{
			auto listHolder = std::make_unique<detail::entity_view_list_holder<System, ComponentTypes...>>(
				a_system, m_entities);
			auto& t_entityList = listHolder->m_entityList;
			m_systemEntityViewLists.emplace_back(std::move(listHolder));
			return t_entityList;
		}

	private:
		// Attributes
		EntityMap m_entities;
		std::deque<std::unique_ptr<detail::basic_entity_view_list_holder>> m_systemEntityViewLists;

		std::vector<std::unique_ptr<entity>> m_frameSpawns;
		std::vector<entity_id> m_unusedEntityIds;
		spawn_manager m_spawnManager;

		std::vector<entity_id> m_frameUnspawns;
		unspawn_manager m_unspawnManager;

		// Methods
		void process_spawns();

		void process_unspawns();
	};
}
