#include "vob/aoe/rendering/systems/DebugRenderProfilingSystem.h"

#include "vob/aoe/debug/Check.h"

#include "imgui.h"



namespace vob::aoegl
{
	namespace
	{
		bool areTimerResultsReady(std::vector<GpuTimer> const& a_timers, int32_t a_slotIndex)
		{
			auto hasStartedTimer = false;
			for (auto const& timer : a_timers)
			{
				if (!timer.startedInSlot[a_slotIndex])
				{
					continue;
				}
				hasStartedTimer = true;

				GraphicInt isAvailable = 0;
				glGetQueryObjectiv(timer.queries[a_slotIndex][1], GL_QUERY_RESULT_AVAILABLE, &isAvailable);
				if (isAvailable != GL_TRUE)
				{
					return false;
				}
			}
			return hasStartedTimer;
		}

		void accumulateTimerResults(
			std::vector<GpuTimer>& a_timers, int32_t a_slotIndex, std::optional<int32_t> a_accumulationCount)
		{
			for (auto& timer : a_timers)
			{
				if (timer.startedInSlot[a_slotIndex])
				{
					uint64_t startTimeNs;
					glGetQueryObjectui64v(timer.queries[a_slotIndex][0], GL_QUERY_RESULT, &startTimeNs);
					uint64_t stopTimeNs;
					glGetQueryObjectui64v(timer.queries[a_slotIndex][1], GL_QUERY_RESULT, &stopTimeNs);
					timer.runningAccumulationNs += stopTimeNs - startTimeNs;
					timer.startedInSlot[a_slotIndex] = false;
				}

				// A scope that did not run still has to advance its averaging window, otherwise it
				// keeps displaying whatever it last measured.
				if (a_accumulationCount.has_value())
				{
					timer.lastDurationNs = timer.runningAccumulationNs / *a_accumulationCount;
					timer.runningAccumulationNs = 0;
				}

				accumulateTimerResults(timer.children, a_slotIndex, a_accumulationCount);
			}
		}

		void resetTimerStartedInSlots(std::vector<GpuTimer>& a_timers, int32_t a_slotIndex)
		{
			for (auto& timer : a_timers)
			{
				timer.startedInSlot[a_slotIndex] = false;
				resetTimerStartedInSlots(timer.children, a_slotIndex);
			}
		}

		void displayTimers(std::vector<GpuTimer> const& a_timers)
		{
			for (auto const& timer : a_timers)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextUnformatted(timer.name.data(), timer.name.data() + timer.name.size());

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f ms", static_cast<double>(timer.lastDurationNs) / 1'000'000.0);

				ImGui::Indent();
				displayTimers(timer.children);
				ImGui::Unindent();
			}
		}
	}

	void DebugRenderProfilingSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_renderProfilingCtx.init(a_wdar);
		m_debugUiCtx.init(a_wdar);
	}

	void DebugRenderProfilingSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& renderProfilingCtx = m_renderProfilingCtx.get(a_wdap);

		auto& gpuProfiler = renderProfilingCtx.gpuProfiler;

		VOB_AOE_CHECK_TERMINATE(
			gpuProfiler.scopeTimerStack.empty(), "Gpu timer scope still open.");

		while (areTimerResultsReady(gpuProfiler.rootTimers, gpuProfiler.nextReadTimerSlotIndex))
		{
			std::optional<int32_t> accumulationCount = std::nullopt;
			if (++gpuProfiler.accumulationIndex == gpuProfiler.accumulationCount)
			{
				accumulationCount = gpuProfiler.accumulationCount;
				gpuProfiler.lastDroppedFrameCount = gpuProfiler.runningDroppedFrameCount;
				gpuProfiler.runningDroppedFrameCount = 0;
				gpuProfiler.accumulationIndex = 0;
			}

			accumulateTimerResults(
				gpuProfiler.rootTimers, gpuProfiler.nextReadTimerSlotIndex, accumulationCount);

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

		resetTimerStartedInSlots(gpuProfiler.rootTimers, gpuProfiler.writeTimerSlotIndex);

		if (m_debugUiCtx.get(a_wdap).isDisplayed)
		{
			if (ImGui::Begin("Render Performance"))
			{
				ImGui::BeginDisabled();
				ImGui::InputInt("Light Count", &renderProfilingCtx.lightCount);
				ImGui::InputInt("Static Mesh Count", &renderProfilingCtx.staticOpaqueMeshCount);
				ImGui::InputInt("Rigged Mesh Count", &renderProfilingCtx.riggedOpaqueMeshCount);

				if (ImGui::BeginTable("Gpu Timers", 2, ImGuiTableFlags_BordersInnerH))
				{
					displayTimers(gpuProfiler.rootTimers);
					ImGui::EndTable();
				}

				ImGui::InputInt("Dropped Frames", &gpuProfiler.lastDroppedFrameCount);

				ImGui::EndDisabled();
			}
			ImGui::End();
		}
	}
}
