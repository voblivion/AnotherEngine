#include <vob/aoe/physics/systems/test_system.h>

#include <vob/aoe/physics/debug_drawer.h>

#include <vob/aoe/rendering/camera_util.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_cross_product.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "imgui.h"

#include "bullet/BulletDynamics/Dynamics/btRigidBody.h"
#include "bullet/BulletDynamics/Dynamics/btSimpleDynamicsWorld.h"

namespace ImGui
{
	static void Vector(glm::vec2 a_vector)
	{
		ImGui::Text("%.3f, %.3f", a_vector.x, a_vector.y);
	}

	static void Vector(glm::vec3 a_vector)
	{
		ImGui::Text("%.3f, %.3f, %.3f", a_vector.x, a_vector.y, a_vector.z);
	}
}

glm::vec3 cleanup(glm::vec4 const& a_vector)
{
	return glm::vec3{ a_vector / a_vector.w };
}

namespace vob::aoeph
{
#pragma region Structures
	struct segment
	{
		glm::vec3 m_origin;
		glm::vec3 m_displacement;
	};

	struct plane
	{
		glm::vec3 m_origin;
		glm::vec3 m_normal;
	};

	struct triangle
	{
		glm::vec3 m_p0;
		glm::vec3 m_p1;
		glm::vec3 m_p2;
	};
#pragma endregion

#pragma region Intersection
	static inline float intersection_time(glm::vec3 const& a_point, glm::vec3 const& a_displacement, plane const& a_plane)
	{
		return -glm::dot(a_point - a_plane.m_origin, a_plane.m_normal) / glm::dot(a_displacement, a_plane.m_normal);
	}

	static inline glm::vec3 unit_sphere_intersection_point(glm::vec3 const& a_displacement, plane const& a_plane)
	{
		auto const spherePoint = -a_plane.m_normal;
		auto const t = intersection_time(-a_plane.m_normal, a_displacement, a_plane);
		return spherePoint + t * a_displacement;
	}

	static inline bool is_inside(glm::vec3 const& a_point, triangle const& a_triangle)
	{
		auto const normal = glm::cross(a_triangle.m_p1 - a_triangle.m_p0, a_triangle.m_p2 - a_triangle.m_p0);
		auto const d0 = glm::dot(glm::cross(a_triangle.m_p1 - a_triangle.m_p0, a_point - a_triangle.m_p0), normal);
		auto const d1 = glm::dot(glm::cross(a_triangle.m_p2 - a_triangle.m_p1, a_point - a_triangle.m_p1), normal);
		auto const d2 = glm::dot(glm::cross(a_triangle.m_p0 - a_triangle.m_p2, a_point - a_triangle.m_p2), normal);
		return d0 * d1 > 0.0f && d1 * d2 > 0.0f;
	}

	static inline float unit_sphere_intersection_time(glm::vec3 const& a_point, glm::vec3 const& a_displacement)
	{
		auto const a = glm::dot(a_displacement, a_displacement);
		auto const b = 2.0f * glm::dot(a_displacement, a_point);
		auto const c = glm::dot(a_point, a_point) - 1.0f;
		auto const delta = b * b - 4 * a * c;
		if (delta < 0.0f)
		{
			return 1.0f;
		}
		return (-b - std::sqrt(delta)) / (2.0f * a);
	}

	static inline std::pair<float, glm::vec3> unit_sphere_cast(glm::vec3 const& a_displacement, segment const& a_segment)
	{
		// 1. segment equation: P = s.origin + t * s.displacement
		// 2. sphere equation: (Px-displacementx)^2 + (Py-displacementy)^2 + (Pz-displacementz)^2 = 1
		// 3. inject 1 into 2
		// 4. intersection => 1 solution in t => b^2 - 4*a*c = 0
		// 5. solve b^2 - 4 * a * c for u

		auto const M = glm::dot(a_segment.m_origin, a_segment.m_displacement);
		auto const N = glm::dot(a_displacement, a_segment.m_displacement);
		auto const I = glm::dot(a_segment.m_origin, a_segment.m_origin) - 1.0f;
		auto const J = 2.0f * glm::dot(a_segment.m_origin, a_displacement);
		auto const K = glm::dot(a_displacement, a_displacement);
		auto const Q = glm::dot(a_segment.m_displacement, a_segment.m_displacement);
		auto const A = N * N - K * Q;
		auto const B = J * Q - 2 * M * N;
		auto const C = M * M - I * Q;
		auto const D = B * B - 4 * A * C;

		if (D < 0.0f)
		{
			return { 1.0f, a_displacement };
		}

		auto const U0 = (-B + std::sqrt(D)) / (2 * A);
		auto const W0 = a_segment.m_origin - U0 * a_displacement;
		auto const aa0 = glm::dot(a_segment.m_displacement, a_segment.m_displacement);
		auto const bb0 = -2.0f * glm::dot(W0, a_segment.m_displacement);
		auto const cc0 = glm::dot(W0, W0) - 1.0f;
		auto const tt0 = std::clamp(bb0 / (2 * aa0), 0.0f, 1.0f);
		auto const segmentPoint0 = a_segment.m_origin + tt0 * a_segment.m_displacement;
		auto const t0 = unit_sphere_intersection_time(segmentPoint0, -a_displacement);

		auto const U1 = (-B - std::sqrt(D)) / (2 * A);
		auto const W1 = a_segment.m_origin - U1 * a_displacement;
		auto const aa1 = glm::dot(a_segment.m_displacement, a_segment.m_displacement);
		auto const bb1 = -2.0f * glm::dot(W1, a_segment.m_displacement);
		auto const cc1 = glm::dot(W1, W1) - 1.0f;
		auto const tt1 = std::clamp(bb1 / (2 * aa1), 0.0f, 1.0f);
		auto const segmentPoint1 = a_segment.m_origin + tt1 * a_segment.m_displacement;
		auto const t1 = unit_sphere_intersection_time(segmentPoint1, -a_displacement);

		return { t0, segmentPoint0 };
	}

	static inline std::pair<float, glm::vec3> unit_sphere_cast(glm::vec3 const& a_displacement, triangle const& a_triangle)
	{
		auto const planeNormal = glm::normalize(glm::cross(a_triangle.m_p1 - a_triangle.m_p0, a_triangle.m_p2 - a_triangle.m_p0));
		auto const planePoint = unit_sphere_intersection_point(a_displacement, plane{ a_triangle.m_p0, planeNormal });
		if (is_inside(planePoint, a_triangle))
		{
			auto const t = unit_sphere_intersection_time(planePoint, -a_displacement);
			return { t, planePoint };
		}

		auto const [t0, p0] = unit_sphere_cast(a_displacement, segment{ a_triangle.m_p0, a_triangle.m_p1 - a_triangle.m_p0 });
		auto const [t1, p1] = unit_sphere_cast(a_displacement, segment{ a_triangle.m_p1, a_triangle.m_p2 - a_triangle.m_p1 });
		auto const [t2, p2] = unit_sphere_cast(a_displacement, segment{ a_triangle.m_p2, a_triangle.m_p0 - a_triangle.m_p2 });

		return t0 < t1 ? (t0 < t2 ? std::pair{ t0, p0 } : std::pair{ t2, p2 }) : (t1 < t2 ? std::pair{ t1, p1 } : std::pair{ t2, p2 });
	}

	static inline std::pair<float, glm::vec3> ellipsoid_cast(glm::mat4 const& a_transform, glm::mat4 const& a_invTransform, glm::vec3 const& a_move, triangle const& a_triangle)
	{
		auto [hitTime, hitPos] = unit_sphere_cast(
			glm::vec3{ a_invTransform * glm::vec4{ a_move, 0.0f } },
			triangle{
				cleanup(a_invTransform * glm::vec4{a_triangle.m_p0, 1.0f}),
				cleanup(a_invTransform * glm::vec4{a_triangle.m_p1, 1.0f}),
				cleanup(a_invTransform * glm::vec4{a_triangle.m_p2, 1.0f})
			});
		return { hitTime, cleanup(a_transform * glm::vec4{hitPos, 1.0f}) };
	}

	static inline bool unit_sphere_intersect_test(plane const& a_plane)
	{
		return glm::dot(-a_plane.m_origin, a_plane.m_normal) * glm::dot(-a_plane.m_origin - a_plane.m_normal, a_plane.m_normal) < 0.0f;
	}

	static inline std::tuple<glm::vec3, float, float, float> origin_closest_point(segment const& a_segment)
	{
		auto const t = -glm::dot(a_segment.m_origin, a_segment.m_displacement) / glm::dot(a_segment.m_displacement, a_segment.m_displacement);
		auto const originClosestPoint = a_segment.m_origin + glm::clamp(t, 0.0f, 1.0f) * a_segment.m_displacement;
		
		auto const a = glm::dot(a_segment.m_displacement, a_segment.m_displacement);
		auto const b = 2.0f * glm::dot(a_segment.m_origin, a_segment.m_displacement);
		auto const c = glm::dot(a_segment.m_origin, a_segment.m_origin) - 1.0f;
		auto const d = b * b - 4.0f * a * c;
		if (d <= 0.0f)
		{
			return { originClosestPoint, d, 0.0f, 1.0f };
		}

		auto const sqrtD = std::sqrt(d);
		auto const r1 = (-b - sqrtD) / (2.0f * a);
		auto const r2 = (-b + sqrtD) / (2.0f * a);
		return { originClosestPoint, d, r1, r2 };
	}

	float triangle_surface(glm::vec3 const& x, glm::vec3 const& y, glm::vec3 const& z)
	{
		auto const r0 = glm::length(x - y);
		auto const r1 = glm::length(z - y);

		auto const d = glm::dot((x - y) / r0, z - y);
		return r0 * glm::sqrt(r1 * r1 - d * d);
	}

	float origin_angle_between(glm::vec3 const& x, glm::vec3 const& y)
	{
		return std::atan2(glm::length(glm::cross(x, y)), glm::dot(x, y));
	}

	float origin_triangle_surface(glm::vec3 const& x, glm::vec3 const& y)
	{
		auto const d = glm::dot(x, y);
		return std::sqrt(std::max(0.0f, 1.0f - d * d));
	}

	float unit_circle_triangle_edge_uncovered_surface_ratio(glm::vec3 const& i0, glm::vec3 const& i1, glm::vec3 const& z)
	{
		auto const arcSurface = origin_angle_between(i0, i1);
		auto const triangleSurface = origin_triangle_surface(i0, i1);
		if (glm::dot(glm::cross(z - i0, i1 - i0), glm::cross(-i0, i1 - i0)) > 0.0f)
		{
			return (arcSurface - triangleSurface) / (2.0f * std::numbers::pi_v<float>);
		}
		return (2.0f * std::numbers::pi_v<float> + triangleSurface - arcSurface) / (2.0f * std::numbers::pi_v<float>);
	}

	float unit_circle_triangle_corner_uncovered_surface_ratio(glm::vec3 const& x, glm::vec3 const& y, glm::vec3 const& z)
	{
		auto const arcSurface = origin_angle_between(x, y);
		auto const arcTriangleSurface = origin_triangle_surface(x, y);
		auto const triangleSurface = triangle_surface(x, y, z);
		auto const cornerSurface = arcSurface - arcTriangleSurface + triangleSurface;

		return cornerSurface / (2.0f * std::numbers::pi_v<float>);
	}

	float circle_triangle_edge_uncovered_surface_ratio(glm::vec3 const& c0, glm::vec3 const& i0, glm::vec3 const& i1, glm::vec3 const& z)
	{
		auto const r = glm::length(i0 - c0);
		return unit_circle_triangle_edge_uncovered_surface_ratio((i0 - c0) / r, (i1 - c0) / r, (z - c0) / r);
	}

