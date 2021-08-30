#pragma once

#include <optional>

#include <vob/aoe/common/render/OpenGl.h>
#include <glm/glm.hpp>

#include <vob/aoe/api.h>

namespace vob::aoe::common
{
	class VOB_AOE_API RenderTexture
	{
	public:
		// Constructors / Destructor
		explicit RenderTexture(glm::ivec2 const a_size, std::size_t a_multiSampling = 1);

		// Methods
		auto getSize() const
		{
			return m_size;
		}
		auto const& getFramebufferId() const
		{
			return m_state.m_framebufferId;
		}
		auto const& getTextureId() const
		{
			return m_state.m_textureId;
		}
		auto getMultiSampling() const
		{
			return m_multiSampling;
		}
		bool isReady() const
		{
			return m_state.m_isReady;
		}
		void create() const;
		void destroy() const;

	private:
		// Types
		struct State
		{
			bool m_isReady = false;
			GraphicObjectId m_textureId;
			GraphicObjectId m_framebufferId;
			GraphicObjectId m_renderbufferId;
		};

		// Attributes
		mutable State m_state = {};
		glm::ivec2 const m_size;
		std::size_t m_multiSampling;
	};
}
