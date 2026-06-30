#include "vob/aoe/editor/AxisRotateHandle.h"

#include "vob/aoe/math/MathUtils.h"

#include "glm/gtx/norm.hpp"

#include <algorithm>
#include <limits>


namespace vob::aoedi
{
    namespace
    {
        auto computeClosestAngle(
            glm::vec3 const& a_rayPos,
            glm::vec3 const& a_rayDir,
            glm::vec3 const& a_arcCenterPos,
            glm::quat const& a_arcRotation,
            float const a_arcRadius)
        {
            auto const axis = a_arcRotation * glm::vec3{ 1.0f, 0.0f, 0.0f };
            auto const arcFrom = a_arcRotation * glm::vec3{ 0.0f, 1.0f, 0.0f };
            auto const arcTo = a_arcRotation * glm::vec3{ 0.0f, 0.0f, 1.0f };

            auto const r = a_rayPos - a_arcCenterPos;
            auto const depth = glm::distance(a_rayPos, a_arcCenterPos);

            auto const a = glm::dot(a_rayDir, a_rayDir);
            auto const b = 2.0f * glm::dot(a_rayDir, r);
            auto const c = glm::dot(r, r) - (a_arcRadius * a_arcRadius * depth * depth);

            auto const delta = b * b - 4.0f * a * c;
            auto const t0 = delta < 0.0f ? 0.0f : std::max(0.0f, (-b - std::sqrt(delta)) / (2.0f * a));
            auto const t1 = delta < 0.0f ? 0.0f : std::max(0.0f, (-b + std::sqrt(delta)) / (2.0f * a));
            auto const denom = glm::dot(axis, a_rayDir);
            auto const t2 = std::abs(denom) < 1e-6f ? 0.0f : glm::dot(axis, -r) / denom;

            auto closestDist = std::numeric_limits<float>::infinity();
            auto closestT = 0.0f;
            for (auto const t : { t0, t1, t2 })
            {
                auto const p = r + t * a_rayDir;
                auto const projected = p - glm::dot(axis, p) * axis;
                auto const d = aoema::normalizeWithDefault(projected, glm::vec3{ 0.0f });

                auto const cosFrom = glm::dot(d, arcFrom);
                auto const cosTo = glm::dot(d, arcTo);
                glm::vec3 arcPoint;
                if (cosFrom >= 0.0f && cosTo >= 0.0f)
                {
                    arcPoint = a_arcRadius * depth * d;
                }
                else if (cosFrom < cosTo)
                {
                    arcPoint = a_arcRadius * depth * arcFrom;
                }
                else
                {
                    arcPoint = a_arcRadius * depth * arcTo;
                }

                auto const dist = glm::distance(p, arcPoint);
                if (dist < closestDist)
                {
                    closestDist = dist;
                    closestT = t;
                }
            }

            auto const p = r + closestT * a_rayDir;
            auto const projected = p - glm::dot(axis, p) * axis;
            auto const d = aoema::normalizeWithDefault(projected, glm::vec3{ 0.0f });

            auto const angle = std::atan2(glm::dot(d, arcTo), glm::dot(d, arcFrom));

            return angle;
        }
    }

    float AxisRotateHandle::getDistance(glm::vec3 const& a_position, glm::vec3 const& a_direction) const
    {
        auto const axis = m_rotation * m_axis * glm::vec3{ 1.0f, 0.0f, 0.0f };
        auto const arcFrom = m_rotation * m_axis * glm::vec3{ 0.0f, 1.0f, 0.0f };
        auto const arcTo = m_rotation * m_axis * glm::vec3{ 0.0f, 0.0f, 1.0f };

        auto const r = a_position - m_position;
        auto const depth = glm::distance(a_position, m_position);

        auto const a = glm::dot(a_direction, a_direction);
        auto const b = 2.0f * glm::dot(a_direction, r);
        auto const c = glm::dot(r, r) - (m_radius * m_radius * depth * depth);

        auto const delta = b * b - 4.0f * a * c;
        auto const t0 = delta < 0.0f ? 0.0f : std::max(0.0f, (-b - std::sqrt(delta)) / (2.0f * a));
        auto const t1 = delta < 0.0f ? 0.0f : std::max(0.0f, (-b + std::sqrt(delta)) / (2.0f * a));
        auto const denom = glm::dot(axis, a_direction);
        auto const t2 = std::abs(denom) < 1e-6f ? 0.0f : glm::dot(axis, -r) / denom;

        auto const p0 = r + t0 * a_direction;
        auto const p1 = r + t1 * a_direction;
        auto const p2 = r + t2 * a_direction;

        auto closestDist = std::numeric_limits<float>::infinity();
        for (auto const& p : { p0, p1, p2 })
        {
            auto const projected = p - glm::dot(axis, p) * axis;
            auto const d = aoema::normalizeWithDefault(projected, glm::vec3{ 0.0f });
            
            auto const cosFrom = glm::dot(d, arcFrom);
            auto const cosTo = glm::dot(d, arcTo);
            glm::vec3 arcPoint;
            if (cosFrom >= 0.0f && cosTo >= 0.0f)
            {
                arcPoint = m_radius * depth * d;
            }
            else if (cosFrom < cosTo)
            {
                arcPoint = m_radius * depth * arcFrom;
            }
            else
            {
                arcPoint = m_radius * depth * arcTo;
            }

            auto const dist = glm::distance(p, arcPoint);
            closestDist = std::min(closestDist, dist);
        }

        auto const distance = closestDist / depth;

        return distance < 0.01f ? distance : std::numeric_limits<float>::infinity();
    }

    void AxisRotateHandle::beginDrag(glm::vec3 const& a_position, glm::vec3 const& a_direction)
    {
        auto const a = computeClosestAngle(a_position, a_direction, m_position, m_rotation * m_axis, m_radius);
        m_initialRotation = m_rotation;
        m_initialDragOffset = a;
    }

    void AxisRotateHandle::drag(glm::vec3 const& a_position, glm::vec3 const& a_direction)
    {
        auto const a = computeClosestAngle(a_position, a_direction, m_position, m_initialRotation * m_axis, m_radius);
        auto const axis = m_initialRotation * m_axis * glm::vec3{ 1.0f, 0.0f, 0.0f };
        m_rotation = glm::angleAxis(a - m_initialDragOffset, axis) * m_initialRotation;
    }
}
