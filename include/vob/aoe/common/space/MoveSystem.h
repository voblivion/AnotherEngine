#pragma once

#include <vob/aoe/ecs/WorldDataProvider.h>
#include <vob/aoe/api.h>
#include <vob/aoe/common/space/VelocityComponent.h>
#include <vob/aoe/common/space/TransformComponent.h>
#include <vob/aoe/common/time/WorldTimeComponent.h>
#include <glm/gtc/quaternion.hpp>

namespace vob::aoe::common
{
	class MoveSystem final
	{
	public:
		using Components = aoecs::ComponentTypeList<
			TransformComponent
			, VelocityComponent const
		>;

		explicit MoveSystem(aoecs::WorldDataProvider& a_worldDataProvider)
			: m_worldTimeComponent{ *a_worldDataProvider.getWorldComponent<WorldTimeComponent>() }
			, m_entities{ a_worldDataProvider.getEntityViewList(*this, Components{}) }
		{}

		void update() const
		{
			for (auto const& entity : m_entities)
			{
				/*auto& transform = entity.getComponent<TransformComponent>();
				auto const& move = entity.getComponent<VelocityComponent>();
				glm::translate(transform.m_matrix, move.m_linear * m_worldTime.m_elapsedTime.value);

				glm::quat angularMove;
				glm::slerp(angularMove, move.m_angular, m_worldTime.m_elapsedTime.value);
				transform.m_matrix *= glm::mat4{ angularMove };*/
			}
		}

	private:
		WorldTimeComponent& m_worldTimeComponent;
		aoecs::EntityViewList<TransformComponent, VelocityComponent const> const& m_entities;
	};
}
