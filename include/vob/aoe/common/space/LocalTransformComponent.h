#pragma once

#include <glm/glm.hpp>

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/visitor/Aggregate.h>

namespace vob::aoe::common
{
	struct LocalTransformComponent final
		: public vis::Aggregate<LocalTransformComponent, ecs::AComponent>
	{
		// Attributes
		glm::mat4 m_matrix{ 1.0f };

		// Methods
		friend class vis::Aggregate<LocalTransformComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		// ReSharper disable once CppMemberFunctionMayBeStatic
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			glm::vec3 position;
			a_visitor.visit(vis::makeNameValuePair("Position", position));
			glm::vec3 rotation;
			a_visitor.visit(vis::makeNameValuePair("Rotation", rotation));

			a_this.m_matrix = glm::translate(a_this.m_matrix, position);
			a_this.m_matrix *= glm::mat4{ glm::quat{ rotation } };
		}
	};
}
