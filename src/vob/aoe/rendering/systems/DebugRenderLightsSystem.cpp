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
			if (lightCmp.type == LightComponent::Type::Spot)
			{
				debugMeshContext.addLine(
					positionCmp.value,
					positionCmp.value + rotationCmp.value * (lightCmp.radius * glm::vec3{ 0.0f, 0.0f, -1.0f }),
					aoegl::k_gray);
			}
		}
	}
}
