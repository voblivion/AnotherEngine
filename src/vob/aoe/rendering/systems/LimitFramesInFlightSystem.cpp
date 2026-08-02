#include "vob/aoe/rendering/systems/LimitFramesInFlightSystem.h"

#include <vob/misc/std/container_util.h>

#include <algorithm>


namespace vob::aoegl
{
	namespace
	{
		constexpr GLuint64 k_fenceTimeoutNs = 1'000'000'000;
	}

	void LimitFramesInFlightSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_limitFramesInFlightCtx.init(a_wdar);
	}

	void LimitFramesInFlightSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& limitFramesInFlightCtx = m_limitFramesInFlightCtx.get(a_wdap);

		auto const fenceCount = std::max(1, limitFramesInFlightCtx.frameInFlightCapacity);
		if (mistd::isize(limitFramesInFlightCtx.fences) != fenceCount)
		{
			for (auto const fence : limitFramesInFlightCtx.fences)
			{
				if (fence != nullptr)
				{
					glDeleteSync(fence);
				}
			}
			limitFramesInFlightCtx.fences.assign(fenceCount, nullptr);
			limitFramesInFlightCtx.index = 0;
		}

		if (limitFramesInFlightCtx.fences[limitFramesInFlightCtx.index] != nullptr)
		{
			glDeleteSync(limitFramesInFlightCtx.fences[limitFramesInFlightCtx.index]);
		}
		limitFramesInFlightCtx.fences[limitFramesInFlightCtx.index] =
			glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		limitFramesInFlightCtx.index = (limitFramesInFlightCtx.index + 1) % fenceCount;

		auto const oldestFence = limitFramesInFlightCtx.fences[limitFramesInFlightCtx.index];
		if (oldestFence != nullptr)
		{
			glClientWaitSync(oldestFence, GL_SYNC_FLUSH_COMMANDS_BIT, k_fenceTimeoutNs);
		}
	}
}
