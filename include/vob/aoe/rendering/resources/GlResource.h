#pragma once

#include "vob/aoe/rendering/resources/GlDeleteQueue.h"

#include <utility>


namespace vob::aoegl
{
	template <auto TDeleteQueuePush>
	class GlResource
	{
	public:
		GlResource() = default;

		GlResource(GlDeleteQueue& a_deleteQueue, GraphicId const a_id)
			: m_deleteQueue{ &a_deleteQueue }
			, m_id{ a_id }
		{}

		GlResource(GlResource const&) = delete;
		GlResource& operator=(GlResource const&) = delete;

		GlResource(GlResource&& a_other) noexcept
			: m_deleteQueue{ std::exchange(a_other.m_deleteQueue, nullptr) }
			, m_id{ std::exchange(a_other.m_id, k_invalidId) }
		{}

		GlResource& operator=(GlResource&& a_other) noexcept
		{
			if (this != &a_other)
			{
				release();
				m_deleteQueue = std::exchange(a_other.m_deleteQueue, nullptr);
				m_id = std::exchange(a_other.m_id, k_invalidId);
			}
			return *this;
		}

		~GlResource()
		{
			release();
		}

		operator GraphicId() const
		{
			return m_id;
		}

	private:
		GlDeleteQueue* m_deleteQueue = nullptr;
		GraphicId m_id = k_invalidId;

		void release()
		{
			if (m_deleteQueue != nullptr && m_id != k_invalidId)
			{
				(m_deleteQueue->*TDeleteQueuePush)(m_id);
			}
		}
	};

	using GlBuffer = GlResource<&GlDeleteQueue::pushBuffer>;
	using GlTexture = GlResource<&GlDeleteQueue::pushTexture>;
	using GlFramebuffer = GlResource<&GlDeleteQueue::pushFramebuffer>;
	using GlVertexArray = GlResource<&GlDeleteQueue::pushVertexArray>;
	using GlProgram = GlResource<&GlDeleteQueue::pushProgram>;
}
