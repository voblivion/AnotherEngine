#include "vob/aoe/rendering/systems/DebugRenderProfilingSystem.h"

#include "imgui.h"

#include <algorithm>
#include <array>
#include <cstring>


namespace vob::aoegl
{
	namespace
	{
		bool isTimerSlotReady(std::vector<GpuTimer> const& a_timers, int32_t a_slotIndex)
		{
			auto startedTimerCount = 0;
			for (auto const& timer : a_timers)
			{
				if (!timer.startedInSlot[a_slotIndex])
				{
					continue;
				}

				++startedTimerCount;
				GraphicInt isAvailable = 0;
				glGetQueryObjectiv(timer.queries[a_slotIndex][1], GL_QUERY_RESULT_AVAILABLE, &isAvailable);
				if (isAvailable == 0)
				{
					return false;
				}
			}

			return startedTimerCount > 0;
		}

		auto toSmallStr(std::string_view a_stringView)
		{
			constexpr size_t k_maxSize = 16;
			auto size = std::min(a_stringView.size(), k_maxSize);
			std::array<char, k_maxSize + 1> smallStr;
			std::memcpy(smallStr.data(), a_stringView.data(), size);
			smallStr[size] = 0;
			return smallStr;
		}
	}

	void DebugRenderProfilingSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_renderProfilingCtx.init(a_wdar);
	}

	void DebugRenderProfilingSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& renderProfilingCtx = m_renderProfilingCtx.get(a_wdap);

		// TODO: stop the root timer here, it is the marker telling us the frame is complete.
		// stopTimedGpuScope(renderProfilingCtx.timers, renderProfilingCtx.writeTimerSlotIndex, "Frame");

		// TODO: once the root timer exists it is guaranteed to be the last query issued, so checking it
		// alone would be enough.
		while (isTimerSlotReady(
			renderProfilingCtx.timers, nextGpuTimerSlot(renderProfilingCtx.readTimerSlotIndex)))
		{
			renderProfilingCtx.readTimerSlotIndex =
				nextGpuTimerSlot(renderProfilingCtx.readTimerSlotIndex);
			for (auto& timer : renderProfilingCtx.timers)
			{
				if (timer.startedInSlot[renderProfilingCtx.readTimerSlotIndex])
				{
					uint64_t startTimeNs;
					glGetQueryObjectui64v(
						timer.queries[renderProfilingCtx.readTimerSlotIndex][0],
						GL_QUERY_RESULT,
						&startTimeNs);
					uint64_t stopTimeNs;
					glGetQueryObjectui64v(
						timer.queries[renderProfilingCtx.readTimerSlotIndex][1],
						GL_QUERY_RESULT,
						&stopTimeNs);
					timer.runningAccumulationNs += stopTimeNs - startTimeNs;
					timer.startedInSlot[renderProfilingCtx.readTimerSlotIndex] = false;
				}
			}

			if (++renderProfilingCtx.accumulationIndex == renderProfilingCtx.accumulationCount)
			{
				for (auto& timer : renderProfilingCtx.timers)
				{
					timer.lastDurationNs =
						timer.runningAccumulationNs / renderProfilingCtx.accumulationCount;
					timer.runningAccumulationNs = 0;
				}
				renderProfilingCtx.lastDroppedFrameCount = renderProfilingCtx.runningDroppedFrameCount;
				renderProfilingCtx.runningDroppedFrameCount = 0;
				renderProfilingCtx.accumulationIndex = 0;
			}
		}

		if (nextGpuTimerSlot(renderProfilingCtx.writeTimerSlotIndex) != renderProfilingCtx.readTimerSlotIndex)
		{
			renderProfilingCtx.writeTimerSlotIndex = nextGpuTimerSlot(renderProfilingCtx.writeTimerSlotIndex);
		}
		else
		{
			++renderProfilingCtx.runningDroppedFrameCount;
		}

		for (auto& timer : renderProfilingCtx.timers)
		{
			timer.startedInSlot[renderProfilingCtx.writeTimerSlotIndex] = false;
		}

		// TODO: start the root timer for the slot the next frame will write into.
		// startTimedGpuScope(renderProfilingCtx.timers, renderProfilingCtx.writeTimerSlotIndex, "Frame");

		if (ImGui::Begin("Render Performance"))
		{
			ImGui::BeginDisabled();
			ImGui::InputInt("Light Count", &renderProfilingCtx.lightCount);
			ImGui::InputInt("Static Mesh Count", &renderProfilingCtx.staticOpaqueMeshCount);
			ImGui::InputInt("Rigged Mesh Count", &renderProfilingCtx.riggedOpaqueMeshCount);

			for (auto const& timer : renderProfilingCtx.timers)
			{
				auto const timerStr = toSmallStr(timer.name);
				auto durationMs = static_cast<float>(timer.lastDurationNs) / 1'000'000.0f;
				ImGui::InputFloat(timerStr.data(), &durationMs);
			}
			ImGui::InputInt("Dropped Frames", &renderProfilingCtx.lastDroppedFrameCount);

			ImGui::EndDisabled();
		}
		ImGui::End();
	}
}