	float circle_triangle_corner_uncovered_surface_ratio(glm::vec3 const& c0, glm::vec3 const& i0, glm::vec3 const& i1, glm::vec3 const& z)
	{
		auto const r = glm::length(i0 - c0);
		return unit_circle_triangle_corner_uncovered_surface_ratio((i0 - c0) / r, (i1 - c0) / r, (z - c0) / r);
	}

	static inline std::tuple<glm::vec3, glm::vec3, float> unit_sphere_intersect(triangle const& a_triangle)
	{
		auto const planeNormal = glm::normalize(glm::cross(a_triangle.m_p1 - a_triangle.m_p0, a_triangle.m_p2 - a_triangle.m_p0));
		auto const planeDist = glm::dot(-planeNormal, a_triangle.m_p0 + planeNormal);
		auto const planePoint = -planeNormal * (1 + planeDist);
		auto const [p0, delta0, r00, r01] = origin_closest_point(segment{ a_triangle.m_p0, a_triangle.m_p1 - a_triangle.m_p0 });
		auto const [p1, delta1, r10, r11] = origin_closest_point(segment{ a_triangle.m_p1, a_triangle.m_p2 - a_triangle.m_p1 });
		auto const [p2, delta2, r20, r21] = origin_closest_point(segment{ a_triangle.m_p2, a_triangle.m_p0 - a_triangle.m_p2 });
		auto const d0Sq = glm::dot(p0, p0);
		auto const d1Sq = glm::dot(p1, p1);
		auto const d2Sq = glm::dot(p2, p2);

		auto const T0 = a_triangle.m_p0;
		auto const T1 = a_triangle.m_p1;
		auto const T2 = a_triangle.m_p2;
		auto const i00 = T0 + (T1 - T0) * r00;
		auto const i01 = T0 + (T1 - T0) * r01;
		auto const i10 = T1 + (T2 - T1) * r10;
		auto const i11 = T1 + (T2 - T1) * r11;
		auto const i20 = T2 + (T0 - T2) * r20;
		auto const i21 = T2 + (T0 - T2) * r21;
		auto const C0 = planePoint;
		auto const r = planeDist < 1.0f ? std::sqrt(1.0f - planeDist * planeDist) : 1.0f;

		auto e0 = delta0 > 0.0f ? circle_triangle_edge_uncovered_surface_ratio(C0, i00, i01, T2) : 0.0f;
		auto e1 = delta1 > 0.0f ? circle_triangle_edge_uncovered_surface_ratio(C0, i10, i11, T0) : 0.0f;
		auto e2 = delta2 > 0.0f ? circle_triangle_edge_uncovered_surface_ratio(C0, i20, i21, T1) : 0.0f;
		auto f0 = r01 > 1.0f && r10 < 0.0f ? circle_triangle_corner_uncovered_surface_ratio(C0, i01, i10, T1) : 0.0f;
		auto f1 = r11 > 1.0f && r20 < 0.0f ? circle_triangle_corner_uncovered_surface_ratio(C0, i11, i20, T2) : 0.0f;
		auto f2 = r21 > 1.0f && r00 < 0.0f ? circle_triangle_corner_uncovered_surface_ratio(C0, i21, i00, T0) : 0.0f;
		auto const coverRatio = std::clamp(1.0f - e0 - e1 - e2 + f0 + f1 + f2, 0.0f, 1.0f);

		if (is_inside(planePoint, a_triangle))
		{
			return { planePoint , -planeNormal, coverRatio };
		}

		auto result = [&](auto const& p, auto const dSq)
			{
				auto const d = std::sqrt(dSq);
				auto const n = (d == 0.0f) ? -planeNormal : p / d;
				return std::make_tuple(p, n, coverRatio);
			};

		return d0Sq < d1Sq ? (d0Sq < d2Sq ? result(p0, d0Sq) : result(p2, d2Sq)) : (d1Sq < d2Sq ? result(p1, d1Sq) : result(p2, d2Sq));
	}

	static inline std::tuple<float, glm::vec3, glm::vec3, float> ellipsoid_intersect(glm::mat4 const& a_transform, glm::mat4 const& a_invTransform, triangle const& a_triangle)
	{
		auto const [sphereTrianglePos, spherePos, coverRatio] = unit_sphere_intersect(
			triangle{
				cleanup(a_invTransform * glm::vec4{a_triangle.m_p0, 1.0f}),
				cleanup(a_invTransform * glm::vec4{a_triangle.m_p1, 1.0f}),
				cleanup(a_invTransform * glm::vec4{a_triangle.m_p2, 1.0f})
			});

		auto const trianglePos = cleanup(a_transform * glm::vec4{ sphereTrianglePos, 1.0f });
		auto const ellipsoidPos = cleanup(a_transform * glm::vec4{ spherePos, 1.0f });
		auto const planeNormal = glm::normalize(glm::cross(a_triangle.m_p1 - a_triangle.m_p0, a_triangle.m_p2 - a_triangle.m_p0));

		auto const dist = glm::dot(planeNormal, ellipsoidPos - a_triangle.m_p0);

		

		return { glm::dot(planeNormal, aoest::get_position(a_transform) - a_triangle.m_p0) < 0.0f ? -dist : dist, ellipsoidPos - dist * planeNormal, ellipsoidPos, coverRatio};
	}

	static inline std::tuple<float, glm::vec3, glm::vec3> ellipsoid_intersect2(glm::mat4 const& a_transform, glm::mat4 const& a_invTransform, triangle const& a_triangle)
	{
		auto const [sphereTrianglePos, spherePos, coverRatio] = unit_sphere_intersect(
			triangle{
				cleanup(a_invTransform * glm::vec4{a_triangle.m_p0, 1.0f}),
				cleanup(a_invTransform * glm::vec4{a_triangle.m_p1, 1.0f}),
				cleanup(a_invTransform * glm::vec4{a_triangle.m_p2, 1.0f})
			});

		auto const trianglePos = cleanup(a_transform * glm::vec4{ sphereTrianglePos, 1.0f });
		auto const ellipsoidPos = cleanup(a_transform * glm::vec4{ spherePos, 1.0f });

		auto const planeNormal = glm::normalize(glm::cross(a_triangle.m_p1 - a_triangle.m_p0, a_triangle.m_p2 - a_triangle.m_p0));

		auto const dist = glm::length(trianglePos - ellipsoidPos);

		auto const isInside = glm::dot(planeNormal, aoest::get_position(a_transform) - a_triangle.m_p0) > 0.0f
			&& glm::dot(sphereTrianglePos, sphereTrianglePos) < 1.0f;

		return { isInside ? -dist : dist, ellipsoidPos, trianglePos };
	}

	static inline std::pair<float, glm::vec3> ellipsoid_move_rec(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		glm::vec3 const& a_radiuses,
		glm::vec3 const& a_linearMove,
		glm::vec3 const& a_angularMove,
		triangle const& a_triangle,
		std::pair<float, float> const& a_timeRange)
	{
		if (a_timeRange.second - a_timeRange.first < 0.0001f)
		{
			auto const position = a_position + a_linearMove * a_timeRange.first;
			auto const rotation = a_rotation * glm::quat{ a_angularMove * a_timeRange.first };
			auto const transform = glm::scale(aoest::combine(position, rotation), a_radiuses);
			auto const invTransform = glm::inverse(transform);

			auto const [point, normal, coverRatio] = unit_sphere_intersect(
				triangle{
					cleanup(invTransform * glm::vec4{a_triangle.m_p0, 1.0f}),
					cleanup(invTransform * glm::vec4{a_triangle.m_p1, 1.0f}),
					cleanup(invTransform * glm::vec4{a_triangle.m_p2, 1.0f})
				});

			auto const p = cleanup(transform * glm::vec4{ point, 1.0f });
			return { a_timeRange.first, p };
		}

		auto const midTime = (a_timeRange.first + a_timeRange.second) * 0.5f;

		auto const midRotation = a_rotation * glm::quat{ a_angularMove * midTime };
		auto const startTransform = glm::scale(aoest::combine(a_position, midRotation), a_radiuses);
		auto const invStartTransform = glm::inverse(startTransform);
		auto const [dist, startEllipsoidPos, startTrianglePos, coverRatio] = ellipsoid_intersect(startTransform, invStartTransform, a_triangle);
		if (dist < 0.0f)
		{
			return ellipsoid_move_rec(a_position, a_rotation, a_linearMove, a_angularMove, a_radiuses, a_triangle, { a_timeRange.first, midTime });
		}

		auto const [maxLinearMoveRatio, maxEllipsoidPos] = ellipsoid_cast(startTransform, invStartTransform, a_linearMove, a_triangle);
		if (maxLinearMoveRatio < midTime)
		{
			return ellipsoid_move_rec(a_position, a_rotation, a_linearMove, a_angularMove, a_radiuses, a_triangle, { a_timeRange.first, midTime });
		}
		else
		{
			return ellipsoid_move_rec(a_position, a_rotation, a_linearMove, a_angularMove, a_radiuses, a_triangle, { midTime, a_timeRange.second });
		}
	}

	static inline std::pair<float, glm::vec3> ellipsoid_move_rec2(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		glm::vec3 const& a_radiuses,
		glm::vec3 const& a_linearMove,
		glm::vec3 const& a_angularMove,
		triangle const& a_triangle,
		std::pair<float, float> const& a_timeRange)
	{
		if (a_timeRange.second - a_timeRange.first < 0.0001f)
		{
			auto const position = a_position + a_linearMove * a_timeRange.first;
			auto const rotation = a_rotation * glm::quat{ a_angularMove * a_timeRange.first };
			auto const transform = glm::scale(aoest::combine(position, rotation), a_radiuses);
			auto const invTransform = glm::inverse(transform);

			auto const [point, normal, coverRatio] = unit_sphere_intersect(
				triangle{
					cleanup(invTransform * glm::vec4{a_triangle.m_p0, 1.0f}),
					cleanup(invTransform * glm::vec4{a_triangle.m_p1, 1.0f}),
					cleanup(invTransform * glm::vec4{a_triangle.m_p2, 1.0f})
				});

			auto const p = cleanup(transform * glm::vec4{ point, 1.0f });
			return { a_timeRange.first, p };
		}

		auto const midTime = (a_timeRange.first + a_timeRange.second) * 0.5f;

		auto const midRotation = a_rotation * glm::quat{ a_angularMove * midTime };
		auto const startTransform = glm::scale(aoest::combine(a_position, midRotation), a_radiuses);
		auto const invStartTransform = glm::inverse(startTransform);
		auto const [dist, startEllipsoidPos, startTrianglePos] = ellipsoid_intersect2(startTransform, invStartTransform, a_triangle);
		if (dist < 0.0f)
		{
			return ellipsoid_move_rec2(a_position, a_rotation, a_linearMove, a_angularMove, a_radiuses, a_triangle, { a_timeRange.first, midTime });
		}

		auto const [maxLinearMoveRatio, maxEllipsoidPos] = ellipsoid_cast(startTransform, invStartTransform, a_linearMove, a_triangle);
		if (maxLinearMoveRatio < midTime)
		{
			return ellipsoid_move_rec2(a_position, a_rotation, a_linearMove, a_angularMove, a_radiuses, a_triangle, { a_timeRange.first, midTime });
		}
		else
		{
			return ellipsoid_move_rec2(a_position, a_rotation, a_linearMove, a_angularMove, a_radiuses, a_triangle, { midTime, a_timeRange.second });
		}
	}

