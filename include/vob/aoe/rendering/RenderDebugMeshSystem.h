#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/CameraDirectorContext.h>
#include <vob/aoe/rendering/DebugRenderContext.h>
#include <vob/aoe/rendering/DebugMeshContext.h>
#include <vob/aoe/rendering/CameraComponent.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/spacetime/Transform.h>
#include <vob/aoe/window/WindowContext.h>


namespace vob::aoegl
{
	class VOB_AOE_API RenderDebugMeshSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoewi::WindowContext> m_windowContext;
		aoeng::EcsWorldContextRef<CameraDirectorContext> m_cameraDirectorContext;
		aoeng::EcsWorldContextRef<DebugRenderContext> m_debugRenderContext;
		aoeng::EcsWorldContextRef<DebugMeshContext> m_debugMeshContext;

		aoeng::EcsWorldViewRef<aoest::Position const, aoest::Rotation const, CameraComponent const> m_cameraEntities;
	};
}
