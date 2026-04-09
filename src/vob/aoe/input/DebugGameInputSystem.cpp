#include "vob/aoe/input/DebugGameInputSystem.h"

#include "vob/aoe/input/DebugGameInputContext.h"
#include "vob/aoe/input/GameInputContext.h"

#include "imgui.h"
#include "implot.h"


namespace vob::aoein
{
	namespace
	{
		float getEventCount(void* a_data, int32_t a_index)
		{
			return static_cast<float>(reinterpret_cast<int32_t*>(a_data)[a_index]);
		}
	}

	void DebugGameInputSystem::init(aoeng::EcsWorldDataAccessRegistrar& a_wdar)
	{
		m_gameInputContext.init(a_wdar);
		m_debugGameInputContext.init(a_wdar);
	}

	void DebugGameInputSystem::execute(aoeng::EcsWorldDataAccessProvider const& a_wdap) const
	{
		auto const& gameInputCtx = m_gameInputContext.get(a_wdap);
		auto& debugGameInputCtx = m_debugGameInputContext.get(a_wdap);

		for (auto& debugValue : debugGameInputCtx.values)
		{
			if (debugValue.values.size() != debugGameInputCtx.historyLength)
			{
				debugValue.values.resize(debugGameInputCtx.historyLength, 0.0f);
			}

			debugValue.values[debugGameInputCtx.nextIndex] = gameInputCtx.getValue(debugValue.id);
		}

		for (auto& debugEvent : debugGameInputCtx.events)
		{
			if (debugEvent.events.size() != debugGameInputCtx.historyLength)
			{
				debugEvent.events.resize(debugGameInputCtx.historyLength, 0);
			}

			debugEvent.events[debugGameInputCtx.nextIndex] = 0;
		}
		auto const& events = gameInputCtx.getEvents();
		for (auto const& event : events)
		{
			for (auto& debugEvent : debugGameInputCtx.events)
			{
				if (debugEvent.id == event)
				{
					++debugEvent.events[debugGameInputCtx.nextIndex];
				}
			}
		}

		auto nextHistoryLength = debugGameInputCtx.historyLength;
		if (ImGui::Begin("Inputs"))
		{
			ImGui::InputInt("History Length", &nextHistoryLength, 1, 64);
			if (ImGui::BeginTable("ValueTable", 3, ImGuiTableFlags_SizingStretchProp))
			{
				ImGui::TableSetupColumn("Name", 0 /* flags */, 50.0f /* weight */);
				ImGui::TableSetupColumn("Value", 0 /* flags */, 20.0f /* weight */);
				ImGui::TableSetupColumn("History", 0 /* flags */, 150.0f /* weight */);
				ImGui::TableHeadersRow();

				for (auto const& debugValue : debugGameInputCtx.values)
				{
					ImGui::TableNextRow();
					
					ImGui::TableNextColumn();
					ImGui::Text(debugValue.name);
					
					ImGui::TableNextColumn();
					ImGui::Text("%.4f", debugValue.values[debugGameInputCtx.nextIndex]);

					ImGui::TableNextColumn();
					ImGui::PushItemWidth(-1);
					ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));
					if (ImPlot::BeginPlot("##HistoryPlot", ImVec2(-1, 30)))
					{
						ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
						if (debugValue.range.first < debugValue.range.second)
						{
							ImPlot::SetupAxesLimits(
								0, debugGameInputCtx.historyLength, debugValue.range.first, debugValue.range.second, ImPlotCond_Always);
							ImPlot::PlotLine(
								"##Values",
								debugValue.values.data(),
								debugGameInputCtx.historyLength,
								1.0,  // xscale
								0.0,  // xstart
								ImPlotLineFlags_None,
								debugGameInputCtx.nextIndex);
						}
						ImPlot::EndPlot();
					}
					ImPlot::PopStyleVar();
					ImGui::PopItemWidth();
				}

				for (auto const& debugEvent : debugGameInputCtx.events)
				{
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::Text(debugEvent.name);

					ImGui::TableNextColumn();
					ImGui::Text("%d", debugEvent.events[debugGameInputCtx.nextIndex]);

					ImGui::TableNextColumn();
					ImGui::PushItemWidth(-1);
					ImGui::PlotHistogram(
						"##HistoryPlot",
						&getEventCount,
						const_cast<int32_t*>(debugEvent.events.data()),
						debugGameInputCtx.historyLength,
						debugGameInputCtx.nextIndex);
					ImGui::PopItemWidth();
				}

				ImGui::EndTable();
			}
		}
		ImGui::End();

		debugGameInputCtx.historyLength = nextHistoryLength;
		debugGameInputCtx.nextIndex = (debugGameInputCtx.nextIndex + 1) % debugGameInputCtx.historyLength;
	}
}
