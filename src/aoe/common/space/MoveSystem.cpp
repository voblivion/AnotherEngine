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
					TransformComponent, VelocityComponent const>() }
		{}

		void MoveSystem::update() const
		{
			for (auto const& entity : m_entities)
			{
				auto& transform = entity.getComponent<TransformComponent>();
				auto const& move = entity.getComponent<VelocityComponent>();
				transform.m_position += move.m_linear
					* m_worldTime.m_elapsedTime;
				transform.m_orientation += move.m_angular
					* m_worldTime.m_elapsedTime;
			}
		}
	}
}