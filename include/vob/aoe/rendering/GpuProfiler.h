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
		std::vector<GpuTimer> rootTimers;
		std::vector<GpuTimer*> scopeTimerStack;

		int32_t nextReadTimerSlotIndex = 0;
		int32_t writeTimerSlotIndex = 0;

		int32_t accumulationIndex = 0;
		int32_t accumulationCount = 50;
		int32_t runningDroppedFrameCount = 0;
		int32_t lastDroppedFrameCount = 0;
	};

	inline void pushGpuTimerScope(GpuProfiler& a_profiler, std::string_view a_name)
	{
		auto& timers = a_profiler.scopeTimerStack.empty()
			? a_profiler.rootTimers
			: a_profiler.scopeTimerStack.back()->children;

		auto timerIt = std::ranges::find(timers, a_name, &GpuTimer::name);
		if (timerIt == timers.end())
		{
			timerIt = timers.emplace(timers.end(), GpuTimer{ .name = a_name });
			for (auto& slotQueries : timerIt->queries)
			{
				glGenQueries(2, slotQueries.data());
			}
		}

		a_profiler.scopeTimerStack.push_back(&*timerIt);
		timerIt->startedInSlot[a_profiler.writeTimerSlotIndex] = true;
		glQueryCounter(timerIt->queries[a_profiler.writeTimerSlotIndex][0], GL_TIMESTAMP);
	}

	inline void popGpuTimerScope(GpuProfiler& a_profiler)
	{
		glQueryCounter(
			a_profiler.scopeTimerStack.back()->queries[a_profiler.writeTimerSlotIndex][1], GL_TIMESTAMP);
		a_profiler.scopeTimerStack.pop_back();
	}

	struct [[nodiscard]] GpuTimerGuard
	{
		GpuTimerGuard(GpuProfiler& a_profiler, std::string_view a_name)
			: m_profiler{ a_profiler }
		{
			pushGpuTimerScope(a_profiler, a_name);
		}

		GpuTimerGuard(GpuTimerGuard const&) = delete;
		GpuTimerGuard& operator=(GpuTimerGuard const&) = delete;

		~GpuTimerGuard()
		{
			popGpuTimerScope(m_profiler);
		}

	private:
		GpuProfiler& m_profiler;
	};
}

#define _VOB_AOE_CONCAT_IMPL(A, B) A##B
#define _VOB_AOE_CONCAT(A, B) _VOB_AOE_CONCAT_IMPL(A, B)

#define VOB_AOE_GPU_TIMER_SCOPE(profiler, name) \
	::vob::aoegl::GpuTimerGuard const _VOB_AOE_CONCAT(gpuTimerGuard, __LINE__){ profiler, name }
