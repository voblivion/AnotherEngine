#pragma once

#include <vob/aoe/api.h>
#include <vob/aoe/common/time/TimeComponent.h>
#include <vob/aoe/core/ecs/WorldDataProvider.h>


namespace vob::aoe::common
{
	class VOB_AOE_API TimeSystem final
	{
	public:
		// Constructors
		explicit TimeSystem(ecs::WorldDataProvider& a_worldDataProvider);

		// Methods
		void update() const;

	private:
		// Attributes
		TimeComponent& m_worldTime;
	};
}
