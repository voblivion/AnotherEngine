#pragma once

#include <vob/aoe/physics/shapes.h>

#include <vob/aoe/spacetime/transform.h>

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>

#include "optick.h"

namespace vob::aoeph
{
	template <typename T>
	T square(T const a_value)
	{
		return a_value * a_value;
	}

	inline bool is_inside(glm::vec3 const& a_point, triangle const& a_triangle)
	{
		auto const normal = glm::cross(a_triangle.p1 - a_triangle.p0, a_triangle.p2 - a_triangle.p0);
		auto const d0 = glm::dot(glm::cross(a_triangle.p1 - a_triangle.p0, a_point - a_triangle.p0), normal);
		auto const d1 = glm::dot(glm::cross(a_triangle.p2 - a_triangle.p1, a_point - a_triangle.p1), normal);
		auto const d2 = glm::dot(glm::cross(a_triangle.p0 - a_triangle.p2, a_point - a_triangle.p2), normal);
		return d0 * d1 > 0.0f && d1 * d2 > 0.0f;
	}

	inline bool test_intersection(aabb const& a_lhs, aabb const& a_rhs)
	{
		if (a_lhs.max.x < a_rhs.min.x || a_lhs.max.y < a_rhs.min.y || a_lhs.max.z < a_rhs.min.z)
		{
			return false;
		}

		if (a_rhs.max.x < a_lhs.min.x || a_rhs.max.y < a_lhs.min.y || a_rhs.max.z < a_lhs.min.z)
		{
			return false;
		}

		return true;
	}

	inline aabb compute_ellipsoid_approximate_aabb(glm::vec3 const& a_position, glm::vec3 const& a_radiuses)
	{
		auto const maxRadius = std::max(a_radiuses.x, std::max(a_radiuses.y, a_radiuses.z));
		auto const maxOffset = glm::vec3{ maxRadius };
		return {
			a_position - maxOffset,
			a_position + maxOffset
		};
	}

	inline aabb compute_triangle_aabb(triangle const& a_triangle)
	{
		return {
			glm::vec3{
				std::min(a_triangle.p0.x, std::min(a_triangle.p1.x, a_triangle.p2.x)),
				std::min(a_triangle.p0.y, std::min(a_triangle.p1.y, a_triangle.p2.y)),
				std::min(a_triangle.p0.z, std::min(a_triangle.p1.z, a_triangle.p2.z))
			},
			glm::vec3{
				std::max(a_triangle.p0.x, std::max(a_triangle.p1.x, a_triangle.p2.x)),
				std::max(a_triangle.p0.y, std::max(a_triangle.p1.y, a_triangle.p2.y)),
				std::max(a_triangle.p0.z, std::max(a_triangle.p1.z, a_triangle.p2.z))
			}
		};
	}

