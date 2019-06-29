#pragma once

#include <aoe/core/ecs/System.h>
#include <aoe/core/ecs/WorldDataProvider.h>
#include <aoe/common/Export.h>
#include <aoe/common/space/VelocityComponent.h>
#include <aoe/common/space/TransformComponent.h>
#include <aoe/common/time/TimeComponent.h>
#include <glm/gtc/quaternion.hpp>

namespace aoe
{
	namespace common
	{
		class MoveSystem final
		{
		public:
			explicit MoveSystem(ecs::WorldDataProvider& a_worldDataProvider)
				: m_worldTime{ *a_worldDataProvider.getWorldComponent<
					TimeComponent>() }
				, m_entities{ a_worldDataProvider.getEntityList<
						TransformComponent, VelocityComponent const>() }
			{}

			void update() const
			{
				for (auto const& entity : m_entities)
				{
					auto& transform = entity.getComponent<TransformComponent>();
					auto const& move = entity.getComponent<VelocityComponent>();
					transform.m_position += move.m_linear
						* m_worldTime.m_elapsedTime;

					glm::slerp(transform.m_rotation
						, move.m_angular * transform.m_rotation, m_worldTime.m_elapsedTime);
				}
			}

		private:
			TimeComponent& m_worldTime;
			ecs::SystemEntityList<TransformComponent
				, VelocityComponent const> const& m_entities;
		};
	}
}
