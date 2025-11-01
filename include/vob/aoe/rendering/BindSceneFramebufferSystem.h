#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/rendering/SceneTextureContext.h>

#include <vob/aoe/engine/EcsWorldDataAccess.h>


namespace vob::aoegl
{
	class VOB_AOE_API BindSceneFramebufferSystem
	{
	public:
		void init(aoeng::EcsWorldDataAccessRegistrar& a_wdar);
		void execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const;

	private:
		aoeng::EcsWorldContextRef<aoegl::SceneTextureContext> m_sceneTextureContext;
	};
}