	static inline std::pair<float, glm::vec3> ellipsoid_move(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		glm::vec3 const& a_radiuses,
		glm::vec3 const& a_linearMove,
		glm::vec3 const& a_angularMove,
		triangle const& a_triangle)
	{
		auto const startTransform = glm::scale(aoest::combine(a_position, a_rotation), a_radiuses);
		auto const invStartTransform = glm::inverse(startTransform);
		auto const [startDist, startEllipsoidPos, startTrianglePos, coverRatio1] = ellipsoid_intersect(startTransform, invStartTransform, a_triangle);
		if (startDist < 0.0f)
		{
			return { 0.0f, startEllipsoidPos };
		}

		auto const stopTransform = glm::scale(
			aoest::combine(
				a_position + a_linearMove,
				a_rotation * glm::quat{ aoest::combine(a_position, a_rotation) * glm::vec4{ a_angularMove, 0.0f } }
			),
			a_radiuses);
		auto const invStopTransform = glm::inverse(stopTransform);
		auto const [stopDist, stopEllipsoidPos, stopTrianglePos, coverRatio2] = ellipsoid_intersect(stopTransform, invStopTransform, a_triangle);
		if (stopDist >= 0.0f && glm::dot(startEllipsoidPos - a_triangle.m_p0, a_triangle.m_p1 - a_triangle.m_p0) * glm::dot(stopEllipsoidPos - a_triangle.m_p0, a_triangle.m_p1 - a_triangle.m_p0))
		{
			return { 1.0f, a_position }; // Position is dummy here
		}

		return ellipsoid_move_rec(a_position, a_rotation, a_linearMove, a_angularMove, a_radiuses, a_triangle, { 0.0f, 1.0f });
	}
	

#pragma endregion

#pragma region Draw
	void draw_line(aoegl::debug_mesh_world_component& a_debugMeshContext, glm::vec3 const& a_p0, glm::vec3 const& a_p1, aoegl::rgba const& a_color)
	{
		a_debugMeshContext.add_line(a_p0, a_p1, a_color);
	}

	void _draw_triangle(aoegl::debug_mesh_world_component& a_debugMeshContext, glm::vec3 const& a_p0, glm::vec3 const& a_p1, glm::vec3 const& a_p2, aoegl::rgba const& a_color)
	{
		draw_line(a_debugMeshContext, a_p0, a_p1, a_color);
		draw_line(a_debugMeshContext, a_p1, a_p2, a_color);

		auto const s01Length = glm::length(a_p1 - a_p0);
		auto const s12Length = glm::length(a_p2 - a_p1);

		auto const subdivisionCount = static_cast<std::int32_t>(std::ceil(s01Length / 5.0f));
		for (auto subdivisionIndex = 0; subdivisionIndex < subdivisionCount; ++subdivisionIndex)
		{
			auto const subdivisionRatio = static_cast<float>(subdivisionIndex) / subdivisionCount;

			draw_line(a_debugMeshContext, a_p2 + (a_p1 - a_p2) * subdivisionRatio, a_p0 + (a_p1 - a_p0) * subdivisionRatio, a_color);
		}
	}

	void _draw_ellipsoid(aoegl::debug_mesh_world_component& a_debugMeshContext, glm::mat4 const& a_transform, glm::vec3 const& a_radiuses, aoegl::rgba const& a_color)
	{
		constexpr auto kHorizontalSlices = 7;
		constexpr auto kHorizontalSubdivisions = 8;
		constexpr auto kVerticalSlices = 8;
		constexpr auto kVerticalSubdivisions = 8;

		for (int h = 0; h < kHorizontalSlices; ++h)
		{
			auto const hSliceAngle0 = (static_cast<float>(h) / (kHorizontalSlices + 1) - 0.5f) * std::numbers::pi_v<float>;
			auto const hSliceAngle1 = (static_cast<float>(h + 1) / (kHorizontalSlices + 1) - 0.5f) * std::numbers::pi_v<float>;
			for (int hs = 0; hs < kHorizontalSubdivisions; ++hs)
			{
				auto const hSubR0 = static_cast<float>(hs) / kHorizontalSubdivisions;
				auto const hSubR1 = static_cast<float>(hs + 1) / kHorizontalSubdivisions;
				auto const hSubAngle0 = hSliceAngle0 + hSubR0 * (hSliceAngle1 - hSliceAngle0);
				auto const hSubAngle1 = hSliceAngle0 + hSubR1 * (hSliceAngle1 - hSliceAngle0);

				auto const y0 = a_radiuses.y * std::sin(hSubAngle0);
				auto const r0 = std::cos(hSubAngle0);
				auto const y1 = a_radiuses.y * std::sin(hSubAngle1);
				auto const r1 = std::cos(hSubAngle1);

				for (int v = 0; v < 2 * kVerticalSlices; ++v)
				{
					auto const vSliceAngle = (static_cast<float>(v) / kVerticalSlices) * std::numbers::pi_v<float>;
					auto const vSliceCos = std::cos(vSliceAngle);
					auto const vSliceSin = std::sin(vSliceAngle);
					auto const localPos0 = glm::vec4{ r0 * a_radiuses.x * vSliceSin, y0, r0 * a_radiuses.z * vSliceCos, 1.0f };
					auto const localPos1 = glm::vec4{ r1 * a_radiuses.x * vSliceSin, y1, r1 * a_radiuses.z * vSliceCos, 1.0f };
					a_debugMeshContext.add_line(cleanup(a_transform * localPos0), cleanup(a_transform * localPos1), a_color);
				}
			}

			for (int v = 0; v < 2 * kVerticalSlices; ++v)
			{
				auto const vSliceAngle0 = (static_cast<float>(v) / kVerticalSlices) * std::numbers::pi_v<float>;
				auto const vSliceAngle1 = (static_cast<float>(v + 1) / kVerticalSlices) * std::numbers::pi_v<float>;

				for (int vs = 0; vs < kVerticalSubdivisions; ++vs)
				{
					auto const r = std::cos(hSliceAngle1);
					auto const y = a_radiuses.y * std::sin(hSliceAngle1);

					auto const vSubR0 = static_cast<float>(vs) / kVerticalSubdivisions;
					auto const vSubR1 = static_cast<float>(vs + 1) / kVerticalSubdivisions;
					auto const vSubAngle0 = vSliceAngle0 + vSubR0 * (vSliceAngle1 - vSliceAngle0);
					auto const vSubAngle1 = vSliceAngle0 + vSubR1 * (vSliceAngle1 - vSliceAngle0);
					
					auto const vSubCos0 = std::cos(vSubAngle0);
					auto const vSubSin0 = std::sin(vSubAngle0);
					auto const vSubCos1 = std::cos(vSubAngle1);
					auto const vSubSin1 = std::sin(vSubAngle1);
					auto const localPos0 = glm::vec4{ r * a_radiuses.x * vSubSin0, y, r * a_radiuses.z * vSubCos0, 1.0f };
					auto const localPos1 = glm::vec4{ r * a_radiuses.x * vSubSin1, y, r * a_radiuses.z * vSubCos1, 1.0f };
					a_debugMeshContext.add_line(cleanup(a_transform * localPos0), cleanup(a_transform * localPos1), a_color);
				}
			}
		}

		auto const hSliceAngle0 = (static_cast<float>(kHorizontalSlices) / (kHorizontalSlices + 1) - 0.5f) * std::numbers::pi_v<float>;
		auto const hSliceAngle1 = (static_cast<float>(kHorizontalSlices + 1) / (kHorizontalSlices + 1) - 0.5f) * std::numbers::pi_v<float>;
		for (int hs = 0; hs < kHorizontalSubdivisions; ++hs)
		{
			auto const hSubR0 = static_cast<float>(hs) / kHorizontalSubdivisions;
			auto const hSubR1 = static_cast<float>(hs + 1) / kHorizontalSubdivisions;
			auto const hSubAngle0 = hSliceAngle0 + hSubR0 * (hSliceAngle1 - hSliceAngle0);
			auto const hSubAngle1 = hSliceAngle0 + hSubR1 * (hSliceAngle1 - hSliceAngle0);

			auto const y0 = a_radiuses.y * std::sin(hSubAngle0);
			auto const r0 = std::cos(hSubAngle0);
			auto const y1 = a_radiuses.y * std::sin(hSubAngle1);
			auto const r1 = std::cos(hSubAngle1);

			for (int v = 0; v < 2 * kVerticalSlices; ++v)
			{
				auto const vSliceAngle = (static_cast<float>(v) / kVerticalSlices) * std::numbers::pi_v<float>;
				auto const vSliceCos = std::cos(vSliceAngle);
				auto const vSliceSin = std::sin(vSliceAngle);
				auto const localPos0 = glm::vec4{ r0 * a_radiuses.x * vSliceSin, y0, r0 * a_radiuses.z * vSliceCos, 1.0f };
				auto const localPos1 = glm::vec4{ r1 * a_radiuses.x * vSliceSin, y1, r1 * a_radiuses.z * vSliceCos, 1.0f };
				a_debugMeshContext.add_line(cleanup(a_transform * localPos0), cleanup(a_transform * localPos1), a_color);
			}
		}
	}

	void draw_sphere(aoegl::debug_mesh_world_component& a_debugMeshContext, glm::vec3 const& a_position, float a_radius, aoegl::rgba const& a_color)
	{
		_draw_ellipsoid(a_debugMeshContext, aoest::combine(a_position, glm::quat{}), glm::vec3{ a_radius }, a_color);
	}
#pragma endregion

	float calc_energy(float a_mass, glm::vec3 const& a_linearVelocity, glm::mat3 const& a_inertia, glm::vec3 const& a_angularVelocity, glm::vec3 const& a_gravity, glm::vec3 const& a_position)
	{
		return 0.5f * a_mass * glm::dot(a_linearVelocity, a_linearVelocity) + 0.5f * glm::dot(a_angularVelocity, (a_inertia * a_angularVelocity)) + a_mass * glm::dot(-a_gravity, a_position);
	}

	test_system::test_system(aoeng::world_data_provider& a_wdp)
		: m_inputs{ a_wdp }
		, m_bindings{ a_wdp }
		, m_debugMeshWorldComponent{ a_wdp }
		, m_directorWorldComponent{ a_wdp }
		, m_cameraEntities{ a_wdp }
		, m_windowWorldComponent{ a_wdp }
		, m_simulationTimeContext{ a_wdp }
	{}

	float test(glm::vec3 const& a_radiuses, glm::vec3 const& a_position, glm::vec3 const& a_rotation,
		glm::vec3 const& a_linearVelocity, glm::vec3 const& a_angularVelocity, triangle const& a_triangle,
		float dt, float minDt, float maxDt, float minStep)
	{
		glm::vec3 move = dt * a_linearVelocity;
		glm::vec3 const& rotation = a_rotation + dt * a_angularVelocity;

		glm::mat4 transform = glm::scale(aoest::combine(a_position, rotation), a_radiuses);
		glm::mat4 invTransform = glm::inverse(transform);

		auto [hitTime, hitPos] = ellipsoid_cast(transform, invTransform, move, a_triangle);

		if (hitTime >= 1.0f)
		{
			if (maxDt - dt < minStep)
			{
				return dt;
			}

			return test(a_radiuses, a_position, a_rotation, a_linearVelocity, a_angularVelocity, a_triangle, (dt + maxDt) / 2, dt, maxDt, minStep);
		}
		else if (hitTime <= 0.0f)
		{
			return minDt;
		}
		else
		{
			if (dt - minDt < minStep)
			{
				return minDt;
			}

			return test(a_radiuses, a_position, a_rotation, a_linearVelocity, a_angularVelocity, a_triangle, (minDt + dt) / 2, minDt, dt, minStep);
		}
	}

	struct ellipsoid
	{
		float mass;
		glm::mat3 inertia;
		glm::mat3 invInertia;

		glm::vec3 radiuses;

		glm::vec3 position;
		glm::vec3 rotation;
		
		glm::vec3 linearVelocity;
		glm::vec3 angularVelocity;

		glm::mat4 get_transform() const
		{
			return aoest::combine(position, glm::quat(rotation));
		}

		glm::mat4 get_unit_sphere_transform() const
		{
			return glm::scale(get_transform(), radiuses);
		}

