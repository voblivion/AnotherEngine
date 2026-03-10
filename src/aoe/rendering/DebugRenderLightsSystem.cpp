#include <vob/aoe/rendering/DebugRenderLightsSystem.h>

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

		for (auto [entity, position, rotation, lightCmp] : m_lightEntities.get(a_wdap).each())
		{
			debugMeshContext.addSphere(position, lightCmp.radius, aoegl::Rgba{ lightCmp.color, 1.0 });
		}
	}
}
