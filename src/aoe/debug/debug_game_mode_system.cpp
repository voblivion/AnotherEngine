#include <vob/aoe/debug/debug_game_mode_system.h>

#include <vob/aoe/debug/controlled_tag.h>


namespace vob::aoedb
{
	template <typename TRegistryView>
	decltype(auto) find_next_entity(TRegistryView a_entities, aoeng::entity a_prevEntity)
	{
		auto hasFoundPrevEntity = a_prevEntity == entt::null;
		for (auto entity : a_entities)
		{
			if (hasFoundPrevEntity)
			{
				return entity;
			}

			if (a_prevEntity == entity)
			{
				hasFoundPrevEntity = true;
			}
		}

		return a_entities.front();
	}

	debug_game_mode_system::debug_game_mode_system(aoeng::world_data_provider& a_wdp)
		: m_bindings{ a_wdp }
		, m_directorWorldComponent{ a_wdp }
		, m_debugGameModeWorldComponent{ a_wdp }
		, m_queryRef{ a_wdp }
		, m_controlledEntities{ a_wdp }
		, m_controllableEntities{ a_wdp }
		, m_cameraEntities{ a_wdp }
	{}

	void debug_game_mode_system::update() const
	{
		auto const& switches = m_bindings->switches;
		if (switches.find(m_debugGameModeWorldComponent->m_switchActiveController)->was_pressed())
		{
			auto nextControlledEntity = find_next_entity(m_controllableEntities.get(), m_controlledEntities.get().front());
			if (nextControlledEntity != entt::null)
			{
				// TODO: maybe using a query ref is not necessary and this pool can be modified
				m_queryRef.add(
					[this, nextControlledEntity](aoeng::registry& a_registry) {
						a_registry.clear<controlled_tag>();
						a_registry.emplace<controlled_tag>(nextControlledEntity);
					});
			}
		}
		if (switches.find(m_debugGameModeWorldComponent->m_switchActiveCamera)->was_pressed())
		{
			auto nextActiveCamera = find_next_entity(m_cameraEntities.get(), m_directorWorldComponent->m_activeCamera);
			if (nextActiveCamera != entt::null)
			{
				m_directorWorldComponent->m_activeCamera = nextActiveCamera;
			}
		}
	}
}