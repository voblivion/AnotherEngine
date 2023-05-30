#include <vob/aoe/common/window/WindowCursorSystem.h>


using namespace vob::aoe::common;


WindowCursorSystem::WindowCursorSystem(aoecs::world_data_provider& a_wdp)
	: m_worldWindowComponent{ a_wdp.get_world_component<WorldWindowComponent>() }
	, m_worldCursorComponent{ a_wdp.get_world_component<WorldCursorComponent const>() }
	, m_stopManager{ a_wdp.get_stop_manager() }
{}

void WindowCursorSystem::update() const
{
	auto& window = m_worldWindowComponent.getWindow();
	window.setCursorState(m_worldCursorComponent.m_state);
}
