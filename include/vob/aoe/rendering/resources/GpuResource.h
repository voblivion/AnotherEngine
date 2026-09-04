#pragma once

#include "vob/aoe/rendering/resources/GpuDeleteQueue.h"

#include <utility>


namespace vob::aoegl
{
	template <auto TDeleteQueuePush>
	class GpuResource
	{
	public:
		GpuResource() = default;

		GpuResource(GpuDeleteQueue& a_deleteQueue, GraphicId const a_id)
			: m_deleteQueue{ &a_deleteQueue }
			, m_id{ a_id }
		{}

		GpuResource(GpuResource const&) = delete;
		GpuResource& operator=(GpuResource const&) = delete;

		GpuResource(GpuResource&& a_other) noexcept
			: m_deleteQueue{ std::exchange(a_other.m_deleteQueue, nullptr) }
			, m_id{ std::exchange(a_other.m_id, k_invalidId) }
		{}

		GpuResource& operator=(GpuResource&& a_other) noexcept
		{
			if (this != &a_other)
			{
				release();
				m_deleteQueue = std::exchange(a_other.m_deleteQueue, nullptr);
				m_id = std::exchange(a_other.m_id, k_invalidId);
			}
			return *this;
		}

		~GpuResource()
		{
			release();
		}

		operator GraphicId() const
		{
			return m_id;
		}

	private:
		GpuDeleteQueue* m_deleteQueue = nullptr;
		GraphicId m_id = k_invalidId;

		void release()
		{
			if (m_deleteQueue != nullptr && m_id != k_invalidId)
			{
				(m_deleteQueue->*TDeleteQueuePush)(m_id);
			}
		}
	};

	using GpuBuffer = GpuResource<&GpuDeleteQueue::pushBuffer>;
	using GpuTexture = GpuResource<&GpuDeleteQueue::pushTexture>;
	using GpuFramebuffer = GpuResource<&GpuDeleteQueue::pushFramebuffer>;
	using GpuVertexArray = GpuResource<&GpuDeleteQueue::pushVertexArray>;
	using GpuProgram = GpuResource<&GpuDeleteQueue::pushProgram>;
	using GpuQuery = GpuResource<&GpuDeleteQueue::pushQuery>;
}
