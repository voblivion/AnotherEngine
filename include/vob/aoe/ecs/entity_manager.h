#pragma once

#include <vob/aoe/ecs/entity_map.h>

#include <utility>


namespace vob::aoecs
{
	namespace detail
	{
		struct spawn_observer_base
		{
			virtual ~spawn_observer_base() = default;

			virtual void on_spawn(entity_list::entity_view a_entity) const = 0;
		};

		template <typename TObserver>
		class spawn_observer : public spawn_observer_base
		{
		public:
			explicit spawn_observer(TObserver& a_observer)
				: m_observer(a_observer)
			{}

			void on_spawn(entity_list::entity_view a_entity) const override
			{
				m_observer.get().on_spawn(a_entity);
			}

		private:
			std::reference_wrapper<TObserver> m_observer;
		};

		struct despawn_observer_base
		{
			virtual ~despawn_observer_base() = default;

			virtual void on_despawn(entity_list::entity_view a_entity) const = 0;
		};

		template <typename TObserver>
		class despawn_observer : public despawn_observer_base
		{
		public:
			explicit despawn_observer(TObserver& a_observer)
				: m_observer(a_observer)
			{}

			void on_despawn(entity_list::entity_view a_entity) const override
			{
				m_observer.get().on_despawn(a_entity);
			}

		private:
			std::reference_wrapper<TObserver> m_observer;
		};
	}
	
	class entity_manager
	{
	public:
		struct spawn_entry
		{
			entity_id m_id;
			component_set const& m_componentSet;
			entity_map::create_callback const* m_spawnCallback;
		};

		class spawner
		{
			friend class entity_manager;
		public:
			entity_id spawn(
				component_set const& a_components
				, entity_map::create_callback const* a_createCallback = nullptr)
			{
				entity_id id = 0;
				if (!m_unusedEntityIds.empty())
				{
					id = m_unusedEntityIds.back();
					m_unusedEntityIds.pop_back();
				}
				else
				{
					id = ++m_nextEntityId;
				}
				m_frameSpawns.emplace_back(id, a_components, a_createCallback);
				return id;
			}

		private:
			entity_id m_nextEntityId = 0;
			std::vector<spawn_entry> m_frameSpawns;
			std::vector<entity_id> m_unusedEntityIds;
		};

		class despawner
		{
			friend class entity_manager;
		public:
			void despawn(entity_id const a_id)
			{
				m_frameDespawns.push_back(a_id);
			}

		private:
			std::vector<entity_id> m_frameDespawns;
		};

		void update()
		{
			for (auto const& spawnEntry : m_spawner.m_frameSpawns)
			{
				auto entityView = m_entities.add_entity(
					spawnEntry.m_id, spawnEntry.m_componentSet, spawnEntry.m_spawnCallback);

				for (auto const& observer : m_spawnObservers)
				{
					observer->on_spawn(entityView);
				}
			}
			m_spawner.m_frameSpawns.clear();

			for (auto const& entityId : m_despawner.m_frameDespawns)
			{
				auto entityView = m_entities.find_entity(entityId);
				if (entityView == std::nullopt)
				{
					continue;
				}

				for (auto const& observer : m_spawnObservers)
				{
					observer->on_spawn(*entityView);
				}

				m_entities.remove_entity(entityId);
			}
			m_despawner.m_frameDespawns.clear();
		}

		explicit entity_manager(component_list_factory const& a_componentListFactory)
			: m_entities{ a_componentListFactory }
		{}

		auto& get_spawner()
		{
			return m_spawner;
		}

		auto& get_despawner()
		{
			return m_despawner;
		}

		template <typename... TComponents>
		decltype(auto) observe_entities()
		{
			return m_entities.observe<TComponents...>();
		}

		template <typename TObserver>
		void observe_spawns(TObserver& a_observer)
		{
			m_spawnObservers.push_back(
				mistd::polymorphic_ptr_util::make<detail::spawn_observer<TObserver>>(
					a_observer));
		}

		template <typename TObserver>
		void observe_despawns(TObserver& a_observer)
		{
			m_despawnObservers.push_back(
				mistd::polymorphic_ptr_util::make<detail::despawn_observer<TObserver>>(
					a_observer));
		}

	private:
		entity_map m_entities;
		spawner m_spawner;
		despawner m_despawner;

		std::vector<mistd::polymorphic_ptr<detail::spawn_observer_base>> m_spawnObservers;
		std::vector<mistd::polymorphic_ptr<detail::despawn_observer_base>> m_despawnObservers;
	};
}
