#include "vob/aoe/rendering/systems/DebugRenderLightsSystem.h"

#include <imgui.h>


namespace vob::aoegl
{
	void DebugRenderLightsSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	void DebugRenderLightsSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		static bool k_lights = false;
		if (ImGui::Begin("Debug Mesh"))
		{
			ImGui::Checkbox("Lights", &k_lights);
		}
		ImGui::End();
		if (!k_lights)
		{
			return;
		}

		auto& debugMeshContext = m_debugMeshContext.get(a_wdap);

		for (auto [entity, positionCmp, rotationCmp, lightCmp] : m_lightEntities.get(a_wdap).each())
		{
			debugMeshContext.addSphere(positionCmp.value, lightCmp.radius, aoegl::Rgba{ lightCmp.color, 1.0f });
			if (lightCmp.type == LightType::Spot)
			{
				debugMeshContext.addLine(
					positionCmp.value,
					positionCmp.value + glm::dquat{ rotationCmp.value } * (static_cast<double>(lightCmp.radius) * glm::dvec3{ 0.0, 0.0, -1.0 }),
					aoegl::k_gray);
			}
		}
	}
}
