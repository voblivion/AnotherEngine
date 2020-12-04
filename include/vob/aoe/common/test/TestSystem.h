#pragma once
#include "vob/aoe/common/space/TransformComponent.h"
#include "vob/aoe/core/ecs/WorldDataProvider.h"
#include "TestComponent.h"


namespace vob::aoe::common
{
	struct TestSystem
	{
		using Components = ecs::ComponentTypeList<
			TransformComponent
			, TestComponent
		>;

		explicit TestSystem(ecs::WorldDataProvider& a_worldDataProvider)
			: m_entities{ a_worldDataProvider.getEntityList(*this, Components{}) }
		{}

		void update() const
		{
			/*for(auto& t_entity : m_entities)
			{
				auto& t_transform = t_entity.getComponent<TransformComponent>();
				auto& t_test = t_entity.getComponent<TestComponent>();
				if (t_test.type == 1)
				{
					Quaternion r{ 1.0f, 0.0f, 0.0f, 0.0f };
					r = glm::rotate(r, 0.0001f, glm::vec3{ 0.0f, 1.0f, 0.0f });
					t_transform.m_rotation = r * t_transform.m_rotation;
				}
				else if(t_test.type == 0)
				{
					t_test.t += 0.00027f;
					if (t_test.t > 3.141592f * 2.0f)
					{
						t_test.t -= 3.141592f * 2.0f;
					}
					t_transform.m_position.x = std::sin(t_test.t);
				}
			}*/
		}

		ecs::EntityList<
			TransformComponent
			, TestComponent
		> const& m_entities;
	};
}
