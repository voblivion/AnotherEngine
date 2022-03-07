#include <vob/aoe/common/window/WindowCursorSystem.h>


using namespace vob::aoe::common;


WindowCursorSystem::WindowCursorSystem(aoecs::WorldDataProvider& a_wdp)
	: m_worldWindowComponent{ *a_wdp.getWorldComponent<WorldWindowComponent>() }
	, m_worldCursorComponent{ *a_wdp.getWorldComponent<WorldCursorComponent const>() }
	, m_stopManager{ a_wdp.getStopManager() }
{}

void WindowCursorSystem::update() const
{
	auto& window = m_worldWindowComponent.getWindow();
	window.setCursorState(m_worldCursorComponent.m_state);
}
