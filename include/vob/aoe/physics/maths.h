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

	inline float lengthSquared(glm::vec3 const& a_vector)
	{
		return glm::dot(a_vector, a_vector);
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

	inline bool test_aabb_intersection(glm::vec3 const& a_halfExtents, glm::vec3 const& a_p0, glm::vec3 const& a_p1, glm::vec3 const& a_p2)
	{
		auto const e0 = a_p1 - a_p0;
		auto const e1 = a_p2 - a_p1;
		auto const e2 = a_p0 - a_p2;

		std::array<glm::vec3, 9> axes = {
			glm::vec3{0.0f, -e0.z, e0.y}, glm::vec3{0.0f, -e1.z, e1.y}, glm::vec3{0.0f, -e2.z, e2.y},
			glm::vec3{e0.z, 0.0f, -e0.x}, glm::vec3{e1.z, 0.0f, -e1.z}, glm::vec3{e2.z, 0.0f, -e2.x},
			glm::vec3{-e0.y, e0.x, 0.0f}, glm::vec3{-e1.y, e1.x, 0.0f}, glm::vec3{-e2.y, e2.x, 0.0f}
		};
		for (int32_t i = 0; i < 9; ++i)
		{
			glm::vec3 axis = axes[i];
			float len = glm::length(axis);
			if (len < std::numeric_limits<float>::epsilon())
			{
				continue;
			}
			axis /= len;

			// project box
			float r = a_halfExtents.x * std::abs(axis.x) + a_halfExtents.y * std::abs(axis.y) + a_halfExtents.z * std::abs(axis.z);

			float p0 = glm::dot(a_p0, axis);
			float p1 = glm::dot(a_p1, axis);
			float p2 = glm::dot(a_p2, axis);

			float minP = std::min({ p0, p1, p2 });
			float maxP = std::max({ p0, p1, p2 });

			if (minP > r || maxP < -r)
			{
				return false;
			}
		}

		for (int i = 0; i < 3; ++i)
		{
			float minT = std::min({ a_p0[i], a_p1[i], a_p2[i] });
			float maxT = std::max({ a_p0[i], a_p1[i], a_p2[i] });
			if (minT > a_halfExtents[i] || maxT < -a_halfExtents[i])
			{
				return false;
			}
		}

		glm::vec3 normal = glm::cross(e0, e1);
		float lenN = glm::length(normal);
		if (lenN < std::numeric_limits<float>::epsilon())
		{
			return false;
		}
		normal /= lenN;

		float d = glm::dot(normal, a_p0);
		float r = a_halfExtents.x * std::abs(normal.x) + a_halfExtents.y * std::abs(normal.y) + a_halfExtents.z * std::abs(normal.z);

		if (std::abs(d) > r)
		{
			return false;
		}

		return true;
	}

	// TODO: rename, it's intersection between some obb and a triangle.
	inline bool test_obb_intersection(glm::vec3 const& a_position, glm::vec3 const& a_halfExtents, glm::quat const& a_rotationInv, triangle const& a_triangle)
	{
		auto const p0 = a_rotationInv * (a_triangle.p0 - a_position);
		auto const p1 = a_rotationInv * (a_triangle.p1 - a_position);
		auto const p2 = a_rotationInv * (a_triangle.p2 - a_position);

		return test_aabb_intersection(a_halfExtents, p0, p1, p2);
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

	inline glm::vec3 findClosestTrianglePoint(triangle const& a_triangle, glm::vec3 const& a_point)
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

	inline glm::vec3 findClosestUnitEllipsoidPoint(glm::vec3 const& a_radiuses, glm::vec3 const& a_point)
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

	inline intersection_result intersectUnitEllipsoidWithTriangle(
		glm::vec3 const& a_radiuses,
		triangle const& a_triangle,
		int32_t& a_expensiveTestCount)
	{
		auto const triangleUp = glm::cross(a_triangle.p1 - a_triangle.p0, a_triangle.p2 - a_triangle.p0);
		if (glm::dot(triangleUp, -a_triangle.p0) < 0.0f)
		{
			return {};
		}
		auto const normal = glm::normalize(triangleUp);

		auto const normalEllipsoidDir = normal * a_radiuses * a_radiuses;
		auto const normalEllipsoidPoint = -normalEllipsoidDir / std::sqrt(glm::dot(normalEllipsoidDir, normal));
		auto const normalDistance = glm::dot(normalEllipsoidPoint - a_triangle.p0, normal);
		auto const normalTrianglePoint = normalEllipsoidPoint - normalDistance * normal;

		auto const d0 = glm::dot(glm::cross(a_triangle.p1 - a_triangle.p0, normalTrianglePoint - a_triangle.p0), normal);
		auto const d1 = glm::dot(glm::cross(a_triangle.p2 - a_triangle.p1, normalTrianglePoint - a_triangle.p1), normal);
		auto const d2 = glm::dot(glm::cross(a_triangle.p0 - a_triangle.p2, normalTrianglePoint - a_triangle.p2), normal);
		if (d0 * d1 <= 0.0f || d1 * d2 <= 0.0f)
		{
			return {};
		}

		++a_expensiveTestCount;

		auto const trianglePoint = findClosestTrianglePoint(a_triangle, normalTrianglePoint);
		auto const ellipsoidPoint = findClosestUnitEllipsoidPoint(a_radiuses, trianglePoint);
		auto const distance = glm::length(trianglePoint - ellipsoidPoint);
		auto const t = trianglePoint / a_radiuses;
		return intersection_result{ ellipsoidPoint, trianglePoint, glm::dot(t, t) < 1.0f ? -distance : distance };
	}



	inline intersection_result intersectEllipsoidWithTriangle(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		glm::quat const& a_rotationInv,
		glm::vec3 const& a_radiuses,
		triangle const& a_triangle,
		int32_t* a_expensiveTestCount = nullptr)
	{
		int32_t dummy;
		if (a_expensiveTestCount == nullptr)
		{
			a_expensiveTestCount = &dummy;
		}

		auto intersection = intersectUnitEllipsoidWithTriangle(
			a_radiuses,
			triangle{
				a_rotationInv * (a_triangle.p0 - a_position),
				a_rotationInv * (a_triangle.p1 - a_position),
				a_rotationInv * (a_triangle.p2 - a_position)
			},
			*a_expensiveTestCount);

		auto const ellipsoidPoint = a_position + a_rotation * intersection.firstPoint;
		auto const trianglePoint = a_position + a_rotation * intersection.secondPoint;
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
		int32_t dummy;
		auto intersectionResult = intersectUnitEllipsoidWithTriangle(a_radiuses, a_triangle, dummy);
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
