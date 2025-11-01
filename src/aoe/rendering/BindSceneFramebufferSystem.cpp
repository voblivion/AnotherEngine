#include <vob/aoe/rendering/BindSceneFramebufferSystem.h>

#include <vob/aoe/rendering/Color.h>

#include <gl/glew.h>

namespace vob::aoegl
{
	void BindSceneFramebufferSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_sceneTextureContext.init(a_wdar);
	}

	void BindSceneFramebufferSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_sceneTextureContext.get(a_wdap).texture.framebuffer);
		glClearColor(k_blueprint.r, k_blueprint.g, k_blueprint.b, k_blueprint.a);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}
