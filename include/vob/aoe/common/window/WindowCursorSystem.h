#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/ecs/world_data_provider.h>

#include <vob/aoe/common/window/WorldCursorcomponent.h>
#include <vob/aoe/common/window/WorldWindowcomponent.h>

#include <iomanip>



namespace vob::aoe::common
{
	class VOB_AOE_API WindowCursorSystem
	{
	public:
		explicit WindowCursorSystem(aoecs::world_data_provider& a_wdp);

		void update() const;

	private:
		WorldWindowComponent& m_worldWindowComponent;
		WorldCursorComponent const& m_worldCursorComponent;
		aoecs::stop_manager& m_stopManager;
	};
}
