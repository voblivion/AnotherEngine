#include "vob/aoe/debug/GhostControllerSystem.h"

#include "vob/aoe/spacetime/TransformUtils.h"

#include "glm/gtx/euler_angles.hpp"

#include <chrono>
#include <numbers>


namespace vob::aoedb
{
	void GhostControllerSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_timeContext.init(a_wdar);
		m_ghostControllerEntities.init(a_wdar);
	}

	void GhostControllerSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const elapsedTime = std::chrono::duration<float>(m_timeContext.get(a_wdap).elapsedTime).count();

		auto const& gameInputCtx = m_gameInputCtx.get(a_wdap);
		auto const& gameInputEvents = gameInputCtx.getEvents();

		for (auto [entity, positionCmp, rotationCmp, ghostController] : m_ghostControllerEntities.get(a_wdap).each())
		{
			if (!(gameInputCtx.getValue(ghostController.enableRotationValueId) > 0.0f))
			{
				continue;
			}

			for (auto gameInputEventId : gameInputEvents)
			{
				if (gameInputEventId == ghostController.decreaseSpeedEventId)
				{
					ghostController.moveSpeed = std::max(1.0f, ghostController.moveSpeed / 1.1f);
				}
				else if (gameInputEventId == ghostController.increaseSpeedEventId)
				{
					ghostController.moveSpeed *= 1.1f;
				}
			}

			// update position
			auto moveDir = glm::vec3{ 0.0f };
			moveDir.x = -gameInputCtx.getValue(ghostController.lateralMoveValueId);
			moveDir.y = gameInputCtx.getValue(ghostController.verticalMoveValueId);
			moveDir.z = -gameInputCtx.getValue(ghostController.longitudinalMoveValueId);
			auto const move = moveDir * elapsedTime * ghostController.moveSpeed;
			positionCmp.value += rotationCmp.value * move;

			// update rotation
			float yaw, pitch, roll;
			auto const transform = aoest::combine(positionCmp, rotationCmp);
			glm::extractEulerAngleYXZ(transform, yaw, pitch, roll);
			roll = 0.0f;
			yaw -= gameInputCtx.getValue(ghostController.yawValueId) * elapsedTime;
			pitch -= gameInputCtx.getValue(ghostController.pitchValueId) * elapsedTime;
			if (glm::epsilonNotEqual(roll, 0.0f, 4.0f * std::numeric_limits<float>::epsilon()))
			{
				yaw += std::numbers::pi_v<float> *(pitch > 0.0f ? 1.0f : -1.0f);
			}
			rotationCmp.value = glm::quat_cast(glm::eulerAngleYXZ(yaw, pitch, roll));
		}
	}
}
