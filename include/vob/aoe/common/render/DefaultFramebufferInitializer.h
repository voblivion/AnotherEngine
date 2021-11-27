#pragma once

#include <vob/aoe/ecs/WorldDataProvider.h>

#include <vob/aoe/common/window/WorldWindowComponent.h>

namespace vob::aoe::common
{
	template <
		bool t_depth = false
		, std::uint8_t t_r = 0
		, std::uint8_t t_g = 0
		, std::uint8_t t_b = 0
		, std::uint8_t t_a = 255
	>
	class DefaultFramebufferInitializer
	{
	public:
		// Constructor
		explicit DefaultFramebufferInitializer(aoecs::WorldDataProvider& a_wdp)
			: m_worldWindowComponent{ a_wdp.getWorldComponentRef<WorldWindowComponent>() }
		{}

		// Operators
		bool run() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_worldWindowComponent.getWindow().getDefaultFramebufferId());
			glClearColor(
				static_cast<float>(t_r) / 255
				, static_cast<float>(t_g) / 255
				, static_cast<float>(t_b) / 255
				, static_cast<float>(t_a) / 255
			);
			if constexpr (t_depth)
			{
				glEnable(GL_DEPTH_TEST);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}
			else
			{
				glDisable(GL_DEPTH_TEST);
				glClear(GL_COLOR_BUFFER_BIT);
            }

            auto const windowSize = m_worldWindowComponent.getWindow().getSize();
            glViewport(0, 0, windowSize.x, windowSize.y);
			return true;
		}

	private:
		// Attributes
		WorldWindowComponent& m_worldWindowComponent;
	};
}
