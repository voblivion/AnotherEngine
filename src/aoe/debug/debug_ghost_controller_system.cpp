#include <vob/aoe/debug/debug_ghost_controller_system.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>


namespace vob::aoedb
{
	debug_ghost_controller_system::debug_ghost_controller_system(aoeng::world_data_provider& a_wdp)
		: m_bindings{ a_wdp }
		, m_presentationTimeWorldComponent{ a_wdp }
		, m_ghostControllerEntities{ a_wdp }
	{}

	void debug_ghost_controller_system::update() const
	{
		auto const switches = m_bindings->switches;
		auto const axes = m_bindings->axes;
		auto const dt = m_presentationTimeWorldComponent->m_elapsedTime.get_value();

		auto const ghostControllerEntities = m_ghostControllerEntities.get();
		for (auto [entity, position, rotation, ghostController] : ghostControllerEntities.each())
		{
			// TODO: could be after position update, but conflicting with other controls for now
			if (!switches.find(ghostController.m_enableViewMapping)->is_pressed())
			{
				continue;
			}

			if (switches.find(ghostController.m_increaseSpeedMapping)->was_pressed())
			{
				ghostController.m_moveSpeed *= 1.1f;
			}
			else if (switches.find(ghostController.m_decreaseSpeedMapping)->was_pressed())
			{
				ghostController.m_moveSpeed /= 1.1f;
				if (ghostController.m_moveSpeed < 1.0f)
				{
					ghostController.m_moveSpeed = 1.0f;
				}
			}

			auto moveDir = glm::vec3{ 0.0f };

			// update position
			moveDir.x = -axes.find(ghostController.m_lateralMoveMapping)->get_value();
			moveDir.y = axes.find(ghostController.m_verticalMoveMapping)->get_value();
			moveDir.z = -axes.find(ghostController.m_longitudinalMoveMapping)->get_value();
			auto const move = moveDir * dt * ghostController.m_moveSpeed;
			position += rotation * move;

			// update rotation
			float yaw, pitch, roll;
			auto const transform = aoest::combine(position, rotation);
			glm::extractEulerAngleYXZ(transform, yaw, pitch, roll);
			roll = 0.0f;
			yaw -= axes.find(ghostController.m_yawMapping)->get_value() * dt;
			pitch -= axes.find(ghostController.m_pitchMapping)->get_value() * dt;
			if (glm::epsilonNotEqual(roll, 0.0f, 4 * glm::epsilon<float>()))
			{
				yaw += glm::pi<float>() * (pitch > 0.0f ? 1.0f : -1.0f);
			}
			rotation = glm::quat_cast(glm::eulerAngleYXZ(yaw, pitch, roll));
		}
	}
}
