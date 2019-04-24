#pragma once

#include <aoe/common/Export.h>
#include <aoe/common/time/TimeComponent.h>
#include <aoe/core/ecs/System.h>
#include <aoe/core/ecs/WorldDataProvider.h>


namespace aoe
{
	namespace common
	{
		class AOE_COMMON_API TimeSystem final
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
