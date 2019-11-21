#pragma once

#include <vob/aoe/core/ecs/WorldDataProvider.h>
#include <vob/aoe/api.h>
#include <vob/aoe/common/space/VelocityComponent.h>
#include <vob/aoe/common/space/TransformComponent.h>
#include <vob/aoe/common/time/TimeComponent.h>
#include <glm/gtc/quaternion.hpp>

namespace vob::aoe::common
{
	class MoveSystem final
	{
	public:
		using Components = ecs::ComponentTypeList<
			TransformComponent
			, VelocityComponent const
		>;

		explicit MoveSystem(ecs::WorldDataProvider& a_worldDataProvider)
			: m_worldTime{ *a_worldDataProvider.getWorldComponent<
				TimeComponent>() }
			, m_entities{ a_worldDataProvider.getEntityList(*this, Components{}) }
		{}

		void update() const
		{
			for (auto const& entity : m_entities)
			{
				auto& transform = entity.getComponent<TransformComponent>();
				auto const& move = entity.getComponent<VelocityComponent>();
				transform.m_position += move.m_linear
					* m_worldTime.m_elapsedTime.value;

				glm::slerp(transform.m_rotation
					, move.m_angular * transform.m_rotation
					, m_worldTime.m_elapsedTime.value);
			}
		}

	private:
		TimeComponent& m_worldTime;
		ecs::EntityList<TransformComponent, VelocityComponent const> const& m_entities;
	};
}
