#include <vob/aoe/rendering/systems/bind_window_framebuffer_system.h>

#include <vob/aoe/rendering/_color.h>

#include <GL/glew.h>


namespace vob::aoegl
{
	bind_window_framebuffer_system::bind_window_framebuffer_system(
		aoeng::world_data_provider& a_wdp)
		: m_windowWorldComponent{ a_wdp }
	{}

	void bind_window_framebuffer_system::update() const
	{
		glBindFramebuffer(
			GL_FRAMEBUFFER, m_windowWorldComponent->m_window.get().get_default_framebuffer_id());
		glClearColor(_k_black.r, _k_black.g, _k_black.b, _k_black.a);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}
