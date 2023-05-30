#pragma once
#include "vob/aoe/common/space/Transformcomponent.h"
#include "vob/aoe/ecs/world_data_provider.h"
#include "vob/aoe/ecs/entity_map_observer_list_ref.h"
#include "Testcomponent.h"


namespace vob::aoe::common
{
	struct TestSystem
	{
		explicit TestSystem(aoecs::world_data_provider& a_wdp)
			: m_entities{ a_wdp }
		{}

		void update() const
		{
			/*for(auto& t_entity : m_entities)
			{
				auto& t_transform = t_entity.get_component<TransformComponent>();
				auto& t_test = t_entity.get_component<TestComponent>();
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

		aoecs::entity_map_observer_list_ref<TransformComponent, TestComponent> m_entities;
	};
}
