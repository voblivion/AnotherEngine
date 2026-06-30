#pragma once

#include "vob/aoe/editor/AGizmoHandle.h"

#include "glm/gtc/quaternion.hpp"


namespace vob::aoedi
{
	struct AxisTranslateHandle : public AGizmoHandle
	{
	public:
		AxisTranslateHandle(
			glm::vec3 const& a_position,
			glm::quat const& a_rotation,
			glm::quat const& a_axis,
			float const a_length)
			: m_position{ a_position }
			, m_rotation{ a_rotation }
			, m_axis{ a_axis }
			, m_length{ a_length }
		{
		}

		void updateTransform(glm::vec3 const& a_position, glm::quat const& a_rotation)
		{
			m_position = a_position;
			m_rotation = a_rotation;
		}

		glm::vec3 const& getPosition() const
		{
			return m_position;
		}

		glm::quat const& getAxis() const
		{
			return m_axis;
		}

		float getLength() const
		{
			return m_length;
		}

		float getDistance(glm::vec3 const& a_position, glm::vec3 const& a_direction) const override;

		void beginDrag(glm::vec3 const& a_position, glm::vec3 const& a_direction) override;

		void drag(glm::vec3 const& a_position, glm::vec3 const& a_direction) override;

		void endDrag() override {}

	private:
		glm::vec3 m_position;
		glm::quat m_rotation;
		glm::quat m_axis;
		float m_length;
		float m_initialDragOffset = 0.0f;
	};
}
