#include "vob/aoe/editor/AxisTranslateHandle.h"

#include "glm/gtx/norm.hpp"

#include <algorithm>
#include <limits>


namespace vob::aoedi
{
    namespace
    {
        auto computeClosestTimes(
            glm::vec3 const& a_rayPos, glm::vec3 const& a_rayDir, glm::vec3 const& a_segmentPos, glm::vec3 const& a_segmentOffset)
        {
            auto const r = a_rayPos - a_segmentPos;

            auto const b = glm::dot(a_rayDir, a_segmentOffset);
            auto const c = glm::length2(a_segmentOffset);
            auto const d = glm::dot(a_rayDir, r);
            auto const e = glm::dot(a_segmentOffset, r);
            auto const denom = c - b * b;

            auto t = 0.0f;
            auto u = e / c;
            if (denom > 1e-6f)
            {
                t = (b * e - c * d) / denom;
                u = (e - b * d) / denom;
            }

            return std::make_pair(t, u);
        }
    }

	float AxisTranslateHandle::getDistance(glm::vec3 const& a_position, glm::vec3 const& a_direction) const
	{
        auto const r = a_position - m_position;
        auto const depth = glm::distance(a_position, m_position);

        auto const dir = m_rotation * m_axis * glm::vec3{ m_length * depth, 0.0f, 0.0f };

        auto const b = glm::dot(a_direction, dir);
        auto const c = glm::length2(dir);
        auto const d = glm::dot(a_direction, r);
        auto const e = glm::dot(dir, r);
        auto const denom = c - b * b;

        auto t = 0.0f;
        auto u = e / c;
        if (denom > 1e-6f)
        {
            t = (b * e - c * d) / denom;
            u = (e - b * d) / denom;
        }
        u = std::clamp(u, 0.0f, 1.0f);
        t = (b * u - d);
        if (t < 0.0f)
        {
            t = 0.0f;
            u = std::clamp(e / c, 0.0f, 1.0f);
        }

        auto const rayPoint = a_position + t * a_direction;
        auto const segmentPoint = m_position + u * dir;
        auto const distance = glm::distance(rayPoint, segmentPoint) / depth;
        return distance < 0.01f ? distance : std::numeric_limits<float>::infinity();
	}

    void AxisTranslateHandle::beginDrag(glm::vec3 const& a_position, glm::vec3 const& a_direction)
    {
        auto const dir = m_rotation * m_axis * glm::vec3{ 1.0f, 0.0f, 0.0f };
        auto const [t, u] = computeClosestTimes(a_position, a_direction, m_position, dir);
        m_initialDragOffset = u;
    }

    void AxisTranslateHandle::drag(glm::vec3 const& a_position, glm::vec3 const& a_direction)
    {
        auto const dir = m_rotation * m_axis * glm::vec3{ 1.0f, 0.0f, 0.0f };
        auto const [t, u] = computeClosestTimes(a_position, a_direction, m_position, dir);
        m_position += (u - m_initialDragOffset) * dir;
    }
}
