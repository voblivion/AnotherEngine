#pragma once

#include <vob/aoe/api.h>

#include <iomanip>


namespace vob::aoe::aoecs
{
	class WorldDataProvider;
}

namespace vob::aoe::common
{
	struct WorldInputComponent;
	class WorldWindowComponent;

	class VOB_AOE_API WindowInputSystem
	{
	public:
		explicit WindowInputSystem(aoecs::WorldDataProvider& a_wdp);

		void update() const;

	private:
		WorldWindowComponent& m_worldWindowComponent;
		WorldInputComponent& m_worldInputComponent;
		bool& m_worldStop;
	};
}
