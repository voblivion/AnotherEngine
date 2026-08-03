#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>
#include <vector>


namespace vob::aoegl
{
	constexpr int32_t k_gpuTimerSlotCount = 4;

	constexpr int32_t nextGpuTimerSlot(int32_t a_slotIndex)
	{
		return (a_slotIndex + 1) % k_gpuTimerSlotCount;
	}

	struct GpuTimer
	{
		std::string_view name;
		std::array<std::array<GraphicId, 2>, k_gpuTimerSlotCount> queries{};
		std::array<bool, k_gpuTimerSlotCount> startedInSlot{};
		uint64_t runningAccumulationNs = 0;
		uint64_t lastDurationNs = 0;

		std::vector<GpuTimer> children;
	};

	struct GpuProfiler
	{
		GpuTimer frameTimer{ .name = "Frame" };
		std::vector<GpuTimer*> scopeTimerStack;

		int32_t nextReadTimerSlotIndex = 0;
		int32_t writeTimerSlotIndex = 0;

		int32_t accumulationIndex = 0;
		int32_t accumulationCount = 50;
		int32_t runningDroppedFrameCount = 0;
		int32_t lastDroppedFrameCount = 0;
	};

	struct [[nodiscard]] GpuTimerGuard
	{
		GpuTimerGuard(GpuProfiler& a_profiler, std::string_view a_name)
			: m_stack{ a_profiler.scopeTimerStack }
			, m_slotIndex{ a_profiler.writeTimerSlotIndex }
		{
			auto& timers = m_stack.back()->children;
			auto timerIt = std::ranges::find(timers, a_name, &GpuTimer::name);
			if (timerIt == timers.end())
			{
				timerIt = timers.emplace(timers.end(), GpuTimer{ .name = a_name });
				for (auto& slotQueries : timerIt->queries)
				{
					glGenQueries(2, slotQueries.data());
				}
			}

			m_stack.push_back(&*timerIt);
			timerIt->startedInSlot[m_slotIndex] = true;
			glQueryCounter(timerIt->queries[m_slotIndex][0], GL_TIMESTAMP);
		}

		GpuTimerGuard(GpuTimerGuard const&) = delete;
		GpuTimerGuard& operator=(GpuTimerGuard const&) = delete;

		~GpuTimerGuard()
		{
			glQueryCounter(m_stack.back()->queries[m_slotIndex][1], GL_TIMESTAMP);
			m_stack.pop_back();
		}

	private:
		std::vector<GpuTimer*>& m_stack;
		int32_t m_slotIndex;
	};
}

#define _VOB_AOE_CONCAT_IMPL(A, B) A##B
#define _VOB_AOE_CONCAT(A, B) _VOB_AOE_CONCAT_IMPL(A, B)

#define VOB_AOE_GPU_TIMER_SCOPE(profiler, name) \
	::vob::aoegl::GpuTimerGuard const _VOB_AOE_CONCAT(gpuTimerGuard, __LINE__){ profiler, name }
