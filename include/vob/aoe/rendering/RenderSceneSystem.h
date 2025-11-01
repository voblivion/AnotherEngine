#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/PostProcessRenderContext.h>
#include <vob/aoe/rendering/SceneTextureContext.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>
#include <vob/aoe/window/WindowContext.h>


namespace vob::aoegl
{
	class VOB_AOE_API RenderSceneSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<PostProcessRenderContext> m_postProcessRenderContext;
		aoeng::EcsWorldContextRef<SceneTextureContext> m_sceneTextureContext;
		aoeng::EcsWorldContextRef<aoewi::WindowContext> m_windowContext;
	};
}
