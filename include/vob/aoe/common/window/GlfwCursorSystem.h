#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/core/ecs/WorldDataProvider.h>

#include <vob/aoe/common/window/CursorComponent.h>
#include <vob/aoe/common/window/WindowComponent.h>

#include <iomanip>



namespace vob::aoe::common
{
	class VOB_AOE_API GlfwCursorSystem
	{
	public:
		explicit GlfwCursorSystem(ecs::WorldDataProvider& a_wdp);

		void update() const;

	private:
		WindowComponent& m_worldWindowComponent;
		CursorComponent const& m_worldCursor;
		bool& m_worldStop;
	};
}
