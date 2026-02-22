#include <vob/aoe/physics/DebugRenderCollidersSystem.h>


namespace vob::aoeph
{
	void DebugRenderCollidersSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_debugMeshContext.init(a_wdar);
		m_staticColliderEntities.init(a_wdar);
	}

	void DebugRenderCollidersSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		return;
		auto& debugMeshContext = m_debugMeshContext.get(a_wdap);

		for (auto [entity, position, rotation, staticCollider] : m_staticColliderEntities.get(a_wdap).each())
		{
			for (auto const& staticPart : staticCollider.parts)
			{
				for (auto const& staticTriangle : staticPart.triangles)
				{
					debugMeshContext.addTriangle(
						position + rotation * staticTriangle.p0,
						position + rotation * staticTriangle.p1,
						position + rotation * staticTriangle.p2,
						aoegl::k_gray);
				}
			}
		}

		for (auto [entity, position, rotation, carCollider] : m_carColliderEntities.get(a_wdap).each())
		{
			for (auto const& chassisPart : carCollider.chassisParts)
			{
				debugMeshContext.addEllipsoid(position + rotation * chassisPart.position, rotation * chassisPart.rotation, chassisPart.radiuses, aoegl::k_white);
			}

			for (auto const& wheel : carCollider.wheels)
			{
				auto const wheelPositionLocal = wheel.suspensionAttachPosition + wheel.rotation * glm::vec3{ 0.0f, -wheel.suspensionLength, 0.0f };
				debugMeshContext.addEllipsoid(position + rotation * wheelPositionLocal, rotation * wheel.rotation, wheel.radiuses, aoegl::k_green);
			}
		}
	}
}
