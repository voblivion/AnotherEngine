#include "vob/aoe/spacetime/DebugDisplaySimulationFrameTimeSystem.h"

#include "imgui.h"
#include "implot.h"


namespace vob::aoest
{
	void DebugDisplaySimulationFrameTimeSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{

	}

	void DebugDisplaySimulationFrameTimeSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto& debugSimulationFrameTimeHistoryContext = m_debugSimulationFrameTimeHistoryContext.get(a_wdap);
		if (ImGui::Begin("Debug Simulation Frame Time"))
		{
			ImGui::InputInt("History Length", &debugSimulationFrameTimeHistoryContext.historyLength);
			if (debugSimulationFrameTimeHistoryContext.durationsInMs.size() != debugSimulationFrameTimeHistoryContext.historyLength)
			{
				debugSimulationFrameTimeHistoryContext.durationsInMs.resize(debugSimulationFrameTimeHistoryContext.historyLength, 0.0f);
				if (debugSimulationFrameTimeHistoryContext.nextIndex >= debugSimulationFrameTimeHistoryContext.historyLength)
				{
					debugSimulationFrameTimeHistoryContext.nextIndex = 0;
				}
			}
			if (ImPlot::BeginPlot("Tick Durations", ImVec2(-1, 150)))
			{
				ImPlot::SetupAxes(nullptr, "ms", ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_None);
				auto const maxTickDuration = std::max(10.0f, *std::max_element(
					debugSimulationFrameTimeHistoryContext.durationsInMs.begin(),
					debugSimulationFrameTimeHistoryContext.durationsInMs.end()));
				ImPlot::SetupAxesLimits(0, debugSimulationFrameTimeHistoryContext.historyLength, 0, maxTickDuration, ImPlotCond_Always);
				ImPlotSpec spec;
				spec.Offset = debugSimulationFrameTimeHistoryContext.nextIndex;
				spec.Flags = ImPlotLineFlags_None;
				ImPlot::PlotLine(
					"##TickDurations",
					debugSimulationFrameTimeHistoryContext.durationsInMs.data(),
					debugSimulationFrameTimeHistoryContext.historyLength,
					1.0,  // xscale
					0.0,  // xstart
					spec);
				ImPlot::EndPlot();
			}
		}
		ImGui::End();
	}
}