	inline glm::vec3 find_closest_triangle_point(triangle const& a_triangle, glm::vec3 const& a_point)
	{
		auto const v01 = a_triangle.p1 - a_triangle.p0;
		auto const v02 = a_triangle.p2 - a_triangle.p0;
		auto const v0p = a_point - a_triangle.p0;

		auto const d1 = glm::dot(v01, v0p);
		auto const d2 = glm::dot(v02, v0p);
		if (d1 <= 0.0f && d2 <= 0.0f)
		{
			return a_triangle.p0;
		}

		auto const v13 = a_point - a_triangle.p1;
		auto const d3 = glm::dot(v01, v13);
		auto const d4 = glm::dot(v02, v13);
		if (d3 >= 0.0f && d4 <= d3)
		{
			return a_triangle.p1;
		}

		auto const v2 = d1 * d4 - d2 * d3;
		if (v2 <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
		{
			return a_triangle.p0 + v01 * d1 / (d1 - d3);
		}

		auto const v2p = a_point - a_triangle.p2;
		auto const d5 = glm::dot(v01, v2p);
		auto const d6 = glm::dot(v02, v2p);
		if (d6 >= 0.0f && d5 <= d6)
		{
			return a_triangle.p2;
		}

		auto const v1 = d2 * d5 - d1 * d6;
		if (v1 <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
		{
			return a_triangle.p0 + v02 * d2 / (d2 - d6);
		}

		auto const v0 = d3 * d6 - d4 * d5;
		if (v0 <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
		{
			return a_triangle.p1 + (a_triangle.p2 - a_triangle.p1) * (d4 - d3) / ((d4 - d3) + (d5 - d6));
		}

		auto const d01Sq = glm::dot(v01, v01);
		auto const d02Sq = glm::dot(v02, v02);
		auto const d0102 = glm::dot(v01, v02);

		auto const d = d01Sq * d02Sq - d0102 * d0102;
		auto const v = (d02Sq * glm::dot(v0p, v01) - d0102 * glm::dot(v0p, v02)) / d;
		auto const w = (d01Sq * glm::dot(v0p, v02) - d0102 * glm::dot(v0p, v01)) / d;
		return a_triangle.p0 + v01 * v + v02 * w;
	}

	inline glm::vec3 find_closest_unit_ellipsoid_point(glm::vec3 const& a_radiuses, glm::vec3 const& a_point)
	{
		auto const r2 = a_radiuses * a_radiuses;
		auto const computeError = [&a_point, &a_radiuses, &r2](const float lambda) {
			auto const p2 = a_point * a_point;
			auto const dSqrt = r2 + lambda;
			auto const t = p2 * r2 / (dSqrt * dSqrt);
			return t.x + t.y + t.z - 1.0f;
			};

		auto lambdaMin = -std::min({ r2.x, r2.y, r2.z });
		auto lambdaMax = 0.0f;
		while (lambdaMax - lambdaMin > 1e-4f)
		{
			auto const lambda = (lambdaMin + lambdaMax) * 0.5f;
			auto const error = computeError(lambda);
			if (error < 0.0f)
			{
				lambdaMax = lambda;
			}
			else
			{
				lambdaMin = lambda;
			}
		}

		auto const lambda = (lambdaMin + lambdaMax) * 0.5f;
		return a_point * r2 / (r2 + lambda);
	}

	struct intersection_result
	{
		glm::vec3 firstPoint = glm::vec3{ 0.0f };
		glm::vec3 secondPoint = glm::vec3{ 0.0f };
		float signedDistance = 1.0f;
	};

	inline intersection_result intersect_unit_ellipsoid_with_triangle(glm::vec3 const& a_radiuses, triangle const& a_triangle)
	{
		auto const normal = glm::normalize(glm::cross(a_triangle.p1 - a_triangle.p0, a_triangle.p2 - a_triangle.p0));
		if (glm::dot(normal, -a_triangle.p0) < 0.0f)
		{
			return {};
		}

		auto const normalEllipsoidDir = normal * a_radiuses * a_radiuses;
		auto const normalEllipsoidPoint = -normalEllipsoidDir / std::sqrt(glm::dot(normalEllipsoidDir, normal));
		auto const normalDistance = glm::dot(normalEllipsoidPoint - a_triangle.p0, normal);
		auto const normalTrianglePoint = normalEllipsoidPoint - normalDistance * normal;

		auto const trianglePoint = find_closest_triangle_point(a_triangle, normalTrianglePoint);
		auto const ellipsoidPoint = find_closest_unit_ellipsoid_point(a_radiuses, trianglePoint);
		auto const distance = glm::length(trianglePoint - ellipsoidPoint);
		auto const t = trianglePoint / a_radiuses;
		return intersection_result{ ellipsoidPoint, trianglePoint, glm::dot(t, t) < 1.0f ? -distance : distance };
	}

	inline intersection_result intersect_ellipsoid_with_triangle(
		glm::mat4 const& a_ellipsoidTransform,
		glm::mat4 const& a_ellipsoidTransformInv,
		glm::vec3 const& a_radiuses,
		triangle const& a_triangle)
	{
		OPTICK_EVENT("INTERSECT_E_WITH_T")

		auto intersection = intersect_unit_ellipsoid_with_triangle(
			a_radiuses,
			triangle{
				aoest::apply(a_ellipsoidTransformInv, a_triangle.p0),
				aoest::apply(a_ellipsoidTransformInv, a_triangle.p1),
				aoest::apply(a_ellipsoidTransformInv, a_triangle.p2)
			});

		auto const ellipsoidPoint = aoest::apply(a_ellipsoidTransform, intersection.firstPoint);
		auto const trianglePoint = aoest::apply(a_ellipsoidTransform, intersection.secondPoint);
		return intersection_result{ ellipsoidPoint, trianglePoint, intersection.signedDistance };
	}

	struct cast_result
	{
		float displacementRatio;
		glm::vec3 point;
		glm::vec3 normal;
	};

	inline cast_result cast_unit_ellipsoid_with_plane(
		glm::vec3 const& a_radiuses,
		glm::vec3 const& a_displacement,
		plane const& a_plane)
	{
		auto q = a_radiuses * a_radiuses;
		auto const ellipsoidPoint = -a_plane.normal * a_radiuses * a_radiuses / glm::length(a_radiuses * a_plane.normal);
		auto const displacementRatio = std::min(1.0f, -glm::dot(ellipsoidPoint - a_plane.point, a_plane.normal) / glm::dot(a_displacement, a_plane.normal));

		// TODO: clamp to 1.0f

		return cast_result{ displacementRatio, ellipsoidPoint + displacementRatio * a_displacement, a_plane.normal };
	}

	inline cast_result cast_unit_ellipsoid_with_triangle(
		glm::vec3 const& a_radiuses,
		glm::vec3 const& a_displacement,
		triangle const& a_triangle)
	{
		// TODO: inside already?

		auto intersectionResult = intersect_unit_ellipsoid_with_triangle(a_radiuses, a_triangle);
		if (intersectionResult.signedDistance < 0.0f)
		{
			return cast_result{ 0.0f, intersectionResult.firstPoint, glm::normalize(intersectionResult.secondPoint - intersectionResult.firstPoint) };
		}

		auto const planeNormal = glm::normalize(glm::cross(a_triangle.p1 - a_triangle.p0, a_triangle.p2 - a_triangle.p0));
		auto const castResult = cast_unit_ellipsoid_with_plane(a_radiuses, a_displacement, plane{ a_triangle.p0, planeNormal });
		if (is_inside(-planeNormal + a_displacement * castResult.displacementRatio, a_triangle))
		{
			return castResult;
		}

		return {1.0f};
	}

	inline cast_result cast_ellipsoid_with_triangle(
		glm::mat4 const& a_ellipsoidTransform,
		glm::mat4 const& a_ellipsoidTransformInv,
		glm::vec3 const& a_radiuses,
		glm::vec3 const& a_displacement,
		triangle const& a_triangle)
	{
		auto const castResult = cast_unit_ellipsoid_with_triangle(
			a_radiuses,
			glm::vec3{ a_ellipsoidTransformInv * glm::vec4{ a_displacement, 0.0f } },
			triangle{
				aoest::apply(a_ellipsoidTransformInv, a_triangle.p0),
				aoest::apply(a_ellipsoidTransformInv, a_triangle.p1),
				aoest::apply(a_ellipsoidTransformInv, a_triangle.p2),
			});

		return { castResult.displacementRatio, aoest::apply(a_ellipsoidTransform, castResult.point), glm::vec3{ a_ellipsoidTransform * glm::vec4{castResult.normal, 0.0f} } };
	}

	inline glm::quat differentiate_quaternion(glm::quat const& a_rotation, glm::vec3 const& a_angularVelocity)
	{
		auto const angularVelocity = glm::quat{ 0.0f, a_angularVelocity.x, a_angularVelocity.y, a_angularVelocity.z };
		return 0.5f * a_rotation * angularVelocity;
	}
}
