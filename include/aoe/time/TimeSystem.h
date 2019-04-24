#pragma once

#include <aoe/core/ecs/System.h>
#include <aoe/core/ecs/WorldDataProvider.h>
#include <aoe/time/Export.h>
#include <aoe/time/TimeComponent.h>


namespace aoe
{
	namespace time
	{
		class AOE_TIME_API TimeSystem final
			: public ecs::ASystem
		{
		public:
			// Constructors
			explicit TimeSystem(ecs::WorldDataProvider& a_worldDataProvider);

			// Methods
			void update() const override;

		private:
			// Attributes
			TimeComponent& m_worldTime;
		};
	}
}