		void apply_central_force(glm::vec3 const& a_force, float a_dt)
		{
			linearVelocity += a_force / mass * a_dt;
		}

		void apply_force(glm::vec3 const& a_force, float a_dt, glm::vec3 a_point)
		{
			linearVelocity += a_force * a_dt;
			
			auto const transform = get_transform();
			auto const invTransform = glm::inverse(transform);
			auto const localForce = glm::vec3{ invTransform * glm::vec4{a_force, 0.0f} };
			auto const localPoint = cleanup(invTransform * glm::vec4{ a_point, 1.0f });
			auto const localAngularVelocityChange = invInertia * glm::cross(localPoint, localForce) * a_dt;
			auto const angularVelocityChange = glm::vec3{ transform * glm::vec4{localAngularVelocityChange, 0.0f} };
			angularVelocity + angularVelocityChange;
		}
	};




	std::optional<std::pair<glm::vec2, glm::vec2>> intersect_points_unit_sphere_with_normal_segment(float z, glm::vec2 const& p0, glm::vec2 const& p1)
	{
		assert(z <= 0.0f);

		// too far from sphere in z or null segment
		if (std::abs(z) >= 1.0f || glm::dot(p1 - p0, p1 - p0) < glm::epsilon<float>())
		{
			return std::nullopt;
		}

		auto const zRadiusSq = 1.0f - z * z;
		auto const p0Sq = glm::dot(p0, p0);
		auto const p1Sq = glm::dot(p1, p1);

		// segment entirely inside sphere
		if (p0Sq <= zRadiusSq && p1Sq <= zRadiusSq)
		{
			return std::pair{ p0, p1 };
		}

		auto const segmentVector = p1 - p0;
		auto const a = glm::dot(segmentVector, segmentVector);
		auto const b = 2.0f * glm::dot(p0, segmentVector);
		auto const c = glm::dot(p0, p0) - zRadiusSq;
		auto const d = b * b - 4.0f * a * c;

		// segment coming out
		if (p0Sq <= zRadiusSq)
		{
			auto const r = (-b + std::sqrt(d)) / (2.0f * a);
			auto const q = p0 + r * segmentVector;

			return std::pair{ p0, q };
		}
		// segment coming in
		else if (p1Sq <= zRadiusSq)
		{
			auto const r = (-b - std::sqrt(d)) / (2.0f * a);
			auto const q = p0 + r * segmentVector;

			return std::pair{ p0, q };
		}
		// segment always out
		else if (d < 0.0f)
		{
			return std::nullopt;
		}
		// segment comes in and out
		else
		{
			auto const r0 = (-b - std::sqrt(d)) / (2.0f * a);
			auto const r1 = (-b + std::sqrt(d)) / (2.0f * a);
			auto const q0 = p0 + r0 * segmentVector;
			auto const q1 = p0 + r1 * segmentVector;

			return std::pair{ q0, q1 };
		}
	}

	struct spatial_volume
	{
		glm::vec3 barycenter;
		float volume;
		float volumeB;

		friend spatial_volume operator+(spatial_volume const& lhs, spatial_volume const& rhs)
		{
			auto const totalVolume = lhs.volume + rhs.volume;
			return { lhs.barycenter * lhs.volume / totalVolume + rhs.barycenter * rhs.volume / totalVolume, totalVolume };
		}
	};

	float arctan(float x, float y)
	{
		assert(y >= 0.0f);

		if (x == 0.0f)
		{
			return 0.0f;
		}

		if (y < std::numeric_limits<float>::epsilon())
		{
			return (x < 0 ? -0.5f : 0.5f) * std::numbers::pi_v<float>;
		}

		auto const q = x / y;

		return std::atan(q);
	}

	float argtanh(float x, float y)
	{
		return std::atanh(x / y);
	}

	struct static_body
	{
		struct part
		{
			float ellasticity = 100'000.0f;
			float rolling_friction = 0.5f;
			float linear_resitution = 0.1f;
			float friction = 1.0f;

			std::vector<triangle> triangles;
		};

		glm::mat4 transform;

		std::vector<part> parts;
	};

	static inline std::optional<std::tuple<float, glm::vec3, glm::vec3>> _unit_ellipsoid_intersect2(glm::vec3 const& a_radiuses, triangle const& a_triangle)
	{
		// The "fake" variables refer to those for calculations done in the skewed space where ellispoid is the unit-sphere.
		auto const fakeP0 = a_triangle.m_p0 / a_radiuses;
		auto const fakeP1 = a_triangle.m_p1 / a_radiuses;
		auto const fakeP2 = a_triangle.m_p2 / a_radiuses;

		// 1. Ellipsoid's center is below triangle
		auto const fakeNormal = glm::normalize(glm::cross(fakeP1 - fakeP0, fakeP2 - fakeP0));
		if (glm::dot(fakeNormal, -fakeP0) < 0.0f)
		{
			return std::nullopt;
		}

		// 2. Ellipsoid is above triangle
		auto const fakePlaneDist = glm::dot(-fakeNormal, fakeP0 + fakeNormal);
		if (fakePlaneDist > 0.0f)
		{
			return std::nullopt;
		}

		// 3. Deepest point of triangle's plane inside ellipsoid belongs to the triangle
		auto const fakePlanePoint = -fakePlaneDist * fakeNormal;
		auto const normal = glm::normalize(glm::cross(a_triangle.m_p1 - a_triangle.m_p0, a_triangle.m_p2 - a_triangle.m_p0));
		auto const deepestEllipsoidPointInPlane = -fakeNormal * a_radiuses;
		auto const planePoint = deepestEllipsoidPointInPlane + glm::dot(a_triangle.m_p0 - deepestEllipsoidPointInPlane, normal) * normal;
		if (is_inside(planePoint, a_triangle))
		{
			return std::make_tuple(glm::dot(deepestEllipsoidPointInPlane - planePoint, normal), planePoint, deepestEllipsoidPointInPlane);
		}

		// Compute what point of a triangle's edge is deepest inside ellipsoid
		// (I think it's an approximation but unsure...).
		auto const t01 = -glm::dot(fakeP0, fakeP1 - fakeP0) / glm::dot(fakeP1 - fakeP0, fakeP1 - fakeP0);
		auto const fakeSegmentPoint01 = fakeP0 + glm::clamp(t01, 0.0f, 1.0f) * (fakeP1 - fakeP0);
		auto const t12 = -glm::dot(fakeP1, fakeP2 - fakeP1) / glm::dot(fakeP2 - fakeP1, fakeP2 - fakeP1);
		auto const fakeSegmentPoint12 = fakeP1 + glm::clamp(t12, 0.0f, 1.0f) * (fakeP2 - fakeP1);
		auto const t20 = -glm::dot(fakeP2, fakeP0 - fakeP2) / glm::dot(fakeP0 - fakeP2, fakeP0 - fakeP2);
		auto const fakeSegmentPoint20 = fakeP2 + glm::clamp(t20, 0.0f, 1.0f) * (fakeP0 - fakeP2);
		auto const d01Sq = glm::dot(fakeSegmentPoint01, fakeSegmentPoint01);
		auto const d12Sq = glm::dot(fakeSegmentPoint12, fakeSegmentPoint12);
		auto const d20Sq = glm::dot(fakeSegmentPoint20, fakeSegmentPoint20);
		auto const fakeTrianglePoint = d01Sq < d12Sq ? (d01Sq < d20Sq ? fakeSegmentPoint01 : fakeSegmentPoint20) : (d12Sq < d20Sq ? fakeSegmentPoint12 : fakeSegmentPoint20);

		// 4. Ellipsoid doesn't intersect triangle
		if (glm::dot(fakeTrianglePoint, fakeTrianglePoint) > 1.0f)
		{
			return std::nullopt;
		}

		// 5. Rough approximation only valid near the ellipsoid's surface
		auto const t = std::sqrt(std::max(0.0f, 1.0f / glm::dot(fakeTrianglePoint, fakeTrianglePoint)));
		auto const trianglePoint = fakeTrianglePoint * a_radiuses;
		auto const ellipsoidPoint = trianglePoint * t;
		return std::make_tuple(-glm::distance(trianglePoint, ellipsoidPoint), trianglePoint, ellipsoidPoint);

		// Below is an attempt at a better solution but cannot figure proper solution in all cases.
		/*constexpr auto k_epsilon = 0.01f;
		auto const epsilonRadiuses = a_radiuses * k_epsilon;

		using int3 = std::array<int, 3>;
		auto const radiusOrder = a_radiuses.x < a_radiuses.y ?
			a_radiuses.y < a_radiuses.z ? int3{0, 1, 2} : a_radiuses.x < a_radiuses.z ? int3{0, 2, 1} : int3{2, 0, 1} :
			a_radiuses.x < a_radiuses.z ? int3{1, 0, 2} : a_radiuses.y < a_radiuses.z ? int3{1, 2, 0} : int3{2, 1, 0};

		static const auto axes = std::array<glm::vec3, 3>{ glm::vec3{ 1.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f } };

		if (std::abs(trianglePoint.x) < epsilonRadiuses.x && std::abs(trianglePoint.y) < epsilonRadiuses.y && std::abs(trianglePoint.z) < epsilonRadiuses.z)
		{
			auto const r0 = a_radiuses[radiusOrder[0]];
			auto const r1 = a_radiuses[radiusOrder[1]];
			auto const r2 = a_radiuses[radiusOrder[2]];
			auto const p0 = trianglePoint[radiusOrder[0]];
			auto const p1 = trianglePoint[radiusOrder[1]];
			auto const p2 = trianglePoint[radiusOrder[2]];
			auto const a = 1.0f / (r0 * r0);
			auto const b = 2.0f * p0 / (r0 * r0);
			auto const c = p0 * p0 / (r0 * r0) + p1 * p1 / (r1 * r1) + p2 * p2 / (r2 * r2);
			auto const d = b * b - 4.0f * a * c;
			auto const dSqrt = std::sqrt(std::max(0.0f, d));
			auto const t0 = (-b - dSqrt) / (2.0f * a);
			auto const t1 = (-b + dSqrt) / (2.0f * a);
			auto const ellipsoidPoint0 = trianglePoint + t0 * axes[radiusOrder[0]];
			auto const ellipsoidPoint1 = trianglePoint + t1 * axes[radiusOrder[0]];

			return ???
		}

		auto const f = [&](auto const t) {
			auto fx = (trianglePoint.x / (a_radiuses.x + t / a_radiuses.x));
			auto fy = (trianglePoint.y / (a_radiuses.y + t / a_radiuses.y));
			auto fz = (trianglePoint.z / (a_radiuses.z + t / a_radiuses.z));
			return fx * fx + fy * fy + fz * fz - 1.0f; };

		auto const df = [&](auto const t) {
			auto const r2 = a_radiuses * a_radiuses;
			auto const p2 = trianglePoint * trianglePoint;
			auto const q = r2 + t;
			return -2.0f * (p2 * r2) / (q * q * q); };

		auto const nf = [&](auto const t) { return t - f(t) / df(t); };


		return std::make_tuple(0.0f, trianglePoint, trianglePoint);*/
	}

	static inline std::optional<std::tuple<float, glm::vec3, glm::vec3>> _ellipsoid_intersect(
		glm::mat4 const& a_transform, glm::mat4 const& a_invTransform, glm::vec3 const& a_radiuses, triangle const& a_triangle)
	{
		auto unitResult = _unit_ellipsoid_intersect2(
			a_radiuses,
			triangle{
				cleanup(a_invTransform * glm::vec4{ a_triangle.m_p0, 1.0f }),
				cleanup(a_invTransform * glm::vec4{ a_triangle.m_p1, 1.0f }),
				cleanup(a_invTransform * glm::vec4{ a_triangle.m_p2, 1.0f }) });

		if (unitResult == std::nullopt)
		{
			return std::nullopt;
		}

		auto const [dist, unitTrianglePoint, unitEllipsoidPoint] = *unitResult;
		return std::make_tuple(dist, cleanup(a_transform * glm::vec4{ unitTrianglePoint, 1.0f }), cleanup(a_transform * glm::vec4{ unitEllipsoidPoint, 1.0f }));
	}

