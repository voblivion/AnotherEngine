#pragma once

#include <aoe/core/ecs/System.h>
#include <aoe/core/ecs/WorldDataProvider.h>
#include <aoe/common/Export.h>
#include <aoe/common/space/MoveComponent.h>
#include <aoe/common/space/TransformComponent.h>
#include <aoe/common/time/TimeComponent.h>

namespace aoe
{
	namespace common
	{
		class AOE_COMMON_API MoveSystem final
			: public ecs::ASystem
		{
		public:
			explicit MoveSystem(ecs::WorldDataProvider& a_worldDataProvider);

			virtual void update() const override;

		private:
			TimeComponent& m_worldTime;
			ecs::SystemEntityList<TransformComponent
				, MoveComponent const> const& m_entities;
		};
	}
}
