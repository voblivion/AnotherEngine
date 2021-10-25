#include <vob/aoe/common/window/GlfwCursorSystem.h>


using namespace vob::aoe::common;


GlfwCursorSystem::GlfwCursorSystem(ecs::WorldDataProvider& a_wdp)
	: m_worldWindowComponent{ *a_wdp.getWorldComponent<WindowComponent>() }
	, m_worldCursor{ *a_wdp.getWorldComponent<CursorComponent const>() }
	, m_worldStop{ a_wdp.getStopBool() }
{}

void GlfwCursorSystem::update() const
{
	auto& window = m_worldWindowComponent.getWindow();
	window.setCursorState(m_worldCursor.m_state);
}
