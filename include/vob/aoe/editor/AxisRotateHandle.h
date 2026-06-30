#pragma once

#include "vob/aoe/editor/AGizmoHandle.h"

#include "glm/gtc/quaternion.hpp"


namespace vob::aoedi
{
	struct AxisRotateHandle : public AGizmoHandle
	{
	public:
		AxisRotateHandle(
			glm::vec3 const& a_position,
			glm::quat const& a_rotation,
			glm::quat const& a_axis,
			float a_radius)
			: m_position{ a_position }
			, m_rotation{ a_rotation }
			, m_axis{ a_axis }
			, m_radius{ a_radius }
		{
		}

		void updateTransform(glm::vec3 const& a_position, glm::quat const& a_rotation)
		{
			m_position = a_position;
			m_rotation = a_rotation;
		}

		glm::quat const& getRotation() const
		{
			return m_rotation;
		}

		glm::quat const& getAxis() const
		{
			return m_axis;
		}

		float getRadius() const
		{
			return m_radius;
		}

		float getDistance(glm::vec3 const& a_position, glm::vec3 const& a_direction) const override;

		void beginDrag(glm::vec3 const& a_position, glm::vec3 const& a_direction) override;

		void drag(glm::vec3 const& a_position, glm::vec3 const& a_direction) override;

		void endDrag() override {}

	private:
		glm::vec3 m_position;
		glm::quat m_rotation;
		glm::quat m_axis;
		float m_radius;
		glm::quat m_initialRotation;
		float m_initialDragOffset = 0.0f;
	};
}