	static inline std::optional<std::tuple<float, glm::vec3, glm::vec3>> _ellipsoid_intersect(
		glm::mat4 const& a_transform, glm::mat4 const& a_invTransform, glm::vec3 const& a_radiuses, std::vector<triangle> const& a_triangles)
	{
		std::optional<std::tuple<float, glm::vec3, glm::vec3>> deepestIntersection = std::nullopt;
		for (auto const& triangle : a_triangles)
		{
			auto const intersection = _ellipsoid_intersect(a_transform, a_invTransform, a_radiuses, triangle);
			if (intersection == std::nullopt)
			{
				continue;
			}

			if (deepestIntersection == std::nullopt)
			{
				deepestIntersection = intersection;
				continue;
			}

			if (std::get<0>(*deepestIntersection) > std::get<0>(*intersection))
			{
				deepestIntersection = intersection;
			}
		}

		return deepestIntersection;
	}

	static inline std::pair<float, glm::vec3> _ellipsoid_move2(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		glm::vec3 const& a_radiuses,
		glm::vec3 const& a_linearMove,
		glm::vec3 const& a_angularMove,
		triangle const& a_triangle)
	{
		auto const startTransform = glm::scale(aoest::combine(a_position, a_rotation), a_radiuses);
		auto const invStartTransform = glm::inverse(startTransform);
		auto const [startDist, startEllipsoidPos, startTrianglePos, coverRatio1] = ellipsoid_intersect(startTransform, invStartTransform, a_triangle);
		if (startDist < 0.0f)
		{
			return { 0.0f, startEllipsoidPos };
		}

		auto const stopTransform = glm::scale(
			aoest::combine(
				a_position + a_linearMove,
				a_rotation * glm::quat{ aoest::combine(a_position, a_rotation) * glm::vec4{ a_angularMove, 0.0f } }
			),
			a_radiuses);
		auto const invStopTransform = glm::inverse(stopTransform);
		auto const [stopDist, stopEllipsoidPos, stopTrianglePos, coverRatio2] = ellipsoid_intersect(stopTransform, invStopTransform, a_triangle);
		if (stopDist >= 0.0f && glm::dot(startEllipsoidPos - a_triangle.m_p0, a_triangle.m_p1 - a_triangle.m_p0) * glm::dot(stopEllipsoidPos - a_triangle.m_p0, a_triangle.m_p1 - a_triangle.m_p0))
		{
			return { 1.0f, a_position }; // Position is dummy here
		}

		return ellipsoid_move_rec2(a_position, a_rotation, a_linearMove, a_angularMove, a_radiuses, a_triangle, { 0.0f, 1.0f });
	}

	static inline std::pair<float, glm::vec3> _ellipsoid_move2(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		glm::vec3 const& a_radiuses,
		glm::vec3 const& a_linearMove,
		glm::vec3 const& a_angularMove,
		std::vector<triangle> const& a_triangles)
	{
		std::pair<float, glm::vec3> closestContact = std::make_tuple(1.0f, glm::vec3{0.0f});
		for (auto const& triangle : a_triangles)
		{
			auto const contact = _ellipsoid_move2(a_position, a_rotation, a_radiuses, a_linearMove, a_angularMove, triangle);
			if (0 <= contact.first && contact.first < closestContact.first)
			{
				closestContact = contact;
			}
		}
		return closestContact;
	}
	
	static inline glm::vec3 closest_point_on_ellipsoid(glm::vec3 const& radiuses, glm::vec3 const& point)
	{
		glm::vec3 scaled = point / radiuses; // point in unit-sphere space
		float lambda = 0.0f;

		for (int i = 0; i < 10; ++i)
		{
			glm::vec3 denom = glm::vec3(1.0f) + lambda / (radiuses * radiuses);
			glm::vec3 e = point / denom;

			float f = glm::dot(e / radiuses, e / radiuses) - 1.0f;

			if (std::abs(f) < 1e-4f)
				break;

			glm::vec3 df = -2.0f * e / (radiuses * radiuses * denom);
			float df_dot = glm::dot(df, point);

			lambda -= f / df_dot;
		}

		glm::vec3 denom = glm::vec3(1.0f) + lambda / (radiuses * radiuses);
		return point / denom;
	}

	static inline glm::vec3 _closest_triangle_point(glm::vec3 const& a_point, triangle const& a_triangle)
	{
		auto const a = a_triangle.m_p0;
		auto const b = a_triangle.m_p1;
		auto const c = a_triangle.m_p2;
		auto const p = a_point;

		glm::vec3 ab = b - a;
		glm::vec3 ac = c - a;
		glm::vec3 ap = p - a;

		float d1 = glm::dot(ab, ap);
		float d2 = glm::dot(ac, ap);
		if (d1 <= 0.0f && d2 <= 0.0f) return a;

		glm::vec3 bp = p - b;
		float d3 = glm::dot(ab, bp);
		float d4 = glm::dot(ac, bp);
		if (d3 >= 0.0f && d4 <= d3) return b;

		float vc = d1 * d4 - d3 * d2;
		if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
		{
			float v = d1 / (d1 - d3);
			return a + v * ab;
		}

		glm::vec3 cp = p - c;
		float d5 = glm::dot(ab, cp);
		float d6 = glm::dot(ac, cp);
		if (d6 >= 0.0f && d5 <= d6) return c;

		float vb = d5 * d2 - d1 * d6;
		if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
		{
			float w = d2 / (d2 - d6);
			return a + w * ac;
		}

		float va = d3 * d6 - d5 * d4;
		if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
		{
			float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
			return b + w * (c - b);
		}

		// Inside triangle
		float denom = glm::dot(ab, ab) * glm::dot(ac, ac) - glm::dot(ab, ac) * glm::dot(ab, ac);
		float v = (glm::dot(ac, ac) * glm::dot(ap, ab) - glm::dot(ab, ac) * glm::dot(ap, ac)) / denom;
		float w = (glm::dot(ab, ab) * glm::dot(ap, ac) - glm::dot(ab, ac) * glm::dot(ap, ab)) / denom;
		return a + ab * v + ac * w;
	}

	static inline glm::vec3 _closest_ellipsoid_point(glm::vec3 const& a_point, glm::vec3 const& a_radiuses)
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

	static inline std::tuple<float, glm::vec3, glm::vec3> _unit_ellipsoid_intersect(glm::vec3 const& a_radiuses, triangle const& a_triangle)
	{
		// returns signed distance, ellipsoid point, and triangle point

		auto const normal = glm::normalize(glm::cross(a_triangle.m_p1 - a_triangle.m_p0, a_triangle.m_p2 - a_triangle.m_p0));

		auto const normalEllipsoidDir = normal * a_radiuses * a_radiuses;
		auto const normalEllipsoidPoint = -normalEllipsoidDir / std::sqrt(glm::dot(normalEllipsoidDir, normal));
		auto const normalDistance = glm::dot(normalEllipsoidPoint - a_triangle.m_p0, normal);
		auto const normalTrianglePoint = normalEllipsoidPoint - normalDistance * normal;

		if (glm::dot(normal, -a_triangle.m_p0) < 0.0f)
		{
			return { -normalDistance, normalEllipsoidPoint, normalTrianglePoint };
		}

		auto const trianglePoint = _closest_triangle_point(normalTrianglePoint, a_triangle);
		auto const ellipsoidPoint = _closest_ellipsoid_point(trianglePoint, a_radiuses);
		auto const distance = glm::length(trianglePoint - ellipsoidPoint);
		auto const t = (trianglePoint * trianglePoint) / (a_radiuses * a_radiuses);
		return { t.x + t.y + t.z < 1.0f ? -distance : distance, ellipsoidPoint, trianglePoint };

		//{
		//	// The "fake" variables refer to those for calculations done in the skewed space where ellispoid is the unit-sphere.
		//	auto const fakeP0 = a_triangle.m_p0 / a_radiuses;
		//	auto const fakeP1 = a_triangle.m_p1 / a_radiuses;
		//	auto const fakeP2 = a_triangle.m_p2 / a_radiuses;

		//	// 1. Ellipsoid's center is below triangle
		//	auto const fakeNormal = glm::normalize(glm::cross(fakeP1 - fakeP0, fakeP2 - fakeP0));
		//	if (glm::dot(fakeNormal, -fakeP0) < 0.0f)
		//	{
		//		return { 0.0f, glm::vec3{ 0.0f }, glm::vec3{ 0.0f } };
		//	}

		//	// 2. Ellipsoid is above triangle
		//	auto const fakePlaneDist = glm::dot(-fakeNormal, fakeP0 + fakeNormal);
		//	if (fakePlaneDist > 1.0f)
		//	{
		//		return { 0.0f, glm::vec3{ 0.0f }, glm::vec3{ 0.0f } };
		//	}

		//	// 3. Deepest point of triangle's plane inside ellipsoid belongs to the triangle
		//	auto const fakePlanePoint = -fakePlaneDist * fakeNormal;
		//	auto const normal = glm::normalize(glm::cross(a_triangle.m_p1 - a_triangle.m_p0, a_triangle.m_p2 - a_triangle.m_p0));
		//	auto const deepestEllipsoidPointInPlane = -fakeNormal * a_radiuses;
		//	auto const planePoint = deepestEllipsoidPointInPlane + glm::dot(a_triangle.m_p0 - deepestEllipsoidPointInPlane, normal) * normal;
		//	if (is_inside(planePoint, a_triangle))
		//	{
		//		return std::make_tuple(glm::dot(deepestEllipsoidPointInPlane - planePoint, normal), deepestEllipsoidPointInPlane, planePoint);
		//	}

		//	// Compute what point of a triangle's edge is deepest inside ellipsoid
		//	// (I think it's an approximation but unsure...).
		//	auto const t01 = -glm::dot(fakeP0, fakeP1 - fakeP0) / glm::dot(fakeP1 - fakeP0, fakeP1 - fakeP0);
		//	auto const fakeSegmentPoint01 = fakeP0 + glm::clamp(t01, 0.0f, 1.0f) * (fakeP1 - fakeP0);
		//	auto const t12 = -glm::dot(fakeP1, fakeP2 - fakeP1) / glm::dot(fakeP2 - fakeP1, fakeP2 - fakeP1);
		//	auto const fakeSegmentPoint12 = fakeP1 + glm::clamp(t12, 0.0f, 1.0f) * (fakeP2 - fakeP1);
		//	auto const t20 = -glm::dot(fakeP2, fakeP0 - fakeP2) / glm::dot(fakeP0 - fakeP2, fakeP0 - fakeP2);
		//	auto const fakeSegmentPoint20 = fakeP2 + glm::clamp(t20, 0.0f, 1.0f) * (fakeP0 - fakeP2);
		//	auto const d01Sq = glm::dot(fakeSegmentPoint01, fakeSegmentPoint01);
		//	auto const d12Sq = glm::dot(fakeSegmentPoint12, fakeSegmentPoint12);
		//	auto const d20Sq = glm::dot(fakeSegmentPoint20, fakeSegmentPoint20);
		//	auto const fakeTrianglePoint = d01Sq < d12Sq ? (d01Sq < d20Sq ? fakeSegmentPoint01 : fakeSegmentPoint20) : (d12Sq < d20Sq ? fakeSegmentPoint12 : fakeSegmentPoint20);

		//	// 4. Ellipsoid doesn't intersect triangle
		//	if (glm::dot(fakeTrianglePoint, fakeTrianglePoint) > 1.0f)
		//	{
		//		return { 0.0f, fakeTrianglePoint * a_radiuses, fakeTrianglePoint * a_radiuses };
		//	}

		//	// 5. Rough approximation only valid near the ellipsoid's surface
		//	auto const t = std::sqrt(std::max(0.0f, 1.0f / glm::dot(fakeTrianglePoint, fakeTrianglePoint)));
		//	auto const trianglePoint = fakeTrianglePoint * a_radiuses;
		//	auto const ellipsoidPoint = trianglePoint * t;
		//	return std::make_tuple(-glm::distance(trianglePoint, ellipsoidPoint), ellipsoidPoint, trianglePoint);
		//}
	}

