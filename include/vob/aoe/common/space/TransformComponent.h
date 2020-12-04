#pragma once

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/visitor/Aggregate.h>
#include <vob/aoe/core/type/Primitive.h>

namespace vob::aoe::common
{
	const auto g_right = vec3{ 1.0f, 0.0f, 0.0f };
	const auto g_front = vec3{ 0.0f, 1.0f, 0.0f };
	const auto g_up = vec3{ 0.0f, 0.0f, 1.0f };

	struct TransformComponent final
		: public vis::Aggregate<TransformComponent, ecs::AComponent>
	{
		// Attributes
		mat4 m_matrix{ 1.0f };

	private:
		friend class vis::Aggregate<TransformComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			glm::vec3 position;
			a_visitor.visit(vis::makeNameValuePair("Position", position));
			glm::vec3 rotation;
			a_visitor.visit(vis::makeNameValuePair("Rotation", rotation));

			a_this.m_matrix = glm::translate(a_this.m_matrix, position);
			a_this.m_matrix *= mat4{ quat{ rotation } };
		}
	};
}
