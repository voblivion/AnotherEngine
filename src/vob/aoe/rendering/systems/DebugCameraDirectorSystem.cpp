#include "vob/aoe/rendering/systems/DebugCameraDirectorSystem.h"

#include "vob/aoe/debug/DebugNameUtils.h"
#include "vob/aoe/debug/ImGuiUtils.h"

#include "imgui.h"


namespace vob::aoegl
{
	void DebugCameraDirectorSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		// TODO
	}

	void DebugCameraDirectorSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& cameraDirectorCtx = m_cameraDirectorCtx.get(a_wdap);
		auto const& debugCameraDirectorCtx = m_debugCameraDirectorCtx.get(a_wdap);
		auto cameraEntities = m_cameraEntities.get(a_wdap);

		for (auto const eventId : m_gameInputCtx.get(a_wdap).getEvents())
		{
			if (eventId == debugCameraDirectorCtx.prevCameraInputEventId)
			{
				auto it = cameraEntities.begin();
				if (it != cameraEntities.end())
				{
					while (it + 1 != cameraEntities.end() && *(it + 1) != cameraDirectorCtx.activeCameraEntity)
					{
						++it;
					}

					cameraDirectorCtx.activeCameraEntity = *it;
				}
			}
			else if (eventId == debugCameraDirectorCtx.nextCameraInputEventId)
			{
				auto it = cameraEntities.find(cameraDirectorCtx.activeCameraEntity);
				if (it != cameraEntities.end())
				{
					cameraDirectorCtx.activeCameraEntity = *((it + 1 != cameraEntities.end() ? it + 1 : cameraEntities.begin()));
				}
				else
				{
					cameraDirectorCtx.activeCameraEntity = *cameraEntities.begin();
				}
			}
			else if (eventId == debugCameraDirectorCtx.quitInputEventId)
			{
				m_gameController.get(a_wdap).requestStop();
			}
		}

		if (ImGui::Begin("Camera Director"))
		{
			auto debugNameEntities = m_debugNameEntities.get(a_wdap);
			aoedb::ImGuiEntityCombo("Camera", &cameraDirectorCtx.activeCameraEntity, cameraEntities, debugNameEntities);
			aoedb::ImGuiEntityCombo("Debug Camera", &cameraDirectorCtx.debugCameraEntity, cameraEntities, debugNameEntities);
		}
		ImGui::End();
	}
}