	std::tuple<float, glm::vec3, glm::vec3> _ellipsoid_intersect(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		glm::vec3 const& a_radiuses,
		triangle const& a_triangle)
	{
		auto const transform = aoest::combine(a_position, a_rotation);
		auto const invTransform = glm::inverse(transform);

		auto [dist, unitEllipsoidPoint, unitTrianglePoint] = _unit_ellipsoid_intersect(
			a_radiuses,
			triangle{
				cleanup(invTransform * glm::vec4{ a_triangle.m_p0, 1.0f }),
				cleanup(invTransform * glm::vec4{ a_triangle.m_p1, 1.0f }),
				cleanup(invTransform * glm::vec4{ a_triangle.m_p2, 1.0f }) });

		auto const ellipsoidPoint = cleanup(transform * glm::vec4{ unitEllipsoidPoint, 1.0f });
		auto const trianglePoint = cleanup(transform * glm::vec4{ unitTrianglePoint, 1.0f });

		return std::make_tuple(dist, ellipsoidPoint, trianglePoint);
	}

	float _ellipsoid_cast(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		glm::vec3 const& a_radiuses,
		glm::vec3 const& a_linearMove,
		triangle const& a_triangle)
	{
		auto const transform = glm::scale(aoest::combine(a_position, a_rotation), a_radiuses);
		auto const invTransform = glm::inverse(transform);

		auto [hitTime, hitPos] = unit_sphere_cast(
			glm::vec3{ invTransform * glm::vec4{ a_linearMove, 0.0f } },
			triangle{
				cleanup(invTransform * glm::vec4{a_triangle.m_p0, 1.0f}),
				cleanup(invTransform * glm::vec4{a_triangle.m_p1, 1.0f}),
				cleanup(invTransform * glm::vec4{a_triangle.m_p2, 1.0f})
			});

		return hitTime;
	}

	std::tuple<float, glm::vec3, glm::vec3> _ellipsoid_move(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		glm::vec3 const& a_radiuses,
		glm::vec3 const& a_linearMove,
		glm::vec3 const& a_angularMove,
		triangle const& a_triangle,
		std::vector<std::pair<glm::mat4, aoegl::rgba>>& a_testTransforms,
		std::int32_t& a_testCount)
	{
		auto const [startDist, startEllipsoidPoint, startTrianglePoint] = _ellipsoid_intersect(
			a_position, a_rotation, a_radiuses, a_triangle);
		if (startDist < 0.0f)
		{
			return { 0.0f, startEllipsoidPoint, startTrianglePoint };
		}

		auto const triangleNormal = glm::normalize(glm::cross(a_triangle.m_p1 - a_triangle.m_p0, a_triangle.m_p2 - a_triangle.m_p0));
		auto const dotLinearMoveTriangleNormal = glm::dot(a_linearMove, triangleNormal);
		if (dotLinearMoveTriangleNormal > -glm::epsilon<float>())
		{
			return { 1.0f, glm::vec3{ 0.0f }, glm::vec3{ 0.0f } };
		}
		auto const maxTime = std::clamp(glm::dot(a_triangle.m_p0 - a_position, triangleNormal) / dotLinearMoveTriangleNormal, 0.0f, 1.0f);
		auto const linearMove = maxTime * a_linearMove;
		auto const angularMove = maxTime * a_angularMove;

		auto const finalRotation = glm::quat{ angularMove } * a_rotation;
		auto const [preRotationDist, preRotationEllipsoidPoint, preRotationTrianglePoint] = _ellipsoid_intersect(
			a_position, finalRotation, a_radiuses, a_triangle);

		auto const castDist = _ellipsoid_cast(a_position, finalRotation, a_radiuses, linearMove, a_triangle);
		if (preRotationDist >= 0.0f && (castDist < 0.0f || castDist >= 1.0f))
		{
			return { 1.0f, glm::vec3{ 0.0f }, glm::vec3{ 0.0f } };
		}

		a_testCount = 0;
		auto timeRange = std::pair<float, float>{ 0.0f, 1.0f };
		auto const k_stepRatio = 0.5f;
		while (timeRange.second - timeRange.first > 0.01f)
		{
			auto const leftPosition = a_position + timeRange.first * linearMove;
			auto const midTime = timeRange.first * (1.0f - k_stepRatio) + timeRange.second * k_stepRatio;
			auto const midRotation = glm::quat{ angularMove * midTime } * a_rotation;
			auto const [rotationDist, rotationEllipsoidPoint, rotationTrianglePoint] = _ellipsoid_intersect(leftPosition, midRotation, a_radiuses, a_triangle);
			if (rotationDist < 0.0f)
			{
				timeRange.second = midTime;
				continue;
			}

			auto const castDist = _ellipsoid_cast(leftPosition, midRotation, a_radiuses, linearMove * (midTime - timeRange.first), a_triangle);
			if (castDist >= 0.0f && castDist < 1.0f)
			{
				timeRange.second = midTime;
				continue;
			}

			timeRange.first = midTime;
			timeRange.second = 1.0f;
			// a_testTransforms.emplace_back(aoest::combine(a_position + midTime * linearMove, midRotation), aoegl::k_gray);

			++a_testCount;
		}

		auto const leftPosition = a_position + timeRange.first * linearMove;
		auto const leftRotation = glm::quat{ angularMove * timeRange.first } * a_rotation;
		// a_testTransforms.emplace_back(aoest::combine(leftPosition, leftRotation), aoegl::k_cyan);
		auto const finalCastDist = _ellipsoid_cast(leftPosition, leftRotation, a_radiuses, linearMove * (timeRange.second - timeRange.first), a_triangle);

		if (finalCastDist < 0.0f)
		{
			auto [finalDist, finalEllipsoidPosition, finalTrianglePosition] = _ellipsoid_intersect(
				a_position + timeRange.second * linearMove, glm::quat{ angularMove * timeRange.second } * a_rotation, a_radiuses, a_triangle);
			return { timeRange.second * maxTime, finalEllipsoidPosition, finalTrianglePosition };
		}

		auto const finalTime = std::clamp(finalCastDist, 0.0f, 1.0f) * (timeRange.second - timeRange.first) + timeRange.first;
		auto const finalPosition = a_position + finalTime * linearMove;

		auto const [_, ellipsoidPoint, trianglePoint] = _ellipsoid_intersect(finalPosition, leftRotation, a_radiuses, a_triangle);
		return { finalTime * maxTime, ellipsoidPoint, trianglePoint };
	}

	glm::vec3 _ellipsoid_normal(
		glm::vec3 const& a_position,
		glm::quat const& a_rotation,
		glm::vec3 const& a_radiuses,
		glm::vec3 const& a_point)
	{
		auto const transform = aoest::combine(a_position, a_rotation);
		auto const invTransform = glm::inverse(transform);

		auto const invPoint = cleanup(invTransform * glm::vec4{ a_point, 1.0f });
		auto const invNormal = invPoint / (a_radiuses * a_radiuses);
		return glm::normalize(glm::vec3{ transform * glm::vec4{ invNormal, 0.0f } });
	}

	struct solid_shape
	{
		struct part
		{
			glm::vec3 position = glm::vec3{ 0.0f };
			glm::quat rotation = glm::quat{};
			glm::vec3 radiuses = glm::vec3{ 1.0f };
		};

		std::vector<part> parts;

		glm::vec3 barycenter = glm::vec3{ 0.0f };
	};

	glm::quat _differentiate_quaternion(glm::quat const& a_rotation, glm::vec3 const& a_angularVelocity)
	{
		auto const angularVelocity = glm::quat{ 0.0f, a_angularVelocity.x, a_angularVelocity.y, a_angularVelocity.z };
		return 0.5f * a_rotation * angularVelocity;
	}

