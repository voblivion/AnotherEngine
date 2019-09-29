#pragma once
#include "aoe/core/ecs/Component.h"
#include "aoe/core/ecs/EntityId.h"
#include "aoe/core/ecs/ComponentManager.h"
#include <aoe/core/data/Handle.h>


namespace aoe
{
	namespace common
	{
		struct SimpleControllerComponent final
			: public vis::Aggregate<SimpleControllerComponent, ecs::AComponent>
		{
			glm::vec3 m_orientation{ 0.0f };
			data::Handle<ecs::ComponentManager> m_bullet;

			explicit SimpleControllerComponent(data::ADatabase& a_database)
				: m_bullet{ a_database }
			{}

			// Methods
			friend class vis::Aggregate<SimpleControllerComponent, ecs::AComponent>;
			template <typename VisitorType, typename ThisType>
			// ReSharper disable once CppMemberFunctionMayBeStatic
			static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
			{
				a_visitor.visit(vis::nvp("BulletArchetype", a_this.m_bullet));
			}
		};
	}
}
