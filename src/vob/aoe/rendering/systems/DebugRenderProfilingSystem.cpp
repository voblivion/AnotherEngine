#include "vob/aoe/rendering/systems/DebugRenderProfilingSystem.h"

#include "vob/aoe/debug/Check.h"

#include "imgui.h"



namespace vob::aoegl
{
	namespace
	{
		bool isTimerResultReady(GpuTimer const& a_timer, int32_t a_slotIndex)
		{
			GraphicInt isAvailable = 0;
			glGetQueryObjectiv(a_timer.queries[a_slotIndex][1], GL_QUERY_RESULT_AVAILABLE, &isAvailable);
			return isAvailable == GL_TRUE;
		}

		void accumulateTimerResultsRec(GpuTimer& a_timer, int32_t a_slotIndex, std::optional<int32_t> a_accumulationCount)
		{
			uint64_t startTimeNs;
			glGetQueryObjectui64v(a_timer.queries[a_slotIndex][0], GL_QUERY_RESULT, &startTimeNs);
			uint64_t stopTimeNs;
			glGetQueryObjectui64v(a_timer.queries[a_slotIndex][1], GL_QUERY_RESULT, &stopTimeNs);
			a_timer.runningAccumulationNs += stopTimeNs - startTimeNs;
			a_timer.startedInSlot[a_slotIndex] = false;

			if (a_accumulationCount.has_value())
			{
				a_timer.lastDurationNs = a_timer.runningAccumulationNs / *a_accumulationCount;
				a_timer.runningAccumulationNs = 0;
			}

			for (auto& childTimer : a_timer.children)
			{
				if (!childTimer.startedInSlot[a_slotIndex])
				{
					continue;
				}

				accumulateTimerResultsRec(childTimer, a_slotIndex, a_accumulationCount);
			}
		}

		void resetTimerStartedInSlotsRec(GpuTimer& a_timer, int32_t a_slotIndex)
		{
			a_timer.startedInSlot[a_slotIndex] = false;

			for (auto& childTimer : a_timer.children)
			{
				resetTimerStartedInSlotsRec(childTimer, a_slotIndex);
			}
		}

		void displayTimersRec(GpuTimer const& a_timer)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(a_timer.name.data(), a_timer.name.data() + a_timer.name.size());

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.3f ms", static_cast<double>(a_timer.lastDurationNs) / 1'000'000.0);

			ImGui::Indent();
			for (auto const& childTimer : a_timer.children)
			{
				displayTimersRec(childTimer);
			}
			ImGui::Unindent();
		}
	}

	void DebugRenderProfilingSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_renderProfilingCtx.init(a_wdar);
	}

	void DebugRenderProfilingSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& renderProfilingCtx = m_renderProfilingCtx.get(a_wdap);

		auto& gpuProfiler = renderProfilingCtx.gpuProfiler;

		auto& scopeTimerStack = gpuProfiler.scopeTimerStack;
		VOB_AOE_CHECK_TERMINATE(
			scopeTimerStack.size() == 1 && scopeTimerStack[0] == &gpuProfiler.frameTimer, "Invalid scoped GPU timer stack.");

		glQueryCounter(scopeTimerStack.back()->queries[gpuProfiler.writeTimerSlotIndex][1], GL_TIMESTAMP);

		while (isTimerResultReady(gpuProfiler.frameTimer, gpuProfiler.nextReadTimerSlotIndex))
		{
			std::optional<int32_t> accumulationCount = std::nullopt;
			if (++gpuProfiler.accumulationIndex == gpuProfiler.accumulationCount)
			{
				accumulationCount = gpuProfiler.accumulationCount;
				gpuProfiler.lastDroppedFrameCount = gpuProfiler.runningDroppedFrameCount;
				gpuProfiler.runningDroppedFrameCount = 0;
				gpuProfiler.accumulationIndex = 0;
			}
			
			accumulateTimerResultsRec(gpuProfiler.frameTimer, gpuProfiler.nextReadTimerSlotIndex, accumulationCount);

			gpuProfiler.nextReadTimerSlotIndex = nextGpuTimerSlot(gpuProfiler.nextReadTimerSlotIndex);
		}

		if (nextGpuTimerSlot(gpuProfiler.writeTimerSlotIndex) != gpuProfiler.nextReadTimerSlotIndex)
		{
			gpuProfiler.writeTimerSlotIndex = nextGpuTimerSlot(gpuProfiler.writeTimerSlotIndex);
		}
		else
		{
			++gpuProfiler.runningDroppedFrameCount;
		}

		resetTimerStartedInSlotsRec(gpuProfiler.frameTimer, gpuProfiler.writeTimerSlotIndex);

		glQueryCounter(scopeTimerStack.back()->queries[gpuProfiler.writeTimerSlotIndex][0], GL_TIMESTAMP);

		if (ImGui::Begin("Render Performance"))
		{
			ImGui::BeginDisabled();
			ImGui::InputInt("Light Count", &renderProfilingCtx.lightCount);
			ImGui::InputInt("Static Mesh Count", &renderProfilingCtx.staticOpaqueMeshCount);
			ImGui::InputInt("Rigged Mesh Count", &renderProfilingCtx.riggedOpaqueMeshCount);

			if (ImGui::BeginTable("Gpu Timers", 2, ImGuiTableFlags_BordersInnerH))
			{
				displayTimersRec(gpuProfiler.frameTimer);
				ImGui::EndTable();
			}

			ImGui::InputInt("Dropped Frames", &gpuProfiler.lastDroppedFrameCount);

			ImGui::EndDisabled();
		}
		ImGui::End();
	}
}
