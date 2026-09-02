#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"

#include <mutex>
#include <vector>


namespace vob::aoegl
{
	class GlDeleteQueue
	{
	public:
		void pushBuffer(GraphicId const a_id);
		void pushTexture(GraphicId const a_id);
		void pushFramebuffer(GraphicId const a_id);
		void pushVertexArray(GraphicId const a_id);
		void pushProgram(GraphicId const a_id);

		void drain();

	private:
		std::mutex m_mutex;
		std::vector<GraphicId> m_buffers;
		std::vector<GraphicId> m_textures;
		std::vector<GraphicId> m_framebuffers;
		std::vector<GraphicId> m_vertexArrays;
		std::vector<GraphicId> m_programs;
	};
}
