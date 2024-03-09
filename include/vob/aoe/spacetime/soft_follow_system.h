#pragma once

#include <vob/aoe/spacetime/soft_follow.h>
#include <vob/aoe/spacetime/transform.h>
#include <vob/aoe/spacetime/time_world_component.h>
#include <vob/aoe/engine/world_data_provider.h>
#include <vob/aoe/rendering/world_components/debug_mesh_world_component.h>


namespace vob::aoest
{
	class soft_follow_system
	{
	public:
		explicit soft_follow_system(aoeng::world_data_provider& a_wdp)
			: m_simulationTimeWorldComponent{ a_wdp }
			, m_debugMeshWorldComponent{ a_wdp }
			, m_softFollowableEntities{ a_wdp }
			, m_softFollowingEntities{ a_wdp }
		{}

		void update() const
		{
			auto const dt = m_simulationTimeWorldComponent->m_elapsedTime.get_value();

			auto const softFollowingEntities = m_softFollowingEntities.get();
			auto const softFollowableEntities = m_softFollowableEntities.get();
			for (auto [followingEntity, position, rotation, softFollow] : softFollowingEntities.each())
			{
				if (!softFollowableEntities.contains(softFollow.m_target))
				{
					continue;
				}
				
				auto [followedPosition, followedRotation] =
					softFollowableEntities.get(softFollow.m_target);

				auto const followedTransform = aoest::combine(followedPosition, followedRotation);

				auto const desiredPosition =
					glm::vec3{ followedTransform * glm::vec4{softFollow.m_posTarget, 1.0f} };
				if (position != desiredPosition)
				{
					auto const toDesiredPosition = desiredPosition - position;
					auto const desiredPositionDist = glm::length(toDesiredPosition);
					auto const desiredDir = toDesiredPosition / desiredPositionDist;
					if (desiredPositionDist > softFollow.m_maxDist)
					{
						position = desiredPosition - desiredDir * softFollow.m_maxDist;
					}
					else
					{
						position += toDesiredPosition * softFollow.m_smoothing * dt;
					}
				}

				m_debugMeshWorldComponent->add_line(position, desiredPosition, aoegl::k_black);
				auto const desiredAimPosition =
					glm::vec3{ followedTransform * glm::vec4{softFollow.m_aimTarget, 1.0f} };

				auto const desiredAimDir = glm::normalize(desiredAimPosition - position);
				rotation = glm::quatLookAt(desiredAimDir, glm::vec3{ 0.0f, 1.0f, 0.0f });
				/*
				rotation = glm::rotate(glm::quat{})
				rotation = glm::quat{
					glm::lookAt(position, desiredAimPosition, glm::vec3{ 0.0f, 1.0f, 0.0f }) };*/
			}
		}

	private:
		aoeng::world_component_ref<aoest::simulation_time_world_component> m_simulationTimeWorldComponent;
		aoeng::world_component_ref<aoegl::debug_mesh_world_component> m_debugMeshWorldComponent;
		aoeng::registry_view_ref<aoest::position, aoest::rotation> m_softFollowableEntities;
		aoeng::registry_view_ref<aoest::position, aoest::rotation, soft_follow> m_softFollowingEntities;
	};
}
