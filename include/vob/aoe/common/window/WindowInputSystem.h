#pragma once

#include <vob/aoe/input/physical_inputs_world_component.h>
#include <vob/aoe/ecs/stop_manager.h>

#include <vob/aoe/api.h>

#include <iomanip>


namespace vob::_aoecs
{
	class stop_manager;
}

namespace vob::aoecs
{
	class world_data_provider;
}

namespace vob::aoe::common
{
	struct WorldInputComponent;
	class WorldWindowComponent;

	class VOB_AOE_API WindowInputSystem
	{
	public:
		explicit WindowInputSystem(aoecs::world_data_provider& a_wdp);

		void update() const;

	private:
		WorldWindowComponent& m_worldWindowComponent;
		WorldInputComponent& m_worldInputComponent;
		aoein::physical_inputs_world_component& m_physicalInputsComponent;
		aoecs::stop_manager& m_stopManager;
	};
}
