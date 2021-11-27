#pragma once

#include <vob/aoe/api.h>

#include <vob/aoe/ecs/WorldDataProvider.h>

#include <vob/aoe/common/window/WorldCursorComponent.h>
#include <vob/aoe/common/window/WorldWindowComponent.h>

#include <iomanip>



namespace vob::aoe::common
{
	class VOB_AOE_API WindowCursorSystem
	{
	public:
		explicit WindowCursorSystem(aoecs::WorldDataProvider& a_wdp);

		void update() const;

	private:
		WorldWindowComponent& m_worldWindowComponent;
		WorldCursorComponent const& m_worldCursorComponent;
		bool& m_worldStop;
	};
}
