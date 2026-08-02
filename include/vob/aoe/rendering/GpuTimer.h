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
	};

	struct GpuTimerGuard
	{
		GpuTimerGuard(GpuTimer& a_timer, int32_t a_slotIndex)
			: m_endQuery{ a_timer.queries[a_slotIndex][1] }
		{
			a_timer.startedInSlot[a_slotIndex] = true;
			glQueryCounter(a_timer.queries[a_slotIndex][0], GL_TIMESTAMP);
		}

		GpuTimerGuard(GpuTimerGuard const&) = delete;
		GpuTimerGuard& operator=(GpuTimerGuard const&) = delete;

		~GpuTimerGuard()
		{
			glQueryCounter(m_endQuery, GL_TIMESTAMP);
		}

	private:
		GraphicId m_endQuery;
	};

	inline GpuTimerGuard startTimedGpuScope(
		std::vector<GpuTimer>& a_timers, int32_t a_slotIndex, std::string_view a_name)
	{
		auto const timer = std::ranges::find(a_timers, a_name, &GpuTimer::name);
		if (timer != a_timers.end())
		{
			return GpuTimerGuard{ *timer, a_slotIndex };
		}

		auto& newTimer = a_timers.emplace_back(GpuTimer{ .name = a_name });
		for (auto& slotQueries : newTimer.queries)
		{
			glGenQueries(2, slotQueries.data());
		}
		return GpuTimerGuard{ newTimer, a_slotIndex };
	}
}