	glm::quat integrate_quaternion(glm::quat const& a_rotation, glm::vec3 const& a_angularVelocity, float a_dt)
	{
		auto const angle = glm::length(a_angularVelocity) * a_dt;
		if (angle < glm::epsilon<float>())
		{
			return a_rotation;
		}

		auto const axis = glm::normalize(a_angularVelocity);
		return glm::normalize(glm::angleAxis(angle, axis) * a_rotation);
	}

#pragma optimize("", off)
	void test_system::update() const
	{

#pragma region STATIC_AND_IMGUI

		// 0. statics
		// A. time
		static auto k_useFixedTimeStep = true;
		static auto k_maxDeltaTime = 1.0f / 30.0f;
		static auto k_fixedTimeStep = 1.0f / 100.0f;
		static auto k_integrationStepCount = 10;
		static auto k_simulationStep = 0;

		static std::chrono::high_resolution_clock::time_point k_lastUpdateTime = {};
		static auto k_unusedElapsedTime = 0.0f;
		static auto k_computationTime = 0.0f;
		auto const computationStartTime = std::chrono::high_resolution_clock().now();
		// B. world
		static auto k_gravity = glm::vec3{ 0.0f, -10.0f, 0.0f };
		static auto k_groundTriangles = std::vector<triangle>();
		static auto k_groundTriangle = triangle{
			glm::vec3{ -5.0f, 2.0f, -5.0f },
			glm::vec3{ -5.0f, 2.0f, 20.0f },
			glm::vec3{ 20.0f, 2.0f, -5.0f }
		};
		static auto k_groundTriangle2 = triangle{
			glm::vec3{ -10.0f, 1.0f, 0.0f },
			glm::vec3{ -10.0f, 100.0f, 0.0f },
			glm::vec3{ -10.0f, 1.0f, 100.0f }
		};
		static auto k_groundEllasticity = 100'000.0f;
		static auto k_groundRestitution = 0.5f;
		static auto k_groundFriction = 0.128f;
		static auto k_groundRollingFriction = 1.0f; // how sticky+soft surface prevent rolling
		// C. ellipsoid
		static auto k_ellipsoidRadiuses = glm::vec3{ 1.067f, 0.818f, 2.146f };
		static auto k_ellipsoidMass = 1.0f;
		static auto k_ellipsoidStartPosition = glm::vec3{ 0.0f, 10.0f, 0.0f };
		static auto k_ellipsoidStartRotation = glm::vec3{ 0.0f, 0.0f, 0.0f };
		static auto k_ellipsoidStartLinearVelocity = glm::vec3{ 0.0f, 0.0f, 0.0f };
		static auto k_ellipsoidStartAngularVelocity = glm::vec3{ 0.0f, 0.0f, 0.0f };

		static auto k_ellipsoidPosition = k_ellipsoidStartPosition;
		static auto k_ellipsoidRotation = glm::quat(k_ellipsoidStartRotation);
		static auto k_ellipsoidLinearVelocity = k_ellipsoidStartLinearVelocity;
		static auto k_ellipsoidAngularVelocity = k_ellipsoidStartAngularVelocity;
		static auto k_ellipsoidEnergy = 0.0f;
		// D. physics
		static auto k_lastContactTrianglePosition = glm::vec3{ 0.0f };
		static auto k_lastContactEllipsoidPosition = glm::vec3{ 0.0f };
		static auto k_lastContactForce = glm::vec3{ 0.0f };
		static auto k_lastContactNormal = glm::vec3{ 0.0f };
		static auto k_maxIntegrationStepsDone = 0;
		static auto k_y0 = k_ellipsoidPosition.y;
		static auto k_y1 = k_ellipsoidPosition.y;
		static auto k_yMax = k_ellipsoidPosition.y - k_ellipsoidRadiuses.y;
		// E. solid
		solid_shape k_solidShape = solid_shape{
			std::vector<solid_shape::part>{
				// front axel
				solid_shape::part{ glm::vec3{ -0.01553f, 0.36325f, -1.75357f } - glm::vec3{ 0.0f, 0.35f, 0.0f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{0.905f, 0.283f, 0.385f}},
				// mid axel
				solid_shape::part{ glm::vec3{ 0.0f, 0.471f, -0.219f } - glm::vec3{ 0.0f, 0.35f, 0.0f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{0.439f, 0.362f, 1.902f}},
				// cockpit
				solid_shape::part{ glm::vec3{ 0.0f, 0.65281f, 0.89763f } - glm::vec3{ 0.0f, 0.35f, 0.0f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{1.021f, 0.515f, 1.038f}},
				// chassis
				solid_shape::part{ glm::vec3{ 0.0f, 0.44878f, 0.20792f } - glm::vec3{ 0.0f, 0.35f, 0.0f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{0.968f, 0.363f, 1.682f}},
				// front left wheel
				solid_shape::part{ glm::vec3{ -0.86301f, 0.3525f, -1.78209f } - glm::vec3{ 0.0f, 0.35f, 0.0f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{0.182f, 0.364f, 0.364f}},
				// front right wheel
				solid_shape::part{ glm::vec3{ 0.86299f, 0.3525f, -1.78209f } - glm::vec3{ 0.0f, 0.35f, 0.0f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{0.182f, 0.364f, 0.364f}},
				// back left wheel
				solid_shape::part{ glm::vec3{ -0.885f, 0.3525f, 1.2055f } - glm::vec3{ 0.0f, 0.35f, 0.0f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{0.182f, 0.364f, 0.364f}},
				// back right wheel
				solid_shape::part{ glm::vec3{ 0.885f, 0.3525f, 1.2055f } - glm::vec3{ 0.0f, 0.35f, 0.0f }, glm::quat(glm::vec3{ 0.0f }), glm::vec3{0.182f, 0.364f, 0.364f}},
			}
		};

		struct triple
		{
			glm::vec3 first;
			glm::vec3 second;
			glm::vec3 third;
		};

		static std::vector<std::optional<triple>> k_lastCollisionPoints;

		if (m_inputs->keyboard.keys[aoein::keyboard::key::O].is_pressed())
		{
			auto maxRadiuses = glm::vec3{ 0.0f };
			for (auto const& part : k_solidShape.parts)
			{
				maxRadiuses.x = std::max(maxRadiuses.x, std::abs(part.position.x) + part.radiuses.x);
				maxRadiuses.y = std::max(maxRadiuses.y, std::abs(part.position.y) + part.radiuses.y);
				maxRadiuses.z = std::max(maxRadiuses.z, std::abs(part.position.z) + part.radiuses.z);
			}

			k_ellipsoidRadiuses = maxRadiuses;
		}

		if (m_inputs->keyboard.keys[aoein::keyboard::key::P].is_pressed())
		{
			k_ellipsoidLinearVelocity = k_ellipsoidStartLinearVelocity;
			k_ellipsoidAngularVelocity = k_ellipsoidStartAngularVelocity;
			k_ellipsoidPosition = k_ellipsoidStartPosition;
			k_ellipsoidRotation = glm::quat(k_ellipsoidStartRotation);
			k_maxIntegrationStepsDone = 0;
			k_simulationStep = 0;
			k_unusedElapsedTime = 0.0f;
		}

		if (m_inputs->mouse.buttons[aoein::mouse::button::Left].is_pressed())
		{
			k_ellipsoidPosition.y -= m_inputs->mouse.axes[aoein::mouse::axis::Y].get_change() * 0.01f;
		}

		// 1. imgui
		ImGui::Begin("Test System");

		ImGui::SeparatorText("Time Settings");
		ImGui::InputFloat("Max Delta Time", &k_maxDeltaTime);
		ImGui::Checkbox("Use Fixed Simulation Time Step", &k_useFixedTimeStep);
		if (k_useFixedTimeStep)
		{
			ImGui::InputFloat("Fixed Simulation Time Step", &k_fixedTimeStep);
		}
		ImGui::InputInt("Integration Step Count", &k_integrationStepCount);
		ImGui::BeginDisabled();
		ImGui::InputFloat("Computation Time", &k_computationTime);
		ImGui::InputInt("Step", &k_simulationStep);
		auto fps = 1000.0f * m_simulationTimeContext->elapsed_time.get_value();
		ImGui::InputFloat("Frame Time (ms)", &fps);

		auto sp = [](glm::vec3 const& a) { return a.x + a.y + a.z; };
		auto sq = [](glm::quat const& a) { return a.x + a.y + a.z + a.w; };

		ImGui::EndDisabled();

		ImGui::SeparatorText("World Settings");
		ImGui::InputFloat3("Gravity", &k_gravity.x);
		ImGui::InputFloat3("Ground Triangle P0", &k_groundTriangle.m_p0.x);
		ImGui::InputFloat3("Ground Triangle P1", &k_groundTriangle.m_p1.x);
		ImGui::InputFloat3("Ground Triangle P2", &k_groundTriangle.m_p2.x);
		ImGui::InputFloat3("Ground Triangle 2 P0", &k_groundTriangle2.m_p0.x);
		ImGui::InputFloat3("Ground Triangle 2 P1", &k_groundTriangle2.m_p1.x);
		ImGui::InputFloat3("Ground Triangle 2 P2", &k_groundTriangle2.m_p2.x);
		ImGui::InputFloat("Ground Ellasticity", &k_groundEllasticity);
		ImGui::InputFloat("Ground Restitution", &k_groundRestitution);
		ImGui::InputFloat("Ground Friction", &k_groundFriction);
		ImGui::InputFloat("Ground Rolling Friction", &k_groundRollingFriction);

		ImGui::SeparatorText("Ellipsoid Settings");
		ImGui::InputFloat3("Radiuses", &k_ellipsoidRadiuses.x);
		ImGui::InputFloat("Mass", &k_ellipsoidMass);
		ImGui::InputFloat3("Start Position", &k_ellipsoidStartPosition.x);
		ImGui::InputFloat3("Start Rotation", &k_ellipsoidStartRotation.x);
		ImGui::InputFloat3("Start Linear Velocity", &k_ellipsoidStartLinearVelocity.x);
		ImGui::InputFloat3("Start Angular Velocity", &k_ellipsoidStartAngularVelocity.x);

		auto k_ellipsoidInertia = k_ellipsoidMass / 5.0f * glm::diagonal3x3(glm::vec3{
			k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y + k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z,
			k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z + k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x,
			k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x + k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y });

		auto const frameTime = m_simulationTimeContext->elapsed_time.get_value();
		k_unusedElapsedTime += std::clamp(frameTime, 0.0f, k_maxDeltaTime);
		auto const isPaused = frameTime == 0.0f;
		if (isPaused)
		{
			k_unusedElapsedTime = 0;
		}
		auto simulationTimeStep = k_useFixedTimeStep ? (k_unusedElapsedTime > k_fixedTimeStep ? k_fixedTimeStep : 0.0f) : std::min(k_unusedElapsedTime, k_maxDeltaTime);
		k_unusedElapsedTime = k_useFixedTimeStep ? k_unusedElapsedTime - simulationTimeStep : 0.0f;
		if (simulationTimeStep > 0.0f)
		{
			++k_simulationStep;
		}

		k_ellipsoidEnergy = calc_energy(k_ellipsoidMass, k_ellipsoidLinearVelocity, k_ellipsoidInertia, k_ellipsoidAngularVelocity, k_gravity, k_ellipsoidPosition);

		ImGui::SeparatorText("State");
		if (!isPaused)
		{
			ImGui::BeginDisabled();
		}

		{
			auto position = k_ellipsoidPosition;
			ImGui::InputFloat3("Position", &position.x);
			auto ellipsoidEuler = glm::eulerAngles(k_ellipsoidRotation);
			ImGui::InputFloat3("Rotation", &ellipsoidEuler.x);
			auto linearVelocity = k_ellipsoidLinearVelocity;
			ImGui::InputFloat3("Linear Velocity", &linearVelocity.x);
			auto angularVelocity = k_ellipsoidAngularVelocity;
			ImGui::InputFloat3("Angular Velocity", &angularVelocity.x);
		}

		if (isPaused)
		{
			ImGui::BeginDisabled();
		}
		ImGui::InputFloat("Energy", &k_ellipsoidEnergy);
		ImGui::InputFloat("Max Y", &k_yMax);
		ImGui::EndDisabled();
		static bool k_rk4 = true;
		ImGui::Checkbox("RK4", &k_rk4);

		ImGui::End();
#pragma endregion

#pragma region DEBUG_DRAW

		// 2. draw
		_draw_triangle(*m_debugMeshWorldComponent, k_groundTriangle.m_p0, k_groundTriangle.m_p1, k_groundTriangle.m_p2, aoegl::to_rgba(glm::vec4{ 0.25f }));
		_draw_triangle(*m_debugMeshWorldComponent, k_groundTriangle2.m_p0, k_groundTriangle2.m_p1, k_groundTriangle2.m_p2, aoegl::to_rgba(glm::vec4{ 0.25f }));
		// vob _draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses, aoegl::to_rgba(glm::vec4{ 0.5f }));
		// _draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses, aoegl::to_rgba(glm::vec4{ 0.4f, 0.1f, 0.1f, 1.0f }));

		// draw solid
		for (auto const& solidPart : k_solidShape.parts)
		{
			_draw_ellipsoid(
				*m_debugMeshWorldComponent,
				aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation) * aoest::combine(solidPart.position, solidPart.rotation),
				solidPart.radiuses,
				// vob aoegl::to_rgba(glm::vec4{ 0.4f, 0.1f, 0.1f, 1.0f }));
				aoegl::to_rgba(glm::vec4{ 0.5f }));
		}
		draw_line(*m_debugMeshWorldComponent, k_ellipsoidPosition, k_ellipsoidPosition + k_ellipsoidLinearVelocity, aoegl::k_green);
		draw_line(*m_debugMeshWorldComponent, k_ellipsoidPosition, k_ellipsoidPosition + k_ellipsoidAngularVelocity, aoegl::k_red);

		for (auto const& lastCollisionPoint : k_lastCollisionPoints)
		{
			if (lastCollisionPoint.has_value())
			{
				draw_sphere(*m_debugMeshWorldComponent, lastCollisionPoint->first, 0.1f, aoegl::k_red);
				draw_line(*m_debugMeshWorldComponent, lastCollisionPoint->first, lastCollisionPoint->first + lastCollisionPoint->second, aoegl::k_orange);
				draw_sphere(*m_debugMeshWorldComponent, lastCollisionPoint->third, 0.1f, aoegl::k_yellow);
			}
		}

		// draw hits
		//draw_line(*m_debugMeshWorldComponent, k_lastContactEllipsoidPosition, k_lastContactEllipsoidPosition + k_lastContactForce, aoegl::k_orange);
		//draw_sphere(*m_debugMeshWorldComponent, k_lastContactEllipsoidPosition, 0.1f, aoegl::k_red);
#pragma endregion





		if (simulationTimeStep == 0.0f)
		{
			return;
		}

		if (m_inputs->keyboard.keys[aoein::keyboard::key::Up].is_pressed())
		{
			k_ellipsoidLinearVelocity += 10.0f * simulationTimeStep * glm::vec3{ aoest::combine(glm::vec3{0.0f}, k_ellipsoidRotation) * glm::vec4{0.0f, 0.0f, -1.0f, 1.0f} };
			k_ellipsoidAngularVelocity = glm::vec3{ 0.0f };
		}
		if (m_inputs->keyboard.keys[aoein::keyboard::key::Down].is_pressed())
		{
			k_ellipsoidLinearVelocity -= 10.0f * simulationTimeStep * glm::vec3{ aoest::combine(glm::vec3{0.0f}, k_ellipsoidRotation) * glm::vec4{0.0f, 0.0f, -1.0f, 1.0f} };
			k_ellipsoidAngularVelocity = glm::vec3{ 0.0f };
		}
		if (m_inputs->keyboard.keys[aoein::keyboard::key::Left].is_pressed())
		{
			auto const localVelocity = glm::inverse(glm::mat3{ k_ellipsoidRotation }) * k_ellipsoidLinearVelocity;

			auto const sign = glm::dot(localVelocity, glm::vec3{ 0.0f, 0.0f, -1.0f }) > 0.0f ? 1.0f : -1.0f;
			auto const theta = sign * glm::vec3{ 0.0f, 1.0f, 0.0f } * simulationTimeStep * std::sqrt(glm::length(localVelocity) / 10.0f);
			auto const thetaMagnitude = glm::length(theta);
			if (thetaMagnitude > glm::epsilon<float>()) // some friction constant?
			{
				glm::vec3 axis = theta / thetaMagnitude;
				float halfTheta = thetaMagnitude / 2.0f;
				k_ellipsoidRotation = glm::quat(std::cos(halfTheta), axis * std::sin(halfTheta)) * k_ellipsoidRotation;
			}

			k_ellipsoidLinearVelocity = glm::mat3{ k_ellipsoidRotation } *localVelocity;
		}
		if (m_inputs->keyboard.keys[aoein::keyboard::key::Right].is_pressed())
		{
			auto const localVelocity = glm::inverse(glm::mat3{ k_ellipsoidRotation }) * k_ellipsoidLinearVelocity;

			auto const sign = glm::dot(localVelocity, glm::vec3{0.0f, 0.0f, -1.0f }) > 0.0f ? 1.0f : -1.0f;
			auto const theta = sign * glm::vec3{ 0.0f, -1.0f, 0.0f } *simulationTimeStep * std::sqrt(glm::length(localVelocity) / 10.0f);
			auto const thetaMagnitude = glm::length(theta);
			if (thetaMagnitude > glm::epsilon<float>()) // some friction constant?
			{
				glm::vec3 axis = theta / thetaMagnitude;
				float halfTheta = thetaMagnitude / 2.0f;
				k_ellipsoidRotation = glm::quat(std::cos(halfTheta), axis * std::sin(halfTheta)) * k_ellipsoidRotation;
			}

			k_ellipsoidLinearVelocity = glm::mat3{ k_ellipsoidRotation } *localVelocity;
		}


		// 3. physics
		auto const ellipsoidTransform = glm::scale(aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses);
		auto const ellipsoidTransformInv = glm::inverse(ellipsoidTransform);

		// for each dynamic solid
		auto const& solidShape = k_solidShape;
		auto const solidMass = k_ellipsoidMass;
		auto const solidInertiaLocal = k_ellipsoidInertia;
		auto solidPosition = k_ellipsoidPosition;
		auto solidLinearVelocity = k_ellipsoidLinearVelocity;
		auto solidRotation = k_ellipsoidRotation;
		auto solidAngularVelocityLocal = k_ellipsoidAngularVelocity;

		// (pre-find static triangles to consider)
		auto const staticTriangles = std::array<triangle, 2>{ k_groundTriangle, k_groundTriangle2 };
		/* debug */k_lastCollisionPoints.clear();
		/* debug */k_lastCollisionPoints.resize(k_solidShape.parts.size(), std::nullopt);

		// for each integration step
		auto const integrationStepDuration = simulationTimeStep / k_integrationStepCount;
		for (auto integrationStepIndex = 0; integrationStepIndex < k_integrationStepCount; ++ integrationStepIndex)
		{
			// integrate with rk4
			struct rk4_state
			{
				glm::vec3 position;
				glm::quat rotation;
				glm::vec3 linearVelocity;
				glm::vec3 angularVelocityLocal;
			};
				
			auto const rk4_derivate = [&](rk4_state const& a_state)
				{
					auto const solidRotationMatrix = glm::mat3_cast(a_state.rotation);
					auto const solidInertia = solidRotationMatrix * solidInertiaLocal * glm::transpose(solidRotationMatrix);
					auto const solidInertiaInv = glm::inverse(solidInertia);

					auto force = k_gravity * solidMass;
					auto torque = glm::vec3{ 0.0f };

					auto contactPointCount = 0;
					for (auto const& solidPart : solidShape.parts)
					{
						auto const solidPartPosition = a_state.position + solidRotationMatrix * solidPart.position;
						auto const solidPartRotation = a_state.rotation * solidPart.rotation;
						auto const solidPartRadiuses = solidPart.radiuses;

						for (auto const& staticTriangle : staticTriangles)
						{
							auto [staticTriangleDist, ellipsoidPoint, staticTrianglePoint] = _ellipsoid_intersect(solidPartPosition, solidPartRotation, solidPartRadiuses, staticTriangle);
							if (staticTriangleDist < 0.0f)
							{
								++contactPointCount;
								break;
							}
						}
					}
						
					/* debug */ int32_t partIndex = 0;
					for (auto const& solidPart : solidShape.parts)
					{
						/* debug */ ++partIndex;
						auto const solidPartPosition = a_state.position + solidRotationMatrix * solidPart.position;
						auto const solidPartRotation = a_state.rotation * solidPart.rotation;
						auto const solidPartRadiuses = solidPart.radiuses;


						auto closestStaticTriangleDist = 0.0f;
						auto closestStaticTrianglePoint = glm::vec3{};
						auto closestEllipsoidPoint = glm::vec3{};
						auto closestStaticTriangle = triangle{};
						for (auto const& staticTriangle : staticTriangles)
						{
							auto [staticTriangleDist, ellipsoidPoint, staticTrianglePoint] = _ellipsoid_intersect(solidPartPosition, solidPartRotation, solidPartRadiuses, staticTriangle);
							if (staticTriangleDist < closestStaticTriangleDist)
							{
								closestStaticTriangleDist = staticTriangleDist;
								closestStaticTrianglePoint = staticTrianglePoint;
								closestEllipsoidPoint = ellipsoidPoint;
								closestStaticTriangle = staticTriangle;
							}
						}

						auto const ellipsoidToTriangle = closestStaticTrianglePoint - closestEllipsoidPoint;
						auto const penetration = glm::length(ellipsoidToTriangle);
						if (penetration > glm::epsilon<float>())
						{
							auto const lever = closestEllipsoidPoint - a_state.position;
							auto const hitVelocity = a_state.linearVelocity + glm::cross(a_state.angularVelocityLocal, lever);
							auto const hitNormal = glm::normalize(ellipsoidToTriangle);
							auto const hitVelocityNormal = glm::dot(hitVelocity, hitNormal) * hitNormal;
							auto const hitVelocityTangent = hitVelocity - hitVelocityNormal;
							/* debug */ k_lastCollisionPoints[partIndex - 1] = { closestEllipsoidPoint, hitNormal, closestStaticTrianglePoint };

							// spring
							static auto k_maxPenetration = 0.1f;
							auto const clampedPenetration = std::min(penetration, k_maxPenetration);
							auto const springForce = k_groundEllasticity * clampedPenetration * hitNormal;

							// dampener
							auto const lne = std::log(k_groundRestitution);
							auto const zetaLow = std::sqrt(lne * lne / (lne * lne + std::numbers::pi_v<float> *std::numbers::pi_v<float>));
							auto const zetaHigh = zetaLow; // 1.0f;
							auto const speedNormal = glm::length(hitVelocityNormal);
							auto const zeta = glm::mix(zetaHigh, zetaLow, glm::smoothstep(0.01f, 0.2f, speedNormal));
							auto const dampingCoefficient = 2.0f * std::sqrt(k_groundEllasticity * (solidMass / contactPointCount)) * zeta; // should I use something else than solidMass/contactPointCount ?
							auto const dampenerForce = -dampingCoefficient * hitVelocityNormal;

							// friction
							auto frictionForce = glm::vec3{ 0.0f };
							if (glm::length(hitVelocityTangent) > glm::epsilon<float>())
							{
								auto const frictionDir = -glm::normalize(hitVelocityTangent);
								auto const maxFriction = k_groundFriction * glm::length(springForce);
								frictionForce = frictionDir * std::min(maxFriction, (solidMass / contactPointCount) * glm::length(hitVelocityTangent) / integrationStepDuration);
							}

							force += springForce + dampenerForce + frictionForce;
							torque += glm::cross(lever, springForce + dampenerForce + frictionForce);
							// torque -= k_groundRollingFriction * angularVelocity -> how?
							// if (penetration > k_maxPenetration && glm::dot(s.velocity, hitNormal) < 0.0f)
							// {
							//	force -= k_ellipsoidMass * glm::dot(s.velocity, hitNormal) * hitNormal / ds;
							// } -> how?
						}
					}

					rk4_state derivativeState;
					derivativeState.position = a_state.linearVelocity;
					derivativeState.linearVelocity = force / solidMass;
					derivativeState.rotation = _differentiate_quaternion(a_state.rotation, glm::transpose(solidRotationMatrix) * a_state.angularVelocityLocal);
					derivativeState.angularVelocityLocal = solidInertiaInv * (torque - glm::cross(a_state.angularVelocityLocal, solidInertia * a_state.angularVelocityLocal));
					return derivativeState;
				};

			auto const rk4_step = [&](rk4_state const& a_initialState, rk4_state const& a_prevDerivativeState, float const a_dt)
				{
					rk4_state state = a_initialState;
					state.position += a_prevDerivativeState.position * a_dt;
					state.rotation = glm::normalize(state.rotation + a_prevDerivativeState.rotation * a_dt);
					state.linearVelocity += a_prevDerivativeState.linearVelocity * a_dt;
					state.angularVelocityLocal += a_prevDerivativeState.angularVelocityLocal * a_dt;

					return rk4_derivate(state);
				};

			auto const initialState = rk4_state{ solidPosition, solidRotation, solidLinearVelocity, solidAngularVelocityLocal };

			auto const k1 = rk4_derivate(initialState);
			auto const k2 = rk4_step(initialState, k1, integrationStepDuration / 2.0f);
			auto const k3 = rk4_step(initialState, k2, integrationStepDuration / 2.0f);
			auto const k4 = rk4_step(initialState, k3, integrationStepDuration);

			solidPosition += (integrationStepDuration / 6.0f) * (k1.position + 2.0f * k2.position + 2.0f * k3.position + k4.position);
			solidLinearVelocity += (integrationStepDuration / 6.0f) * (k1.linearVelocity + 2.0f * k2.linearVelocity + 2.0f * k3.linearVelocity + k4.linearVelocity);
			solidRotation = glm::normalize(solidRotation + (integrationStepDuration / 6.0f) * (k1.rotation + 2.0f * k2.rotation + 2.0f * k3.rotation + k4.rotation));
			solidAngularVelocityLocal += (integrationStepDuration / 6.0f) * (k1.angularVelocityLocal + 2.0f * k2.angularVelocityLocal + 2.0f * k3.angularVelocityLocal + k4.angularVelocityLocal);
		}

		static bool k_specialPause = false;
		if (!k_specialPause)
		{
			k_ellipsoidPosition = solidPosition;
			k_ellipsoidLinearVelocity = solidLinearVelocity;
			k_ellipsoidRotation = solidRotation;
			k_ellipsoidAngularVelocity = solidAngularVelocityLocal;
		}
	}
}