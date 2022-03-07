#pragma once
#include "vob/aoe/common/space/Transformcomponent.h"
#include "vob/aoe/ecs/WorldDataProvider.h"
#include "Testcomponent.h"


namespace vob::aoe::common
{
	struct TestSystem
	{
		using Components = aoecs::ComponentTypeList<
			TransformComponent
			, TestComponent
		>;

		explicit TestSystem(aoecs::WorldDataProvider& a_worldDataProvider)
			: m_entities{ a_worldDataProvider.getentity_view_list(*this, Components{}) }
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

		aoecs::entity_view_list<
			TransformComponent
			, TestComponent
		> const& m_entities;
	};
}
