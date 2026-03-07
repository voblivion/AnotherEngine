#pragma once

#include <vob/aoe/physics/shapes.h>

#include <vob/aoe/spacetime/transform.h>

#include <vob/misc/std/bounded_vector.h>

#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtx/norm.hpp>


#pragma optimize("", off)
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

	inline glm::mat3 abs(glm::mat3 const& a_matrix)
	{
		return glm::mat3{
			glm::abs(a_matrix[0]),
			glm::abs(a_matrix[1]),
			glm::abs(a_matrix[2])
		};
	}

	inline Aabb computeBounds(glm::vec3 const& a_position, glm::quat const& a_rotation, glm::vec3 const& a_localHalfExtents)
	{
		auto const rotationMatrix = glm::mat3_cast(a_rotation);
		auto const halfExtents = abs(rotationMatrix) * a_localHalfExtents;
		return Aabb{ a_position - halfExtents, a_position + halfExtents };
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

	inline glm::vec3 findClosestTrianglePoint(triangle const& a_triangle, glm::vec3 const& a_point, glm::bvec3& o_boundingSides)
	{
		auto const v01 = a_triangle.p1 - a_triangle.p0;
		auto const v02 = a_triangle.p2 - a_triangle.p0;
		auto const v0p = a_point - a_triangle.p0;

		auto const d1 = glm::dot(v01, v0p);
		auto const d2 = glm::dot(v02, v0p);
		if (d1 <= 0.0f && d2 <= 0.0f)
		{
			o_boundingSides = { true, false, true };
			return a_triangle.p0;
		}

		auto const v13 = a_point - a_triangle.p1;
		auto const d3 = glm::dot(v01, v13);
		auto const d4 = glm::dot(v02, v13);
		if (d3 >= 0.0f && d4 <= d3)
		{
			o_boundingSides = { true, true, false };
			return a_triangle.p1;
		}

		auto const v2 = d1 * d4 - d2 * d3;
		if (v2 <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
		{
			o_boundingSides = { true, false, false };
			return a_triangle.p0 + v01 * d1 / (d1 - d3);
		}

		auto const v2p = a_point - a_triangle.p2;
		auto const d5 = glm::dot(v01, v2p);
		auto const d6 = glm::dot(v02, v2p);
		if (d6 >= 0.0f && d5 <= d6)
		{
			o_boundingSides = { false, true, true };
			return a_triangle.p2;
		}

		auto const v1 = d2 * d5 - d1 * d6;
		if (v1 <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
		{
			o_boundingSides = { false, false, true };
			return a_triangle.p0 + v02 * d2 / (d2 - d6);
		}

		auto const v0 = d3 * d6 - d4 * d5;
		if (v0 <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
		{
			o_boundingSides = { false, true, false };
			return a_triangle.p1 + (a_triangle.p2 - a_triangle.p1) * (d4 - d3) / ((d4 - d3) + (d5 - d6));
		}

		//auto const d01Sq = glm::dot(v01, v01);
		//auto const d02Sq = glm::dot(v02, v02);
		//auto const d0102 = glm::dot(v01, v02);

		//auto const d = d01Sq * d02Sq - d0102 * d0102;
		//auto const v = (d02Sq * glm::dot(v0p, v01) - d0102 * glm::dot(v0p, v02)) / d;
		//auto const w = (d01Sq * glm::dot(v0p, v02) - d0102 * glm::dot(v0p, v01)) / d;

		o_boundingSides = { false, false, false };
		//return a_triangle.p0 + v01 * v + v02 * w;
		return a_point;
	}

	template <typename TFunc>
	float findZero(TFunc a_func, float a_t0, float a_t1, float a_epsilon = 0.00001f)
	{
		while (std::abs(a_t1 - a_t0) > a_epsilon)
		{
			const auto t = (a_t0 + a_t1) * 0.5f;
			if (a_func(t) < 0.0f)
			{
				a_t0 = t;
			}
			else
			{
				a_t1 = t;
			}
		}
		return (a_t0 + a_t1) * 0.5f;
	}

	inline mistd::bounded_vector<glm::vec3, 6> findClosestUnitEllipsoidPointCandidates(glm::vec3 const& a_radiuses, glm::vec3 const& a_point)
	{
		auto const r2 = a_radiuses * a_radiuses;
		auto const f = [&a_radiuses, &a_point, &r2](float lambda)
			{
				auto const d = a_point / (a_radiuses * (1.0f + lambda / r2));
				return glm::dot(d, d) - 1.0f;
			};
		auto const fNeg = [f](float source, float initialOffset)
			{
				auto t = source + initialOffset;
				while (f(t) > 0.0f)
				{
					t += (t - source);
				}
				return t;
			};

		auto const df = [&a_radiuses, &a_point, &r2](float lambda)
			{
				auto const e = (1.0f + lambda / r2);
				auto const d = a_point * a_point / (r2 * r2 * e * e * e);
				return -2.0f * (d.x + d.y + d.z);
			};
		auto const dfZero = [&df](float t0, float t1)
			{
				auto s0 = (t0 + t1) * 0.5f;
				while (df(s0) > 0.0f && std::abs(s0 - t0) > 0.00001f)
				{
					s0 = (t0 + s0) * 0.5f;
				}
				auto s1 = (t0 + t1) * 0.5f;
				while (df(s1) < 0.0f && std::abs(s1 - t1) > 0.00001f)
				{
					s1 = (s1 + t1) * 0.5f;
				}
				return findZero(df, s0, s1);
			};
		auto const toPoint = [&a_point, &r2](float k)
			{
				return a_point / (1.0f + k / r2);
			};


		std::array<float, 3> bkrs = { -r2.x, -r2.y, -r2.z };
		std::sort(bkrs.begin(), bkrs.end());
		auto const bk1 = bkrs[0];
		auto const bk3 = bkrs[1];
		auto const bk5 = bkrs[2];
		auto const bk0 = fNeg(bk1, -1.0f);
		auto const bk6 = fNeg(bk5, 1.0f);

		mistd::bounded_vector<glm::vec3, 6> candidates;
		candidates.push_back(toPoint(findZero(f, bk0, bk1)));
		if (glm::epsilonNotEqual(bk1, bk3, 0.00001f))
		{
			auto const bk2 = dfZero(bk1, bk3);
			if (f(bk2) < 0.0f)
			{
				candidates.push_back(toPoint(findZero(f, bk2, bk1)));
				candidates.push_back(toPoint(findZero(f, bk2, bk3)));
			}
		}
		if (glm::epsilonNotEqual(bk3, bk5, 0.00001f))
		{
			auto const bk4 = dfZero(bk3, bk5);
			if (f(bk4) < 0.0f)
			{
				candidates.push_back(toPoint(findZero(f, bk4, bk3)));
				candidates.push_back(toPoint(findZero(f, bk4, bk5)));
			}
		}
		candidates.push_back(toPoint(findZero(f, bk6, bk5)));

		return candidates;
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

		/*auto const d0 = glm::dot(glm::cross(a_triangle.p1 - a_triangle.p0, normalTrianglePoint - a_triangle.p0), normal);
		auto const d1 = glm::dot(glm::cross(a_triangle.p2 - a_triangle.p1, normalTrianglePoint - a_triangle.p1), normal);
		auto const d2 = glm::dot(glm::cross(a_triangle.p0 - a_triangle.p2, normalTrianglePoint - a_triangle.p2), normal);
		if (d0 * d1 <= 0.0f || d1 * d2 <= 0.0f)
		{
			return {};
		}*/

		glm::bvec3 boundingSegments = {};
		auto const trianglePoint = findClosestTrianglePoint(a_triangle, normalTrianglePoint, boundingSegments);
		auto const t = trianglePoint / a_radiuses;
		if (glm::dot(t, t) >= 1.0f)
		{
			return intersection_result{ normalEllipsoidPoint, trianglePoint, 1.0f };
		}

		if (!glm::any(boundingSegments))
		{
			auto const distance = glm::length(trianglePoint - normalEllipsoidPoint);
			return intersection_result{ normalEllipsoidPoint, trianglePoint, -distance };
		}

		++a_expensiveTestCount;

		auto const ellipsoidPointCandidates = findClosestUnitEllipsoidPointCandidates(a_radiuses, trianglePoint);
		auto ellipsoidPoint = normalEllipsoidPoint;
		auto smallestDistance2 = std::numeric_limits<float>::max();
		auto const outsideSegment0 = glm::cross(a_triangle.p1 - a_triangle.p0, normal);
		auto const outsideSegment1 = glm::cross(a_triangle.p2 - a_triangle.p1, normal);
		auto const outsideSegment2 = glm::cross(a_triangle.p0 - a_triangle.p2, normal);
		for (auto const& ellipsoidPointCandidate : ellipsoidPointCandidates)
		{
			if (glm::dot(normal, ellipsoidPointCandidate - trianglePoint) > 0.0f)
			{
				continue;
			}
			if (boundingSegments.x && glm::dot(outsideSegment0, ellipsoidPointCandidate - trianglePoint) > 0.0f)
			{
				continue;
			}
			if (boundingSegments.y && glm::dot(outsideSegment1, ellipsoidPointCandidate - trianglePoint) > 0.0f)
			{
				continue;
			}
			if (boundingSegments.z && glm::dot(outsideSegment2, ellipsoidPointCandidate - trianglePoint) > 0.0f)
			{
				continue;
			}

			auto const distance2 = glm::length2(ellipsoidPointCandidate - trianglePoint);
			if (distance2 < smallestDistance2)
			{
				smallestDistance2 = distance2;
				ellipsoidPoint = ellipsoidPointCandidate;
			}
		}

		auto const distance = std::sqrt(smallestDistance2);
		return intersection_result{ ellipsoidPoint, trianglePoint, -distance };
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
		auto const distance = glm::length(ellipsoidPoint - trianglePoint);
		return intersection_result{ ellipsoidPoint, trianglePoint, intersection.signedDistance < 0.0f ? -distance : distance };
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
