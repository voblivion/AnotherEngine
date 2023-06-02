#include <vob/aoe/rendering/systems/bind_scene_framebuffer_system.h>

#include <vob/aoe/rendering/color.h>


namespace vob::aoegl
{
	bind_scene_framebuffer_system::bind_scene_framebuffer_system(aoeng::world_data_provider& a_wdp)
		: m_sceneTextureWorldComponent{ a_wdp }
	{}

	void bind_scene_framebuffer_system::update() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_sceneTextureWorldComponent->m_sceneTexture.m_framebuffer);
		glClearColor(k_dark_blue.r, k_dark_blue.g, k_dark_blue.b, k_dark_blue.a);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}
