#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/common/time/WorldTimecomponent.h>
#include <vob/aoe/ecs/world_data_provider.h>


namespace vob::aoe::common
{
	class VOB_AOE_API TimeSystem final
	{
	public:
		// Constructors
		explicit TimeSystem(aoecs::world_data_provider& a_wdp);

		// Methods
		void update() const;

	private:
		// Attributes
		WorldTimeComponent& m_worldTimeComponent;
	};
}
