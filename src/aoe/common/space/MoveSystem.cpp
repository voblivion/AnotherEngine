#include <aoe/common/space/MoveSystem.h>

#include <iostream>

namespace aoe
{
	namespace common
	{
		// Public
		MoveSystem::MoveSystem(ecs::WorldDataProvider& a_worldDataProvider)
			: m_worldTime{ *a_worldDataProvider.getWorldComponent<
				TimeComponent>() }
			, m_entities{ a_worldDataProvider.getEntityList<
					TransformComponent, MoveComponent const>() }
		{}

		void MoveSystem::update() const
		{
			for (auto const& entity : m_entities)
			{
				auto& transform = entity.getComponent<TransformComponent>();
				auto const& move = entity.getComponent<MoveComponent>();
				transform.m_position += move.m_direction
					* m_worldTime.m_elapsedTime;
				std::cout << transform.m_position.x << std::endl;
			}
		}
	}
}