#pragma once

#include <aoe/core/ecs/System.h>
#include <aoe/core/ecs/WorldDataProvider.h>
#include <aoe/space/Export.h>
#include <aoe/space/MoveComponent.h>
#include <aoe/space/TransformComponent.h>
#include <aoe/time/TimeComponent.h>

namespace aoe
{
	namespace space
	{
		class AOE_SPACE_API MoveSystem final
			: public ecs::ASystem
		{
		public:
			explicit MoveSystem(ecs::WorldDataProvider& a_worldDataProvider);

			virtual void update() const override;

		private:
			time::TimeComponent& m_worldTime;
			ecs::SystemEntityList<TransformComponent
				, MoveComponent const> const& m_entities;
		};
	}
}
