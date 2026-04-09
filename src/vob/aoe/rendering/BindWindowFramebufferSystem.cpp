#include <vob/aoe/rendering/BindWindowFramebufferSystem.h>

#include <vob/aoe/rendering/Color.h>

#include <GL/glew.h>


namespace vob::aoegl
{
	void BindWindowFramebufferSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_windowContext.init(a_wdar);
	}

	void BindWindowFramebufferSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		glBindFramebuffer(
			GL_FRAMEBUFFER, m_windowContext.get(a_wdap).window.get().getDefaultFramebufferId());

		glClearColor(k_black.r, k_black.g, k_black.b, k_black.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}
