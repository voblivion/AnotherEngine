#pragma once

#include <vob/aoe/input/physical_inputs_world_component.h>

#include <vob/aoe/api.h>

#include <iomanip>


namespace vob::aoecs
{
	class WorldDataProvider;
	class stop_manager;
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
		aoein::physical_inputs_world_component& m_physicalInputsComponent;
		aoecs::stop_manager& m_stopManager;
	};
}
