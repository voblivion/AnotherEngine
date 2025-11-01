#include <vob/aoe/debug/GhostControllerSystem.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include <chrono>
#include <numbers>


namespace vob::aoedb
{
	void GhostControllerSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_inputBindings.init(a_wdar);
		m_timeContext.init(a_wdar);
		m_ghostControllerEntities.init(a_wdar);
	}

	void GhostControllerSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const& switches = m_inputBindings.get(a_wdap).switches;
		auto const& axes = m_inputBindings.get(a_wdap).axes;
		auto const elapsedTime = std::chrono::duration<float>(m_timeContext.get(a_wdap).elapsedTime).count();

		for (auto [entity, position, rotation, ghostController] : m_ghostControllerEntities.get(a_wdap).each())
		{
			if (!switches[ghostController.enableRotationBinding].isPressed())
			{
				continue;
			}

			if (switches[ghostController.increaseSpeedBinding].wasPressed())
			{
				ghostController.moveSpeed *= 1.1f;
			}
			else if (switches[ghostController.decreaseSpeedBinding].wasPressed())
			{
				ghostController.moveSpeed = std::max(1.0f, ghostController.moveSpeed / 1.1f);
			}

			// update position
			auto moveDir = glm::vec3{ 0.0f };
			moveDir.x = -axes[ghostController.lateralMoveBinding].getValue();
			moveDir.y = axes[ghostController.verticalMoveMapping].getValue();
			moveDir.z = -axes[ghostController.longitudinalMoveBinding].getValue();
			auto const move = moveDir * elapsedTime * ghostController.moveSpeed;
			position += rotation * move;

			// update rotation
			float yaw, pitch, roll;
			auto const transform = aoest::combine(position, rotation);
			glm::extractEulerAngleYXZ(transform, yaw, pitch, roll);
			roll = 0.0f;
			yaw -= axes[ghostController.yawBinding].getValue() * elapsedTime;
			pitch -= axes[ghostController.pitchBinding].getValue() * elapsedTime;
			if (glm::epsilonNotEqual(roll, 0.0f, 4.0f * std::numeric_limits<float>::epsilon()))
			{
				yaw += std::numbers::pi_v<float> *(pitch > 0.0f ? 1.0f : -1.0f);
			}
			rotation = glm::quat_cast(glm::eulerAngleYXZ(yaw, pitch, roll));
		}
	}
}
