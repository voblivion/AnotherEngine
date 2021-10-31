#include <vob/aoe/common/window/WindowCursorSystem.h>


using namespace vob::aoe::common;


WindowCursorSystem::WindowCursorSystem(ecs::WorldDataProvider& a_wdp)
	: m_worldWindowComponent{ *a_wdp.getWorldComponent<WorldWindowComponent>() }
	, m_worldCursorComponent{ *a_wdp.getWorldComponent<WorldCursorComponent const>() }
	, m_worldStop{ a_wdp.getStopBool() }
{}

void WindowCursorSystem::update() const
{
	auto& window = m_worldWindowComponent.getWindow();
	window.setCursorState(m_worldCursorComponent.m_state);
}
