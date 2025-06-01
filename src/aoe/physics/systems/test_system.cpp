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

	void draw_triangle(aoegl::debug_mesh_world_component& a_debugMeshContext, glm::vec3 const& a_p0, glm::vec3 const& a_p1, glm::vec3 const& a_p2, aoegl::rgba const& a_color)
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

	void draw_ellipsoid(aoegl::debug_mesh_world_component& a_debugMeshContext, glm::mat4 const& a_transform, glm::vec3 const& a_radiuses, aoegl::rgba const& a_color)
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
		draw_ellipsoid(a_debugMeshContext, aoest::combine(a_position, glm::quat{}), glm::vec3{ a_radius }, a_color);
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

	void test_system::update_v1() const
	{
		static float k_elapsedTime = 0.0f;
		k_elapsedTime += std::clamp(m_simulationTimeContext->m_elapsedTime.get_value(), 0.0f, 1.0f / 30.0f);

		static bool k_useFixedTimeStep = true;
		//auto const dt = m_simulationTimeContext->m_elapsedTime.get_value() > 0.0f ? 1.0f / 300 : 0.0f;
		auto const dt = k_useFixedTimeStep ? (k_elapsedTime > 1.0f / 100 ? 1.0f / 100 : 0.0f) : k_elapsedTime;
		k_elapsedTime -= dt;

		// world
		static glm::vec3 ps[] = {
			glm::vec3{0.0f, 1.0f, 0.0f},
			glm::vec3{ 0.0f, 1.0f, 100.0f },
			glm::vec3{ 100.0f, 1.0f, 0.0f }
		};
		static auto ellasticity = 100'000.0f;
		static auto rollingFriction = 0.5f; // how sticky+soft surface prevent rolling
		static auto restitution = 0.1f;
		static auto friction = 1.0f;
		static auto staticFriction = 2.0f;
		static auto gravity = glm::vec3{ 0.0f, -25.0f, 0.0f };
		const triangle groundTriangle{ ps[0], ps[1], ps[2] };
		auto const groundNormal = glm::normalize(glm::cross(ps[1] - ps[0], ps[2] - ps[0]));

		// intersect_volume_unit_sphere_with_normal_plane_section(-0.01f, -0.9999f, 0.9999f);
		// intersect_volume_unit_sphere_with_triangle(triangle{ glm::vec3{-3.0f, -0.002f, -3.1f}, glm::vec3{-3.0f, -0.002f, 3.1f}, glm::vec3{3.1f, -0.002f, 0.0f} });

		draw_triangle(*m_debugMeshWorldComponent, ps[0], ps[1], ps[2], aoegl::k_eggplant);

		// ellipsoid
		static auto centerOfMassOffset = glm::vec3{ 0.0f };
		static auto radiuses = glm::vec3{ 1.0f, 1.f, 3.f };
		static auto mass = 10.0f;
		auto inertia = mass / 5.0f * glm::mat3{
			glm::vec3{radiuses.y * radiuses.y + radiuses.z * radiuses.z, 0.0f, 0.0f},
			glm::vec3{0.0f, radiuses.z * radiuses.z + radiuses.x * radiuses.x, 0.0f},
			glm::vec3{0.0f, 0.0f, radiuses.x * radiuses.x + radiuses.y * radiuses.y} };
		static auto integralStep = 1.0f / 1000.f;

		static auto startPosition = glm::vec3{ 5.0f, 20.0f, 5.0f };
		static auto startRotation = glm::vec3{ 2.0f, 3.1415926535f / 4, 0.0f };
		static auto startLinearVelocity = glm::vec3{ 0.0f, 0.0f, 0.0f };
		static auto startAngularVelocity = glm::vec3{ 0.0f, 0.0f, 0.0f };
		static auto position = startPosition;
		static auto rotation = glm::quat{ startRotation };
		static auto linearVelocity = glm::vec3{ 0.0f };
		static auto angularVelocity = glm::vec3{ 0.0f };
		static bool isGrounded = false;

		static std::optional<glm::vec3> lastHitPos = std::nullopt;
		static std::optional<glm::vec3> lastHitImpulse = std::nullopt;
		static std::optional<glm::vec3> lastHitContactVelocity = std::nullopt;
		static std::optional<glm::vec3> lastHitPosition = std::nullopt;
		static std::optional<glm::quat> lastHitRotation = std::nullopt;
		static glm::vec3 lastPenetrationTrianglePos = glm::vec3{ 0.0f };
		static glm::vec3 lastPenetrationPos = glm::vec3{ 0.0f };
		static glm::vec3 lastPenetrationNormal = glm::vec3{ 0.0f, 1.0f, 0.0f };
		static std::pair<float, glm::vec3> projectedHit = {};
		static float lastPenetrationTime = 0.0f;
		static float lastPenetrationDist = 0.0f;
		static float lastCoverRatio = 0.0f;
		if (m_inputs->keyboard.keys[aoein::keyboard::key::P].is_pressed())
		{
			linearVelocity = glm::vec3{ 0.0f };
			linearVelocity = startLinearVelocity;
			angularVelocity = startAngularVelocity;
			position = startPosition;
			rotation = glm::quat{ startRotation };
			isGrounded = false;
		}

		auto const pa0 = std::atan(0.0f);
		auto const na0 = std::atan(-0.0f);

		static std::optional<float> dragStart;

		draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(position, rotation), radiuses, aoegl::k_gray);

		static bool k_debugPenetration = false;
		if (k_debugPenetration)
		{
			draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(lastPenetrationPos, glm::quat{}), glm::vec3{ 0.10f }, aoegl::k_orange);
			draw_line(*m_debugMeshWorldComponent, lastPenetrationTrianglePos, lastPenetrationPos, aoegl::k_orange);
			draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(lastPenetrationPos, glm::quat{}), glm::vec3{ 0.10f }, aoegl::k_yellow);

			draw_line(*m_debugMeshWorldComponent, lastPenetrationTrianglePos, ps[0], aoegl::k_azure);
			draw_line(*m_debugMeshWorldComponent, lastPenetrationTrianglePos, ps[1], aoegl::k_azure);
			draw_line(*m_debugMeshWorldComponent, lastPenetrationTrianglePos, ps[2], aoegl::k_azure);
		}

		static bool k_debugLastHit = false;
		if (k_debugLastHit && lastHitPos.has_value())
		{
			draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(*lastHitPos, glm::quat{}), glm::vec3{ 0.05f }, aoegl::k_red);
			draw_line(*m_debugMeshWorldComponent, *lastHitPos, *lastHitPos + *lastHitImpulse / 50.0f, aoegl::k_orange);
			draw_line(*m_debugMeshWorldComponent, *lastHitPos, *lastHitPos + *lastHitContactVelocity, aoegl::k_chartreuse);

			draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(*lastHitPosition, *lastHitRotation), radiuses, aoegl::k_forest);
		}

		static bool k_debugProjectedHit = false;
		if (k_debugProjectedHit)
		{
			draw_sphere(*m_debugMeshWorldComponent, projectedHit.second, 0.05f, aoegl::k_maroon);

			auto const projectedHitEllipsoidPos = projectedHit.second + groundNormal * projectedHit.first;
			draw_line(*m_debugMeshWorldComponent, projectedHit.second, projectedHitEllipsoidPos, aoegl::k_maroon);

			auto toPoint = projectedHitEllipsoidPos - position;
			auto const pointVelocity = linearVelocity + glm::cross(angularVelocity, toPoint);
			draw_line(*m_debugMeshWorldComponent, projectedHitEllipsoidPos, projectedHitEllipsoidPos + pointVelocity, aoegl::k_yellow);
		}

		static bool k_debugVelocity = true;
		if (k_debugVelocity)
		{
			draw_line(*m_debugMeshWorldComponent, position, position + angularVelocity, aoegl::k_orange);
			draw_line(*m_debugMeshWorldComponent, position, position + linearVelocity, aoegl::k_azure);
		}

		ImGui::Begin("Test");

		ImGui::InputFloat3("P0", &ps[0].x);
		ImGui::InputFloat3("P1", &ps[1].x);
		ImGui::InputFloat3("P2", &ps[2].x);
		ImGui::InputFloat3("Gravity", &gravity.x);
		ImGui::InputFloat("Ellasticity", &ellasticity);
		ImGui::InputFloat("Restitution", &restitution);
		ImGui::InputFloat("Friction", &friction);
		ImGui::BeginDisabled();
		ImGui::InputFloat("Static Friction", &staticFriction);
		ImGui::EndDisabled();
		ImGui::InputFloat("Rolling Friction", &rollingFriction);
		ImGui::InputFloat("Integral Step", &integralStep);
		ImGui::Separator();
		ImGui::InputFloat3("Center Of Mass", &centerOfMassOffset.x);
		ImGui::InputFloat3("Radiuses", &radiuses.x);

		ImGui::Separator();
		ImGui::InputFloat3("Start Position", &startPosition.x, "%.6f");
		ImGui::InputFloat3("Start Rotation", &startRotation.x, "%.6f");
		ImGui::InputFloat3("Start Linear Velocity", &startLinearVelocity.x, "%.6f");
		ImGui::InputFloat3("Start Angular Velocity", &startAngularVelocity.x, "%.6f");
		ImGui::InputFloat3("Position", &position.x, "%.6f");
		auto eulerAngles = glm::eulerAngles(rotation);
		ImGui::InputFloat3("Rotation", &eulerAngles.x, "%.6f");
		rotation = glm::quat{ eulerAngles };
		if (false && m_simulationTimeContext->m_elapsedTime.get_value() > 0.0f)
		{
			ImGui::BeginDisabled();
		}
		ImGui::InputFloat3("Linear Velocity", &linearVelocity.x);
		ImGui::InputFloat3("Angular Velocity", &angularVelocity.x);
		if (true || m_simulationTimeContext->m_elapsedTime.get_value() <= 0.0f)
		{
			ImGui::BeginDisabled();
		}
		ImGui::InputFloat("Penetration Time", &lastPenetrationTime);
		ImGui::InputFloat("Penetration Dist", &lastPenetrationDist);
		ImGui::InputFloat("Cover Ratio", &lastCoverRatio);
		static float collisionEnergyLoss = 0.0f;
		float energy = calc_energy(mass, linearVelocity, inertia, angularVelocity, gravity, position);
		ImGui::InputFloat("Energy", &energy);
		ImGui::InputFloat("Collision Energy Loss", &collisionEnergyLoss);
		ImGui::Checkbox("Is Grounded", &isGrounded);
		ImGui::EndDisabled();

		ImGui::Separator();
		ImGui::Checkbox("Used Fixed Time Step", &k_useFixedTimeStep);
		ImGui::Checkbox("Debug Penetration", &k_debugPenetration);
		ImGui::Checkbox("Debug Last Hit", &k_debugLastHit);
		ImGui::Checkbox("Debug Projected Hit", &k_debugProjectedHit);
		ImGui::Checkbox("Debug Velocity", &k_debugVelocity);

		ImGui::End();

		if (m_inputs->mouse.buttons[aoein::mouse::button::Left].was_pressed() && m_inputs->keyboard.keys[aoein::keyboard::key::LControl].is_pressed())
		{
			if (m_inputs->keyboard.keys[aoein::keyboard::key::LShift].is_pressed())
			{
				angularVelocity += glm::vec3{ aoest::combine(position, rotation) * glm::vec4{1.0f, 0.0f, 0.0f, 0.0f} };
			}
			else
			{
				angularVelocity -= glm::vec3{ aoest::combine(position, rotation) * glm::vec4{1.0f, 0.0f, 0.0f, 0.0f} };
			}
			//float sign = m_inputs->keyboard.keys[aoein::keyboard::key::LShift].is_pressed() ? 1.0f : -1.0f;
			//glm::vec3 dW = glm::vec4{ sign * 10.0f, 0.0f, 0.0f, 0.0f };
			//angularVelocity += dW;
		}

		auto const transform = glm::scale(aoest::combine(position, rotation), radiuses);
		auto const invTransform = glm::inverse(transform);

		if (dt <= 0.0f)
		{
			return;
		}


		// 2. apply physics
		{
			auto integrate = [&](auto const maxTime)
				{
					auto z = 0.0f;
					auto pos = position;
					auto rot = rotation;
					auto V = linearVelocity;
					auto W = angularVelocity;

					auto const m = mass;
					auto const k = ellasticity;
					auto const period = std::sqrt(k / m);
					auto const b = 2.0f * mass * mass * (1.0f - restitution); // ellasticity : so .. it doesn't depend on time on ground?
					auto const r = friction;
					auto const N = groundNormal;
					auto const I = inertia;
					auto const iI = glm::inverse(I);

					auto const pe = calc_energy(mass, V, I, W, gravity, pos);

					auto S = 0.0f;
					auto const ds = integralStep;
					while (z <= 0.001f && S <= maxTime)
					{
						lastPenetrationDist = std::max(lastPenetrationDist, std::abs(z));

						S += ds;
						auto const Tr = aoest::combine(pos, rot);
						glm::mat4 const iTr = glm::inverse(Tr);
						auto const [newZ, planeP, ellipsoidP, coverRatio] = ellipsoid_intersect(glm::scale(Tr, radiuses), glm::inverse(glm::scale(Tr, radiuses)), groundTriangle);
						z = glm::dot(ellipsoidP - planeP, N);
						lastCoverRatio = coverRatio;

						auto const iIw = glm::mat3{ Tr } *iI * glm::inverse(glm::mat3{ Tr });

						lastPenetrationPos = ellipsoidP;
						lastPenetrationTrianglePos = planeP;

						auto const R = ellipsoidP - pos;
						auto const Vp = V + glm::cross(W, R);
						auto const Vpn = glm::dot(Vp, N) * N;
						auto const Vpt = Vp - Vpn;
						auto const Fpn = -k * z * N;
						auto const Fdamp = -b * Vpn * coverRatio; // not applied to angular momentum, else energy is just transfered

						// For now assuming static friction = 2 * dynamic friction, or something like that
						auto Fpt = glm::vec3{ 0.0f };
						auto const maxFrictionlessSpeed = 2.0f * r / m * glm::length(Fpn);
						if (glm::length(Vpt) != 0.0f)
						{
							Fpt = -glm::normalize(Vpt) * std::min(r * glm::length(Fpn), 0.5f * glm::length(Vpt) * m / ds);
						}

						auto const Fp = (Fpn + Fpt) * coverRatio;
						auto const dV = (gravity + (Fp + Fdamp) / m) * ds;

						auto const lFp = glm::transpose(glm::mat3{ Tr }) * Fp;
						auto const lR = glm::transpose(glm::mat3{ Tr }) * R;
						auto const lDw = iI * glm::cross(lR, lFp) * ds;
						auto const dW = glm::mat3{ Tr } *lDw;

						V = V + dV;
						W = W - (W * rollingFriction * ds) + dW;

						pos += V * ds;

						auto const theta = W * ds;
						auto const thetaMagnitude = glm::length(theta);
						if (thetaMagnitude > glm::epsilon<float>())
						{
							glm::vec3 axis = theta / thetaMagnitude;
							float halfTheta = thetaMagnitude / 2.0f;
							rot = glm::quat(std::cos(halfTheta), axis * std::sin(halfTheta)) * rot;
						}
					}

					isGrounded = z <= 0.001f;

					linearVelocity = V;
					angularVelocity = W;
					lastPenetrationTime = S;
					position = pos;
					rotation = rot;

					auto const ne = calc_energy(mass, V, I, W, gravity, pos);
					collisionEnergyLoss = ne - pe;
				};

			if (isGrounded)
			{
				integrate(dt);
			}
			else if (!isGrounded)
			{
				linearVelocity += gravity * dt;
				auto linearMove = linearVelocity * dt;
				auto angularMove = angularVelocity * dt;
				auto [hitTime, hitPos] = ellipsoid_move(position, rotation, radiuses, linearMove, angularMove, groundTriangle);
				if (0 <= hitTime && hitTime < 1.0f)
				{
					position += hitTime * linearMove;
					rotation = glm::quat{ hitTime * angularMove } *rotation;
					lastHitPosition = position;
					lastHitRotation = rotation;

					lastPenetrationDist = 0.0f;
					integrate(dt * (1.0f - hitTime));
					return;

					//{
					//	auto const I = inertia;
					//	auto const e = restitution;
					//	auto const V = linearVelocity;
					//	auto const W = angularVelocity;
					//	auto const R = hitPos - position;
					//	auto const Vp = V + glm::cross(W, R);
					//	auto const N = groundNormal;

					//	auto const J = -(1 + e) * glm::dot(Vp, N) / (1.0f / mass + glm::dot(N, glm::cross(glm::inverse(I) * glm::cross(R, N), N))) * N;

					//	linearVelocity += J / mass;
					//	angularVelocity += glm::inverse(I) * glm::cross(R, J);
					//	if (glm::dot(linearVelocity, groundNormal) < 0.0f)
					//	{
					//		//isGrounded = true;
					//	}

					//	lastHitContactVelocity = V + glm::cross(W, R);
					//	lastHitPos = hitPos;
					//	lastHitImpulse = J;
					//}

					//{
					//	auto const _I = inertia;
					//	auto const _Ii = glm::inverse(_I);
					//	auto const V = linearVelocity;
					//	auto const W = angularVelocity;
					//	auto const R = hitPos - position;
					//	auto const _R = glm::matrixCross3(R);

					//	auto const A = 2.0f*(V + glm::cross(W, R));
					//	auto const B = (1.0f / mass * glm::mat3{1.0f} + glm::transpose(_R) * _Ii * _R);
					//	glm::mat3 C(1.0f);
					//	auto const K = -glm::dot(A, A) / glm::dot(A, B * A) * A;

					//	auto const Kn = glm::dot(K, groundNormal) * groundNormal;
					//	auto const Kt = K - Kn;
					//	auto const J = restitution * Kn + friction * Kt;

					//	linearVelocity += J / mass;
					//	angularVelocity += _Ii * glm::cross(R, J);
					//	if (glm::dot(linearVelocity, groundNormal) < 0.0f)
					//	{
					//		//isGrounded = true;
					//	}

					//	lastHitContactVelocity = V + glm::cross(W, R);
					//	lastHitPos = hitPos;
					//	lastHitImpulse = J;

					//	return;
					//}

					//{
					//	auto const m = mass;
					//	auto const V0 = linearVelocity;
					//	auto const I = inertia;
					//	auto const W0 = angularVelocity;
					//	auto const A = m * glm::dot(V0, V0) + glm::dot(W0, I * W0);
					//	auto const B = W0 + m * glm::inverse(I) * V0;
					//	auto const C = I * W0 + m * V0;
					//	auto const D = (glm::dot(B, C) - A) / m;
					//	auto const E = -B - glm::inverse(I) * C;
					//	auto const F = (1.0f + m * glm::inverse(I));

					//	auto const a = F[1][1];
					//	auto const b = E[1];
					//	auto const c = F[0][0] * V0[0] * V0[0] + F[2][2] * V0[2] * V0[2] + E[0] * V0[0] + E[2] * V0[2] + D;

					//	auto const d = b * b - 4 * a * c;
					//	auto const r1 = (-b - std::sqrt(d)) / (2.0f * a);
					//	auto const r2 = (-b + std::sqrt(d)) / (2.0f * a);

					//	auto const V1 = glm::vec3{ V0[0], r2, V0[2] };
					//	auto const W1 = W0 + m * glm::inverse(I) * glm::cross(V0 - V1, glm::cross(glm::cross(hitPos-position, groundNormal), groundNormal));
					//	linearVelocity = V1;
					//	angularVelocity = W1;
					//	return;
					//}

					{
						auto const Vp0 = linearVelocity + glm::cross(angularVelocity, hitPos - position);
						auto const Vp0n = glm::dot(Vp0, groundNormal) * groundNormal;
						auto const Vp0t = Vp0 - Vp0n;


						auto const Jv1 = -mass * Vp0n - mass * friction * Vp0t;
						auto const R = hitPos - position;
						auto const ratio = glm::dot(R, -groundNormal) / glm::length(R);
						auto const J1 = -mass * (1.0f + restitution * ratio) * Vp0n;
						auto const J = J1;

						linearVelocity += J / mass;
						angularVelocity += glm::inverse(inertia) * glm::cross(R, J);

						lastHitContactVelocity = Vp0;
						lastHitPos = hitPos;
						lastHitImpulse = J;

						if (glm::dot(linearVelocity + dt * gravity, groundNormal) < 0.0f)
						{
							isGrounded = true;
						}
					}
				}
				else
				{
					position += linearMove;
					rotation = glm::quat{ angularMove } *rotation;
				}
			}
			else
			{
				// 1. force touch down
				auto const [hitT0, P0] = ellipsoid_cast(transform, invTransform, gravity, groundTriangle);
				auto const Rp0 = P0 - position;
				position += hitT0 * gravity;
				auto const pos0 = position;

				// 2. gravity lever
				auto const rollingFriction = 0.0f;
				auto const deltaAngularVelocity = glm::inverse(inertia) * glm::cross(Rp0, -mass * gravity * dt);
				angularVelocity = angularVelocity * std::pow(1.0f - rollingFriction, dt) + deltaAngularVelocity;
				rotation = rotation * glm::quat{ angularVelocity * dt };

				auto const frictionMove = -dt * glm::cross(angularVelocity, Rp0);
				position += frictionMove;

				auto const transform1 = glm::scale(aoest::combine(position, rotation), radiuses);
				auto const invTransform1 = glm::inverse(transform1);
				auto const [hitT1, P1] = ellipsoid_cast(transform1, invTransform1, -groundNormal, groundTriangle);

				linearVelocity += dt * gravity;
				auto const maxFall = glm::dot(linearVelocity * dt, -groundNormal);
				if (hitT1 < 0.0f || hitT1 < maxFall)
				{
					position -= hitT1 * groundNormal;
					auto const pos1 = position;
					linearVelocity = (pos1 - pos0) / dt;
				}
				else
				{
					position += linearVelocity * dt;
					isGrounded = false;
				}
			}
		}


		//auto const transform = aoest::combine(position, rotation);
		//auto const invTransform = glm::inverse(transform);

		//glm::vec3 totalForce = glm::vec3{ 0.0f };
		//glm::vec3 totalTorque = glm::vec3{ 0.0f };
		//auto applyForce = [&](glm::vec3 const& a_force, glm::vec3 const& a_pos)
		//	{
		//		totalForce += a_force;
		//		auto const relativePos = a_pos - position;
		//		totalTorque += 1.0f * glm::cross(relativePos, a_force);
		//	};
		//auto applyImpulse = [&](glm::vec3 const& a_impulse, glm::vec3 const& a_pos)
		//	{
		//		linearVelocity += (1.0f / mass) * a_impulse;
		//		auto const relativePos = a_pos - position;
		//		angularVelocity += 1.0f * glm::inverse(inertia) * glm::cross(relativePos, a_impulse);
		//	};

		//// 1. apply gravity force
		//applyForce(mass * gravity, position);

		//static auto kEnableDynamic = true;

		//// 2. apply repulsion force
		//auto const angularMove = angularVelocity * dt;
		//rotation = glm::quat{ rotation * angularMove } *rotation;
		//auto ellipsoidTransform = glm::scale(aoest::combine(position, rotation), radiuses);
		//auto invEllipsoidTransform = glm::inverse(ellipsoidTransform);
		//auto [penetrationDist, penetrationPos, penetrationInsidePos] = ellipsoid_intersect(ellipsoidTransform, invEllipsoidTransform, groundTriangle);
		//lastPenetrationDist = penetrationDist;
		//lastPenetrationPos = penetrationPos;
		//lastPenetrationInsidePos = penetrationInsidePos;
		//lastPenetrationNormal = glm::normalize(glm::cross(groundTriangle.m_p1 - groundTriangle.m_p0, groundTriangle.m_p2 - groundTriangle.m_p0));
		//auto linearMove = glm::vec3{ 0.0f };

		//if (penetrationDist < 0.0f)
		//{
		//	if (kEnableDynamic) position += (lastPenetrationPos - lastPenetrationInsidePos);
		//	auto const averageRadius = (radiuses.x + radiuses.y + radiuses.z) / 3.0f;
		//	auto const travelEstimate = -averageRadius * friction * glm::cross(angularMove, lastPenetrationNormal);
		//	if (kEnableDynamic) position += travelEstimate;
		//	// linearMove += (lastPenetrationPos - lastPenetrationInsidePos);
		//}
		//ellipsoidTransform = glm::scale(aoest::combine(position, rotation), radiuses);
		//invEllipsoidTransform = glm::inverse(ellipsoidTransform);

		//// 3. apply forces
		//if (kEnableDynamic) linearVelocity += totalForce * (1.0f / mass) * dt;
		//if (kEnableDynamic) angularVelocity += glm::inverse(inertia) * totalTorque * dt;

		//linearMove += linearVelocity * dt;

		//// 3. apply velocity
		//auto [hitTime, hitPos] = ellipsoid_cast(ellipsoidTransform, invEllipsoidTransform, linearMove, groundTriangle);

		//if (hitTime >= 0.0f && hitTime < 1.0f)
		//{
		//	lastHitPos = hitPos;
		//	lastHitTime = hitTime;
		//	if (kEnableDynamic) position = position + hitTime * linearMove;

		//	glm::vec3 const hitNormal = glm::normalize((position - hitPos) / (radiuses * radiuses));

		//	auto const localVelocity = linearVelocity + glm::cross(angularVelocity, hitPos - position);

		//	auto const localVelocityN = glm::dot(localVelocity, hitNormal) * hitNormal;
		//	auto const localVelocityT = localVelocity - localVelocityN;

		//	lastHitImpulse = hitNormal * glm::dot(hitNormal, -localVelocity) * (1.0f + restitution);
		//	lastVelocityAtImpact = localVelocity;
		//	// applyImpulse(*lastHitImpulse, hitPos);
		//	linearVelocity += -localVelocityN * (1 + restitution);
		//	angularVelocity += 0.2f * glm::cross(hitPos - position, -localVelocityN * restitution);
		//}
		//else
		//{
		//	if (kEnableDynamic) position = position + linearMove;
		//}
	}

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

	void test_system::update_v2() const
	{
		// 0. statics
		// A. time
		static auto k_useFixedTimeStep = true;
		static auto k_maxDeltaTime = 1.0f / 30.0f;
		static auto k_fixedTimeStep = 1.0f / 100.0f;
		static auto k_integrationTimeStep = 1.0f / 1000.0f;
		static auto k_simulationStep = 0;

		static auto k_unusedElapsedTime = 0.0f;
		static auto k_computationTime = 0.0f;
		auto const computationStartTime = std::chrono::high_resolution_clock().now();
		// B. world
		static auto k_gravity = glm::vec3{ 0.0f, -25.0f, 0.0f };
		static auto k_groundTriangles = std::vector<triangle>();
		static auto k_groundTriangle = triangle{
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 100.0f },
			glm::vec3{ 100.0f, 1.0f, 0.0f }
		};
		static auto k_groundTriangle2 = triangle{
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::vec3{ 0.0f, 100.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 100.0f }
		};
		static auto k_groundEllasticity = 100'000.0f;
		static auto k_groundRestitution = 0.1f;
		static auto k_groundFriction = 1.0f;
		static auto k_groundRollingFriction = 0.5f; // how sticky+soft surface prevent rolling
		// C. ellipsoid
		static auto k_ellipsoidRadiuses = glm::vec3{ 3.0f, 2.2f, 3.0f };
		static auto k_ellipsoidMass = 10.0f;
		static auto k_ellipsoidStartPosition = glm::vec3{ 5.0f, 4.0f, 5.0f };
		static auto k_ellipsoidStartRotation = glm::vec3{ 2.0f, 3.1415926535f / 4, 0.0f };
		static auto k_ellipsoidStartLinearVelocity = glm::vec3{ 0.0f, 0.0f, 0.0f };
		static auto k_ellipsoidStartAngularVelocity = glm::vec3{ 0.0f, 0.0f, 0.75f };

		static auto k_ellipsoidPosition = k_ellipsoidStartPosition;
		static auto k_ellipsoidRotation = glm::quat(k_ellipsoidStartRotation);
		static auto k_ellipsoidLinearVelocity = k_ellipsoidStartLinearVelocity;
		static auto k_ellipsoidAngularVelocity = k_ellipsoidStartAngularVelocity;
		static auto k_isEllipsoidGrounded = false;
		static auto k_ellipsoidEnergy = 0.0f;
		// D. physics
		static auto k_lastContactTrianglePosition = glm::vec3{ 0.0f };
		static auto k_lastContactEllipsoidPosition = glm::vec3{ 0.0f };
		static auto k_lastContactNormal = glm::vec3{ 0.0f };
		static auto k_maxIntegrationStepsDone = 0;

		if (m_inputs->keyboard.keys[aoein::keyboard::key::P].is_pressed())
		{
			k_ellipsoidLinearVelocity = k_ellipsoidStartLinearVelocity;
			k_ellipsoidAngularVelocity = k_ellipsoidStartAngularVelocity;
			k_ellipsoidPosition = k_ellipsoidStartPosition;
			k_ellipsoidRotation = glm::quat(k_ellipsoidStartRotation);
			k_isEllipsoidGrounded = false;
			k_maxIntegrationStepsDone = 0;
			k_simulationStep = 0;
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
		// ImGui::InputFloat("Integration Time Step", &k_integrationTimeStep);
		ImGui::BeginDisabled();
		ImGui::InputFloat("Computation Time", &k_computationTime);
		ImGui::InputInt("Step", &k_simulationStep);
		auto fps = 1000.0f * m_simulationTimeContext->m_elapsedTime.get_value();
		ImGui::InputFloat("Frame Time (ms)", &fps);
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

		ImGui::BeginDisabled();
		ImGui::InputFloat("Energy", &k_ellipsoidEnergy);
		ImGui::EndDisabled();

		ImGui::End();

		// 2. draw
		draw_triangle(*m_debugMeshWorldComponent, k_groundTriangle.m_p0, k_groundTriangle.m_p1, k_groundTriangle.m_p2, aoegl::to_rgba(glm::vec4{ 0.25f }));
		draw_triangle(*m_debugMeshWorldComponent, k_groundTriangle2.m_p0, k_groundTriangle2.m_p1, k_groundTriangle2.m_p2, aoegl::to_rgba(glm::vec4{ 0.25f }));
		draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses, aoegl::to_rgba(glm::vec4{ 0.5f }));
		draw_line(
			*m_debugMeshWorldComponent,
			k_lastContactEllipsoidPosition,
			k_lastContactEllipsoidPosition + (k_lastContactTrianglePosition - k_lastContactEllipsoidPosition) * 100.0f,
			aoegl::k_red);
		draw_sphere(*m_debugMeshWorldComponent, k_lastContactTrianglePosition, 0.125f, aoegl::k_red);
		draw_sphere(*m_debugMeshWorldComponent, k_lastContactEllipsoidPosition, 0.125f, aoegl::k_azure);

		// TODO draw hits

		// 3. physics
		k_unusedElapsedTime += std::clamp(m_simulationTimeContext->m_elapsedTime.get_value(), 0.0f, k_maxDeltaTime);
		auto const simulationTimeStep = k_useFixedTimeStep ? (k_unusedElapsedTime > k_fixedTimeStep ? k_fixedTimeStep : 0.0f) : std::min(k_unusedElapsedTime, k_maxDeltaTime);
		k_unusedElapsedTime = k_useFixedTimeStep ? k_unusedElapsedTime - simulationTimeStep : 0.0f;
		if (simulationTimeStep == 0.0f)
		{
			return;
		}
		++k_simulationStep;

		auto const ellipsoidTransform = glm::scale(aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses);
		auto const ellipsoidTransformInv = glm::inverse(ellipsoidTransform);

		{
			auto const ellipsoidInertia = k_ellipsoidMass / 5.0f * glm::mat3{
				glm::vec3{k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y + k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z, 0.0f, 0.0f},
				glm::vec3{0.0f, k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z + k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x, 0.0f},
				glm::vec3{0.0f, 0.0f, k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x + k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y} };

			k_ellipsoidEnergy = calc_energy(k_ellipsoidMass, k_ellipsoidLinearVelocity, ellipsoidInertia, k_ellipsoidAngularVelocity, k_gravity, k_ellipsoidPosition);

			auto integrate = [&](auto const maxTime)
				{
					auto z = 0.0f;
					auto pos = k_ellipsoidPosition;
					auto rot = k_ellipsoidRotation;
					auto V = k_ellipsoidLinearVelocity;
					auto W = k_ellipsoidAngularVelocity;

					auto const m = k_ellipsoidMass;
					auto const k = k_groundEllasticity;
					auto const b = 2.0f * k_ellipsoidMass * k_ellipsoidMass * (1.0f - k_groundRestitution); // ellasticity : so .. it doesn't depend on time on ground?
					auto const r = k_groundFriction;
					// auto const N = glm::normalize(glm::cross(k_groundTriangle.m_p1 - k_groundTriangle.m_p0, k_groundTriangle.m_p2 - k_groundTriangle.m_p0));
					auto const I = ellipsoidInertia;
					auto const iI = glm::inverse(I);

					auto S = 0.0f;
					auto const ds = k_integrationTimeStep;
					auto integrationStepsDone = 0;
					while (z <= 0.001f && S <= maxTime)
					{
						++integrationStepsDone;

						S += ds;
						auto const Tr = aoest::combine(pos, rot);
						glm::mat4 const iTr = glm::inverse(Tr);
						//auto const [newZ, ellipsoidP, triangleP] =
						//	ellipsoid_intersect2(glm::scale(Tr, k_ellipsoidRadiuses), glm::inverse(glm::scale(Tr, k_ellipsoidRadiuses)), k_groundTriangle);
						auto const result = _ellipsoid_intersect(Tr, glm::inverse(Tr), k_ellipsoidRadiuses, { k_groundTriangle, k_groundTriangle2 });
						if (result == std::nullopt)
						{
							V += k_gravity * (maxTime - S);
							pos += V * (maxTime - S);
							auto const theta = W * (maxTime - S);
							auto const thetaMagnitude = glm::length(theta);
							if (thetaMagnitude > glm::epsilon<float>())
							{
								glm::vec3 axis = theta / thetaMagnitude;
								float halfTheta = thetaMagnitude / 2.0f;
								rot = glm::quat(std::cos(halfTheta), axis * std::sin(halfTheta)) * rot;
							}

							break;
						}

						z = std::get<0>(*result);
						auto const trianglePoint = std::get<1>(*result);
						auto const ellipsoidPoint = std::get<2>(*result);
						auto const ellipsoidToTriangle = trianglePoint - ellipsoidPoint;
						auto const N = glm::dot(ellipsoidToTriangle, ellipsoidToTriangle) < glm::epsilon<float>() ?
							glm::normalize(ellipsoidPoint - pos) : glm::normalize(ellipsoidToTriangle);

						k_lastContactTrianglePosition = std::get<1>(*result);
						k_lastContactEllipsoidPosition = std::get<2>(*result);

						auto const iIw = glm::mat3{ Tr } *iI * glm::inverse(glm::mat3{ Tr });

						auto const R = std::get<2>(*result) - pos;
						auto const Vp = V + glm::cross(W, R);
						auto const Vpn = glm::dot(Vp, N) * N;
						auto const Vpt = Vp - Vpn;
						auto const Fpn = -k * z * N;
						auto const Fdamp = -b * Vpn; // not applied to angular momentum, else energy is just transfered

						// For now assuming static friction = 2 * dynamic friction, or something like that
						auto Fpt = glm::vec3{ 0.0f };
						auto const maxFrictionlessSpeed = 2.0f * r / m * glm::length(Fpn);
						if (glm::length(Vpt) != 0.0f)
						{
							Fpt = -glm::normalize(Vpt) * std::min(r * glm::length(Fpn), 0.5f * glm::length(Vpt) * m / ds);
						}

						auto const Fp = (Fpn + Fpt);
						auto const dV = (k_gravity + (Fp + Fdamp) / m) * ds;

						auto const lFp = glm::transpose(glm::mat3{ Tr }) * Fp;
						auto const lR = glm::transpose(glm::mat3{ Tr }) * R;
						auto const lDw = iI * glm::cross(lR, lFp) * ds;
						auto const dW = glm::mat3{ Tr } *lDw;

						V = V + dV;
						W = W - (W * k_groundRollingFriction * ds) + dW;

						pos += V * ds;

						auto const theta = W * ds;
						auto const thetaMagnitude = glm::length(theta);
						if (thetaMagnitude > glm::epsilon<float>()) // some friction constant?
						{
							glm::vec3 axis = theta / thetaMagnitude;
							float halfTheta = thetaMagnitude / 2.0f;
							rot = glm::quat(std::cos(halfTheta), axis * std::sin(halfTheta)) * rot;
						}
					}

					if (integrationStepsDone > k_maxIntegrationStepsDone)
					{
						k_maxIntegrationStepsDone = integrationStepsDone;
					}

					k_isEllipsoidGrounded = z <= 0.001f;

					k_ellipsoidLinearVelocity = V;
					k_ellipsoidAngularVelocity = W;
					k_ellipsoidPosition = pos;
					k_ellipsoidRotation = rot;
				};

			if (k_isEllipsoidGrounded)
			{
				integrate(simulationTimeStep);
			}
			else if (!k_isEllipsoidGrounded)
			{
				k_ellipsoidLinearVelocity += k_gravity * simulationTimeStep;
				auto linearMove = k_ellipsoidLinearVelocity * simulationTimeStep;
				auto angularMove = k_ellipsoidAngularVelocity * simulationTimeStep;
				auto [hitTime, hitPos] = _ellipsoid_move2(k_ellipsoidPosition, k_ellipsoidRotation, k_ellipsoidRadiuses, linearMove, angularMove, { k_groundTriangle, k_groundTriangle2 });
				if (0 <= hitTime && hitTime < 1.0f)
				{
					k_ellipsoidPosition += hitTime * linearMove;
					k_ellipsoidRotation = glm::quat(hitTime * angularMove) * k_ellipsoidRotation;

					integrate(simulationTimeStep * (1.0f - hitTime));
				}
				else
				{
					k_ellipsoidPosition += linearMove;
					k_ellipsoidRotation = glm::quat(angularMove) * k_ellipsoidRotation;
				}
			}
		}

		auto const computationStopTime = std::chrono::high_resolution_clock().now();
		k_computationTime = (computationStopTime - computationStartTime).count() * 0.001f;
	}







	static inline std::tuple<float, glm::vec3, glm::vec3> _unit_ellipsoid_intersect(glm::vec3 const& a_radiuses, triangle const& a_triangle)
	{
		// The "fake" variables refer to those for calculations done in the skewed space where ellispoid is the unit-sphere.
		auto const fakeP0 = a_triangle.m_p0 / a_radiuses;
		auto const fakeP1 = a_triangle.m_p1 / a_radiuses;
		auto const fakeP2 = a_triangle.m_p2 / a_radiuses;

		// 1. Ellipsoid's center is below triangle
		auto const fakeNormal = glm::normalize(glm::cross(fakeP1 - fakeP0, fakeP2 - fakeP0));
		if (glm::dot(fakeNormal, -fakeP0) < 0.0f)
		{
			return { 0.0f, glm::vec3{ 0.0f }, glm::vec3{ 0.0f } };
		}

		// 2. Ellipsoid is above triangle
		auto const fakePlaneDist = glm::dot(-fakeNormal, fakeP0 + fakeNormal);
		if (fakePlaneDist > 1.0f)
		{
			return { 0.0f, glm::vec3{ 0.0f }, glm::vec3{ 0.0f } };
		}

		// 3. Deepest point of triangle's plane inside ellipsoid belongs to the triangle
		auto const fakePlanePoint = -fakePlaneDist * fakeNormal;
		auto const normal = glm::normalize(glm::cross(a_triangle.m_p1 - a_triangle.m_p0, a_triangle.m_p2 - a_triangle.m_p0));
		auto const deepestEllipsoidPointInPlane = -fakeNormal * a_radiuses;
		auto const planePoint = deepestEllipsoidPointInPlane + glm::dot(a_triangle.m_p0 - deepestEllipsoidPointInPlane, normal) * normal;
		if (is_inside(planePoint, a_triangle))
		{
			return std::make_tuple(glm::dot(deepestEllipsoidPointInPlane - planePoint, normal), deepestEllipsoidPointInPlane, planePoint);
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
			return { 0.0f, fakeTrianglePoint * a_radiuses, fakeTrianglePoint * a_radiuses };
		}

		// 5. Rough approximation only valid near the ellipsoid's surface
		auto const t = std::sqrt(std::max(0.0f, 1.0f / glm::dot(fakeTrianglePoint, fakeTrianglePoint)));
		auto const trianglePoint = fakeTrianglePoint * a_radiuses;
		auto const ellipsoidPoint = trianglePoint * t;
		return std::make_tuple(-glm::distance(trianglePoint, ellipsoidPoint), ellipsoidPoint, trianglePoint);
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

	void test_system::update_v3() const
	{
		// 0. statics
		// A. time
		static auto k_useFixedTimeStep = true;
		static auto k_maxDeltaTime = 1.0f / 30.0f;
		static auto k_fixedTimeStep = 1.0f / 100.0f;
		static auto k_integrationTimeStep = 1.0f / 1000.0f;
		static auto k_simulationStep = 0;

		static auto k_unusedElapsedTime = 0.0f;
		static auto k_computationTime = 0.0f;
		auto const computationStartTime = std::chrono::high_resolution_clock().now();
		// B. world
		static auto k_gravity = glm::vec3{ 0.0f, -25.0f, 0.0f };
		static auto k_groundTriangles = std::vector<triangle>();
		static auto k_groundTriangle = triangle{
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 100.0f },
			glm::vec3{ 100.0f, 1.0f, 0.0f }
		};
		static auto k_groundTriangle2 = triangle{
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::vec3{ 0.0f, 100.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 100.0f }
		};
		static auto k_groundEllasticity = 100'000.0f;
		static auto k_groundRestitution = 0.1f;
		static auto k_groundFriction = 1.0f;
		static auto k_groundRollingFriction = 0.5f; // how sticky+soft surface prevent rolling
		// C. ellipsoid
		static auto k_ellipsoidRadiuses = glm::vec3{ 1.0f, 3.0f, 1.0f };
		static auto k_ellipsoidMass = 10.0f;
		static auto k_ellipsoidStartPosition = glm::vec3{ 5.0f, 10.0f, 5.0f };
		static auto k_ellipsoidStartRotation = glm::vec3{ 0.0f, 0.0, 0.0f };
		static auto k_ellipsoidStartLinearVelocity = glm::vec3{ 0.0f, 0.0f, 0.0f };
		static auto k_ellipsoidStartAngularVelocity = glm::vec3{ 0.0f, 0.0f, 3.0f };

		static auto k_ellipsoidPosition = k_ellipsoidStartPosition;
		static auto k_ellipsoidRotation = glm::quat(k_ellipsoidStartRotation);
		static auto k_ellipsoidLinearVelocity = k_ellipsoidStartLinearVelocity;
		static auto k_ellipsoidAngularVelocity = k_ellipsoidStartAngularVelocity;
		static auto k_isEllipsoidGrounded = false;
		static auto k_ellipsoidEnergy = 0.0f;
		// D. physics
		static auto k_lastContactNormal = glm::vec3{ 0.0f };
		static auto k_maxIntegrationStepsDone = 0;

		if (m_inputs->keyboard.keys[aoein::keyboard::key::P].is_pressed())
		{
			k_ellipsoidLinearVelocity = k_ellipsoidStartLinearVelocity;
			k_ellipsoidAngularVelocity = k_ellipsoidStartAngularVelocity;
			k_ellipsoidPosition = k_ellipsoidStartPosition;
			k_ellipsoidRotation = glm::quat(k_ellipsoidStartRotation);
			k_isEllipsoidGrounded = false;
			k_maxIntegrationStepsDone = 0;
			k_simulationStep = 0;
		}

		// E. Collisions
		static auto k_lastHitTrianglePoint = glm::vec3{ 0.0f };
		static auto k_lastHitEllipsoidPoint = glm::vec3{ 0.0f };
		static auto k_lastHitLinearVelocity = glm::vec3{ 0.0f };
		static auto k_lastHitAngularVelocity = glm::vec3{ 0.0f };

		// 1. imgui
		ImGui::Begin("Test System");

		ImGui::SeparatorText("Time Settings");
		ImGui::InputFloat("Max Delta Time", &k_maxDeltaTime);
		ImGui::Checkbox("Use Fixed Simulation Time Step", &k_useFixedTimeStep);
		if (k_useFixedTimeStep)
		{
			ImGui::InputFloat("Fixed Simulation Time Step", &k_fixedTimeStep);
		}
		ImGui::InputFloat("Integration Time Step", &k_integrationTimeStep);
		ImGui::BeginDisabled();
		ImGui::InputFloat("Computation Time", &k_computationTime);
		ImGui::InputInt("Step", &k_simulationStep);
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

		ImGui::BeginDisabled();
		ImGui::InputFloat("Energy", &k_ellipsoidEnergy);
		ImGui::InputFloat3("Linear Velocity", &k_ellipsoidLinearVelocity.x);
		ImGui::EndDisabled();

		ImGui::End();

		// 2. draw
		draw_triangle(*m_debugMeshWorldComponent, k_groundTriangle.m_p0, k_groundTriangle.m_p1, k_groundTriangle.m_p2, aoegl::to_rgba(glm::vec4{ 0.25f }));
		draw_triangle(*m_debugMeshWorldComponent, k_groundTriangle2.m_p0, k_groundTriangle2.m_p1, k_groundTriangle2.m_p2, aoegl::to_rgba(glm::vec4{ 0.25f }));
		draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses, aoegl::to_rgba(glm::vec4{ 0.5f }));
		draw_line(*m_debugMeshWorldComponent, k_lastHitEllipsoidPoint, k_lastHitEllipsoidPoint + k_lastHitLinearVelocity, aoegl::k_red);

		// TODO draw hits

		// 3. physics
		k_unusedElapsedTime += std::clamp(m_simulationTimeContext->m_elapsedTime.get_value(), 0.0f, k_maxDeltaTime);
		auto const simulationTimeStep = k_useFixedTimeStep ? (k_unusedElapsedTime > k_fixedTimeStep ? k_fixedTimeStep : 0.0f) : std::min(k_unusedElapsedTime, k_maxDeltaTime);
		k_unusedElapsedTime = k_useFixedTimeStep ? k_unusedElapsedTime - simulationTimeStep : 0.0f;
		if (simulationTimeStep == 0.0f)
		{
			return;
		}
		++k_simulationStep;

		k_ellipsoidLinearVelocity += k_gravity * simulationTimeStep * 0.5f;
		auto const linearMove = k_ellipsoidLinearVelocity * simulationTimeStep;
		auto const angularMove = k_ellipsoidAngularVelocity * simulationTimeStep;
		k_ellipsoidLinearVelocity += k_gravity * simulationTimeStep * 0.5f;

		auto const ellipsoidInertia = k_ellipsoidMass / 5.0f * glm::mat3{
			glm::vec3{k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y + k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z, 0.0f, 0.0f},
			glm::vec3{0.0f, k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z + k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x, 0.0f},
			glm::vec3{0.0f, 0.0f, k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x + k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y} };
		k_ellipsoidEnergy = calc_energy(k_ellipsoidMass, k_ellipsoidLinearVelocity, ellipsoidInertia, k_ellipsoidAngularVelocity, k_gravity, k_ellipsoidPosition);

		std::vector<std::pair<glm::mat4, aoegl::rgba>> dummyTransforms;
		std::int32_t dummyCount;
		auto [hitRatio, ellipsoidPoint, trianglePoint] = _ellipsoid_move(
			k_ellipsoidPosition, k_ellipsoidRotation, k_ellipsoidRadiuses, linearMove, angularMove, k_groundTriangle, dummyTransforms, dummyCount);

		if (hitRatio < 1.0f)
		{
			k_ellipsoidPosition = linearMove * hitRatio + k_ellipsoidPosition;
			k_ellipsoidRotation = glm::quat{ angularMove * hitRatio } * k_ellipsoidRotation;

			auto const localToGlobalTr = aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation);
			auto const globalToLocalTr = glm::inverse(localToGlobalTr);

			auto const hitNormal = -_ellipsoid_normal(k_ellipsoidPosition, k_ellipsoidRotation, -k_ellipsoidRadiuses, ellipsoidPoint);
			auto const hitOffset = ellipsoidPoint - k_ellipsoidPosition;
			auto const hitRotAxis = aoest::normalize_safe(glm::cross(hitOffset, hitNormal));

			// -r * sin(theta)
			auto const k = 0.4f;
			auto const kSqrt = std::sqrt(k);
			auto const s = glm::length(glm::cross(hitNormal, hitOffset));
			// TODO: inertia along hitRotAxis
			auto const I = ellipsoidInertia[0][0];
			auto const Vh = k_ellipsoidLinearVelocity - glm::cross(hitOffset, k_ellipsoidAngularVelocity);
			auto const Vhn = glm::dot(Vh, hitNormal);
			auto const Ve = k_ellipsoidLinearVelocity;
			auto const Ven = glm::dot(Ve, hitNormal);
			auto const m = k_ellipsoidMass;
			auto const Wer = glm::dot(k_ellipsoidAngularVelocity, hitRotAxis);
			auto const Fe = k * (Ven * Ven + I / m * Wer * Wer);

			auto const a = s * s + I / m;
			auto const b = 2 * s * kSqrt * Vhn;
			auto const c = k * Vhn * Vhn - Fe;

			auto const d = b * b - 4.0f * a * c;
			auto const dSqrt = std::sqrt(std::max(0.0f, d));
			auto const Wer0 = (-b - dSqrt) / (2.0f * a);
			auto const Wer1 = (-b + dSqrt) / (2.0f * a);

			auto const Ven0 = - kSqrt * Vhn - s * Wer0;
			auto const Ven1 = - kSqrt * Vhn - s * Wer1;

			auto const triangleNormal = glm::normalize(glm::cross(k_groundTriangle.m_p1 - k_groundTriangle.m_p0, k_groundTriangle.m_p2 - k_groundTriangle.m_p0));
			if (glm::dot(trianglePoint - ellipsoidPoint, triangleNormal) < 0.0f)
			{
				k_ellipsoidPosition += (trianglePoint - ellipsoidPoint);
			}

			// ouf
			if (glm::dot(Vh, hitNormal) < 0.0f)
			{
				if (Vhn < glm::dot(Ve, hitNormal))
				{
					k_ellipsoidLinearVelocity = (k_ellipsoidLinearVelocity - Ven * hitNormal) + Ven0 * hitNormal;
					k_ellipsoidAngularVelocity = (k_ellipsoidAngularVelocity - Wer * hitRotAxis) + Wer0 * hitRotAxis;
				}
				else
				{
					k_ellipsoidLinearVelocity = (k_ellipsoidLinearVelocity - Ven * hitNormal) + Ven1 * hitNormal;
					k_ellipsoidAngularVelocity = (k_ellipsoidAngularVelocity - Wer * hitRotAxis) + Wer1 * hitRotAxis;
				}


				k_lastHitEllipsoidPoint = ellipsoidPoint;
				k_lastHitLinearVelocity = Vh;
			}
			else
			{
				k_ellipsoidPosition = linearMove + k_ellipsoidPosition;
				k_ellipsoidRotation = glm::quat{ angularMove } *k_ellipsoidRotation;
			}
		}
		else
		{
			k_ellipsoidPosition = linearMove + k_ellipsoidPosition;
			k_ellipsoidRotation = glm::quat{ angularMove } * k_ellipsoidRotation;
		}
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

	void test_system::update_v4() const
	{

		// 0. statics
		// A. time
		static auto k_useFixedTimeStep = true;
		static auto k_maxDeltaTime = 1.0f / 30.0f;
		static auto k_fixedTimeStep = 1.0f / 100.0f;
		static auto k_integrationTimeStep = 1.0f / 400.0f;
		static auto k_simulationStep = 0;

		static auto k_unusedElapsedTime = 0.0f;
		static auto k_computationTime = 0.0f;
		auto const computationStartTime = std::chrono::high_resolution_clock().now();
		// B. world
		static auto k_gravity = glm::vec3{ 0.0f, -25.0f, 0.0f };
		static auto k_groundTriangles = std::vector<triangle>();
		static auto k_groundTriangle = triangle{
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 0.0f, 100.0f },
			glm::vec3{ 100.0f, 0.0f, 0.0f }
		};
		static auto k_groundTriangle2 = triangle{
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::vec3{ 0.0f, 100.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 100.0f }
		};
		static auto k_groundEllasticity = 100'000.0f;
		static auto k_groundRestitution = 0.5f;
		static auto k_groundFriction = 1.0f;
		static auto k_groundRollingFriction = 0.5f; // how sticky+soft surface prevent rolling
		// C. ellipsoid
		static auto k_ellipsoidRadiuses = glm::vec3{ 3.0f, 2.2f, 3.0f };
		static auto k_ellipsoidMass = 1.0f;
		static auto k_ellipsoidStartPosition = glm::vec3{ 10.0f, 4.0f, 10.0f };
		static auto k_ellipsoidStartRotation = glm::vec3{ 0.0f, 0.0f, 0.0f };
		static auto k_ellipsoidStartLinearVelocity = glm::vec3{ 0.0f, 0.0f, 0.0f };
		static auto k_ellipsoidStartAngularVelocity = glm::vec3{ 0.0f, 0.0f, 0.0f };

		static auto k_ellipsoidPosition = k_ellipsoidStartPosition;
		static auto k_ellipsoidRotation = glm::quat(k_ellipsoidStartRotation);
		static auto k_ellipsoidLinearVelocity = k_ellipsoidStartLinearVelocity;
		static auto k_ellipsoidAngularVelocity = k_ellipsoidStartAngularVelocity;
		static auto k_isEllipsoidGrounded = false;
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
			solid_shape::part{ glm::vec3{ -0.01553f, 0.36325f, -1.75357f } - glm::vec3{ 0.0f, 0.35f, 0.0f }, glm::quat(glm::vec3{glm::half_pi<float>(), glm::half_pi<float>(), 0.0f}), glm::vec3{0.385f, 0.905f, 0.283f}},
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

		if (m_inputs->keyboard.keys[aoein::keyboard::key::P].is_pressed())
		{
			k_ellipsoidLinearVelocity = k_ellipsoidStartLinearVelocity;
			k_ellipsoidAngularVelocity = k_ellipsoidStartAngularVelocity;
			k_ellipsoidPosition = k_ellipsoidStartPosition;
			k_ellipsoidRotation = glm::quat(k_ellipsoidStartRotation);
			k_isEllipsoidGrounded = false;
			k_maxIntegrationStepsDone = 0;
			k_simulationStep = 0;
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
		ImGui::InputFloat("Integration Time Step", &k_integrationTimeStep);
		ImGui::BeginDisabled();
		ImGui::InputFloat("Computation Time", &k_computationTime);
		ImGui::InputInt("Step", &k_simulationStep);
		auto fps = 1000.0f * m_simulationTimeContext->m_elapsedTime.get_value();
		ImGui::InputFloat("Frame Time (ms)", &fps);
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

		auto const frameTime = m_simulationTimeContext->m_elapsedTime.get_value();
		k_unusedElapsedTime += std::clamp(frameTime, 0.0f, k_maxDeltaTime);
		auto const isPaused = frameTime == 0.0f;
		if (isPaused)
		{
			k_unusedElapsedTime = 0;
		}
		auto const simulationTimeStep = k_useFixedTimeStep ? (k_unusedElapsedTime > k_fixedTimeStep ? k_fixedTimeStep : 0.0f) : std::min(k_unusedElapsedTime, k_maxDeltaTime);
		k_unusedElapsedTime = k_useFixedTimeStep ? k_unusedElapsedTime - simulationTimeStep : 0.0f;
		if (simulationTimeStep > 0.0f)
		{
			++k_simulationStep;
		}

		ImGui::SeparatorText("State");
		if (!isPaused)
		{
			ImGui::BeginDisabled();
		}

		ImGui::InputFloat3("Position", &k_ellipsoidPosition.x);
		auto ellipsoidEuler = glm::eulerAngles(k_ellipsoidRotation);
		ImGui::InputFloat3("Rotation", &ellipsoidEuler.x);
		k_ellipsoidRotation = glm::quat(ellipsoidEuler);
		ImGui::InputFloat3("Linear Velocity", &k_ellipsoidLinearVelocity.x);
		ImGui::InputFloat3("Angular Velocity", &k_ellipsoidAngularVelocity.x);

		if (isPaused)
		{
			ImGui::BeginDisabled();
		}
		ImGui::InputFloat("Energy", &k_ellipsoidEnergy);
		ImGui::Checkbox("Is Grounded", &k_isEllipsoidGrounded);
		ImGui::InputFloat("Max Y", &k_yMax);
		ImGui::EndDisabled();
		static bool k_rk4 = false;
		ImGui::Checkbox("RK4", &k_rk4);

		ImGui::End();

		// 2. draw
		draw_triangle(*m_debugMeshWorldComponent, k_groundTriangle.m_p0, k_groundTriangle.m_p1, k_groundTriangle.m_p2, aoegl::to_rgba(glm::vec4{ 0.25f }));
		draw_triangle(*m_debugMeshWorldComponent, k_groundTriangle2.m_p0, k_groundTriangle2.m_p1, k_groundTriangle2.m_p2, aoegl::to_rgba(glm::vec4{ 0.25f }));
		// vob draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses, aoegl::to_rgba(glm::vec4{ 0.5f }));
		draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses, aoegl::to_rgba(glm::vec4{ 0.4f, 0.1f, 0.1f, 1.0f }));

		// draw solid
		for (auto const& solidPart : k_solidShape.parts)
		{
			draw_ellipsoid(
				*m_debugMeshWorldComponent,
				aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation) * aoest::combine(solidPart.position, solidPart.rotation),
				solidPart.radiuses,
				// vob aoegl::to_rgba(glm::vec4{ 0.4f, 0.1f, 0.1f, 1.0f }));
				aoegl::to_rgba(glm::vec4{ 0.5f }));
		}
		draw_line(*m_debugMeshWorldComponent, k_ellipsoidPosition, k_ellipsoidPosition + k_ellipsoidLinearVelocity, aoegl::k_green);
		draw_line(*m_debugMeshWorldComponent, k_ellipsoidPosition, k_ellipsoidPosition + k_ellipsoidAngularVelocity, aoegl::k_red);

		// draw hits
		/*draw_line(
			*m_debugMeshWorldComponent,
			k_lastContactEllipsoidPosition,
			k_lastContactEllipsoidPosition + (k_lastContactTrianglePosition - k_lastContactEllipsoidPosition) * 100.0f,
			aoegl::k_red);
		draw_sphere(*m_debugMeshWorldComponent, k_lastContactTrianglePosition, 0.125f, aoegl::k_red);
		draw_sphere(*m_debugMeshWorldComponent, k_lastContactEllipsoidPosition, 0.125f, aoegl::k_azure);*/
		draw_line(*m_debugMeshWorldComponent, k_lastContactEllipsoidPosition, k_lastContactEllipsoidPosition + k_lastContactForce, aoegl::k_orange);

		// 3. physics
		auto const ellipsoidTransform = glm::scale(aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses);
		auto const ellipsoidTransformInv = glm::inverse(ellipsoidTransform);

		if (true)
		{
			auto const k_ellipsoidInertia = k_ellipsoidMass / 5.0f * glm::diagonal3x3(glm::vec3{
				k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y + k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z,
				k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z + k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x,
				k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x + k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y });
			auto const k_ellipsoidInertiaInv = glm::inverse(k_ellipsoidInertia);

			auto dt = simulationTimeStep;
			auto const dampenerFactor = -std::log(k_groundRestitution) * std::sqrt(k_ellipsoidMass * k_groundEllasticity) / std::numbers::pi_v<float>;
			auto const gravityForce = k_gravity.y * k_ellipsoidMass;
			while (dt > 0.0f)
			{
				auto const ds = std::min(dt, std::max(0.0001f, k_integrationTimeStep));

				if (false)
				{
					auto const f1 = [&](auto const _t, auto const p, auto const r, auto const v, auto const w)
						{
							return std::make_pair(v, glm::quat{ w });
						};

					auto const f2 = [&](auto const _t, auto const p, auto const r, auto const v, auto const w)
						{
							auto const t = aoest::combine(p, r);
							auto const [unitDist, ellipsoidPoint, trianglePoint] = _ellipsoid_intersect(p, r, k_ellipsoidRadiuses, k_groundTriangle);
							if (unitDist >= 0.0f)
							{
								return std::make_pair(k_gravity, glm::vec3{ 0.0f });
							}

							auto const lever = ellipsoidPoint - p;
							auto const hitPointVelocity = v + glm::cross(w, lever);
							auto const ellipsoidToTriangle = trianglePoint - ellipsoidPoint;
							auto const penetrationDistSq = glm::dot(ellipsoidToTriangle, ellipsoidToTriangle);
							auto const hitNormal = penetrationDistSq > glm::epsilon<float>()
								? glm::normalize(ellipsoidToTriangle) : glm::normalize(ellipsoidPoint - p);
							auto const hitPointNormalVelocity = glm::dot(hitPointVelocity, hitNormal) * hitNormal;
							auto const hitPointTangentVelocity = hitPointVelocity - hitPointNormalVelocity;
							auto const penetrationDist = std::sqrt(penetrationDistSq);

							auto const gravityForce = k_gravity * k_ellipsoidMass;
							auto const springForce = k_groundEllasticity * penetrationDist * hitNormal;
							auto const dampenerForce = -2.0f * k_ellipsoidMass * k_ellipsoidMass * (1.0f - k_groundRestitution) * hitPointNormalVelocity;
							auto const frictionForce = glm::dot(hitPointTangentVelocity, hitPointTangentVelocity) > glm::epsilon<float>()
								? -glm::normalize(hitPointTangentVelocity)
								* std::min(k_groundFriction * glm::length(springForce), 0.5f * glm::length(hitPointTangentVelocity) * k_ellipsoidMass)
								: glm::vec3{ 0.0f };

							auto const hitForce = gravityForce + springForce + dampenerForce + frictionForce;

							auto const localHitReaction = glm::transpose(glm::mat3{ t }) * springForce;
							auto const localLever = glm::transpose(glm::mat3{ t }) * lever;

							return std::make_pair(hitForce / k_ellipsoidMass, glm::mat3{ t } *k_ellipsoidInertiaInv * glm::cross(localLever, localHitReaction));
						};

					auto const hds = ds / 2.0f;
					auto const p = k_ellipsoidPosition;
					auto const r = k_ellipsoidRotation;
					auto const v = k_ellipsoidLinearVelocity;
					auto const w = k_ellipsoidAngularVelocity;
					auto const [kv1, kw1] = f1(0.0f, p, r, v, w);
					auto const [kdv1, kdw1] = f2(0.0f, p, r, v, w);
					auto const [kv2, kw2] = f1(hds, p + hds * kv1, (hds * kw1) * r, v + hds * kdv1, w + hds * kdw1);
					auto const [kdv2, kdw2] = f2(hds, p + hds * kv1, (hds * kw1) * r, v + hds * kdv1, w + hds * kdw1);
					auto const [kv3, kw3] = f1(hds, p + hds * kv2, (hds * kw2) * r, v + hds * kdv2, w + hds * kdw2);
					auto const [kdv3, kdw3] = f2(hds, p + hds * kv2, (hds * kw2) * r, v + hds * kdv2, w + hds * kdw2);
					auto const [kv4, kw4] = f1(ds, p + ds * kv3, (ds * kw3) * r, v + ds * kdv3, w + ds * kdw3);
					auto const [kdv4, kdw4] = f2(ds, p + ds * kv3, (ds * kw3) * r, v + ds * kdv3, w + ds * kdw3);

					k_ellipsoidPosition = p + ds / 6.0f * (kv1 + 2.0f * kv2 + 2.0f * kv3 + kv4);
					k_ellipsoidRotation = glm::quat(ds / 6.0f * (kw1 + 2.0f * kw2 + 2.0f * kw3 + kw4)) * r;
					k_ellipsoidLinearVelocity = v + ds / 6.0f * (kdv1 + 2.0f * kdv2 + 2.0f * kdv3 + kdv4);
					k_ellipsoidAngularVelocity = w + ds / 6.0f * (kdw1 + 2.0f * kdw2 + 2.0f * kdw3 + kdw4);

					dt -= ds;
				}
				else
				{
					dt -= ds;

					// Runge Kutta 4
					auto const f = [&](auto const t, auto ly, auto lv)
						{
							auto const z = ly - k_ellipsoidRadiuses.y;
							auto const springForce = z < 0.0f ? -k_groundEllasticity * z : 0.0f;
							auto const dampenerForce = z < 0.0f ? -dampenerFactor * lv : 0.0f;
							return (springForce + dampenerForce + gravityForce) / k_ellipsoidMass;
						};

					auto const v = k_ellipsoidLinearVelocity.y;
					auto const y = k_ellipsoidPosition.y;

					auto const C0 = v;
					auto const K0 = f(0.0f, y, C0);
					auto const C1 = v + ds / 2.0f * K0;
					auto const K1 = f(0.0f + ds / 2.0f, y + ds / 2.0f * C0, C1);
					auto const C2 = v + ds / 2.0f * K1;
					auto const K2 = f(0.0f + ds / 2.0f, y + ds / 2.0f * C1, C2);
					auto const C3 = v + ds * K2;
					auto const K3 = f(0.0f + ds, y + ds * C2, C3);

					if (k_rk4)
					{
						auto const dy = ds / 6.0f * (C0 + 2.0f * C1 + 2.0f * C2 + C3);
						auto const dv = ds / 6.0f * (K0 + 2.0f * K1 + 2.0f * K2 + K3);

						k_ellipsoidLinearVelocity.y += dv;
						k_ellipsoidPosition.y += dy;
					}
					else
					{
						auto const dy = ds * C0;
						auto const dv = ds * K0;

						k_ellipsoidLinearVelocity.y += dv;
						k_ellipsoidPosition.y += dy;
					}
				}
			}

			if (k_y0 <= k_y1 && k_y1 > k_ellipsoidPosition.y)
			{
				k_yMax = k_y1 - k_ellipsoidRadiuses.y;
			}

			k_y0 = k_y1;
			k_y1 = k_ellipsoidPosition.y;

			return;
		}

		// for each solid
		auto& solidShape = k_solidShape;
		auto& solidIsGrounded = k_isEllipsoidGrounded;
		auto& solidPosition = k_ellipsoidPosition;
		auto& solidRotation = k_ellipsoidRotation;
		auto& solidLinearVelocity = k_ellipsoidLinearVelocity;
		auto& solidAngularVelocity = k_ellipsoidAngularVelocity;
		auto& solidMass = k_ellipsoidMass;
		auto const& solidInertia = solidMass / 5.0f * glm::mat3{
			glm::vec3{0.5f * 0.5f + 2.0f * 2.0f, 0.0f, 0.0f},
			glm::vec3{0.0f, 2.0f * 2.0f + 1.0f * 1.0f, 0.0f},
			glm::vec3{0.0f, 0.0f, 1.0f * 1.0f + 0.5f * 0.5f} };
		auto solidDampenerStrength = 2.0f * solidMass * solidMass * (1.0f - k_groundRestitution);
		{
			auto dt = simulationTimeStep;
			auto const invertedSolidInertia = glm::inverse(solidInertia);

			if (isPaused)
			{
				auto solidTransform = aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation);
				for (auto& solidShapePart : solidShape.parts)
				{
					// TODO: For each triangle
					auto const triangle = k_groundTriangle;

					auto ellipsoidPos = cleanup(solidTransform * glm::vec4{ solidShapePart.position, 1.0f });
					auto ellipsoidRot = k_ellipsoidRotation * solidShapePart.rotation;
					auto const [unitDist, ellipsoidPoint, trianglePoint] = _ellipsoid_intersect(
						ellipsoidPos, ellipsoidRot, solidShapePart.radiuses, triangle);
					if (unitDist >= 0.0f)
					{
						continue;
					}

					// draw_line(*m_debugMeshWorldComponent, ellipsoidPoint, ellipsoidPoint + (trianglePoint - ellipsoidPoint) * 10.0f, aoegl::k_red);
					// draw_sphere(*m_debugMeshWorldComponent, trianglePoint, 0.01f, aoegl::k_red);
					// draw_sphere(*m_debugMeshWorldComponent, ellipsoidPoint, 0.01f, aoegl::k_azure);

					auto const barycenterPoint = cleanup(solidTransform * glm::vec4{ solidShape.barycenter, 1.0f });
					auto const leverVector = ellipsoidPoint - barycenterPoint;
					auto const hitPointVelocity = solidLinearVelocity + glm::cross(solidAngularVelocity, leverVector);
					// draw_line(*m_debugMeshWorldComponent, ellipsoidPoint, ellipsoidPoint + (hitPointVelocity) * 10.0f, aoegl::k_red);

					auto const ellipsoidToTriangle = trianglePoint - ellipsoidPoint;
					auto const penetrationDistSq = glm::dot(ellipsoidToTriangle, ellipsoidToTriangle);
					auto const hitNormal = glm::dot(ellipsoidToTriangle, ellipsoidToTriangle) < glm::epsilon<float>()
						? glm::normalize(ellipsoidPoint - ellipsoidPos) : glm::normalize(ellipsoidToTriangle);
					auto const hitPointVelocityNormal = glm::dot(hitPointVelocity, hitNormal) * hitNormal;
					auto const hitPointVelocityTangent = hitPointVelocity - hitPointVelocityNormal;
					auto const penetrationDist = std::sqrt(penetrationDistSq);
					auto const springForce = k_groundEllasticity * penetrationDist * hitNormal;
					auto const dampenerForce = -solidDampenerStrength * hitPointVelocityNormal;
					auto frictionForce = glm::vec3{ 0.0f };
					if (glm::length(hitPointVelocityTangent) != 0.0f)
					{
						// TODO: where is dampenerForce here?
						frictionForce = -glm::normalize(hitPointVelocityTangent)
							* std::min(k_groundFriction * glm::length(springForce), 0.5f * glm::length(hitPointVelocityTangent) * solidMass / k_integrationTimeStep);
					}

					auto const hitForce = springForce + dampenerForce + frictionForce;
					auto const dV = (k_gravity + hitForce / solidMass) * k_integrationTimeStep;
					draw_line(*m_debugMeshWorldComponent, ellipsoidPoint, ellipsoidPoint + hitForce * 0.01f, aoegl::k_eggplant);

					auto const localHitForce = glm::transpose(glm::mat3{ solidTransform }) * hitForce;
					auto const localLeverVector = glm::transpose(glm::mat3{ solidTransform }) * leverVector;
					auto const localDW = invertedSolidInertia * glm::cross(localLeverVector, localHitForce) * k_integrationTimeStep;
					auto const dW = glm::mat3{ solidTransform } *localDW;

				}

				return;
			}
			else if (k_rk4)
			{
				struct rk4_source
				{
					solid_shape::part m_part;

					triangle m_triangle;
				};

				while (dt > 0.0f)
				{
					auto solidTransform = aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation);
					if (solidIsGrounded)
					{
						auto const ds = std::min(dt, k_integrationTimeStep);
						std::vector<rk4_source> rk4Sources;

						for (auto& solidShapePart : solidShape.parts)
						{
							// TODO: For each triangle
							auto const triangle = k_groundTriangle;
							auto ellipsoidPos = cleanup(solidTransform * glm::vec4{ solidShapePart.position, 1.0f });
							auto ellipsoidRot = solidRotation * solidShapePart.rotation;
							auto const [unitDist, ellipsoidPoint, trianglePoint] = _ellipsoid_intersect(
								ellipsoidPos, ellipsoidRot, solidShapePart.radiuses, triangle);
							if (unitDist > glm::epsilon<float>())
							{
								continue;
							}

							rk4Sources.emplace_back(solidShapePart, triangle);
						}

						if (rk4Sources.empty())
						{
							solidIsGrounded = false;
						}
						else
						{
							auto const rk4_step = [&](auto const t, auto const p, auto const r, auto const lv, auto const av)
								{
									auto const solidTransform = aoest::combine(p, r);
									auto dlv = glm::vec3{ 0.0f };
									auto dav = glm::vec3{ 0.0f };
									auto ddlv = glm::vec3{ 0.0f };
									auto ddav = glm::vec3{ 0.0f };

									for (auto const& rk4Source : rk4Sources)
									{
										auto const pos = cleanup(solidTransform * glm::vec4{ rk4Source.m_part.position, 1.0f });
										auto const rot = r * rk4Source.m_part.rotation;

										auto const [unitDist, ellipsoidPoint, trianglePoint] = _ellipsoid_intersect(
											pos, rot, rk4Source.m_part.radiuses, rk4Source.m_triangle);
										if (unitDist >= 0.0f)
										{
											continue;
										}

										auto const lever = ellipsoidPoint - p;
										auto const hitPointVelocity = lv + glm::cross(av, lever);
										auto const ellipsoidToTriangle = trianglePoint - ellipsoidPoint;
										auto const penetrationDistSq = glm::dot(ellipsoidToTriangle, ellipsoidToTriangle);
										auto const hitNormal = glm::dot(ellipsoidToTriangle, ellipsoidToTriangle) < glm::epsilon<float>()
											? glm::normalize(ellipsoidPoint - pos) : glm::normalize(ellipsoidToTriangle);
										auto const hitPointNormalVelocity = glm::dot(hitPointVelocity, hitNormal) * hitNormal;
										auto const hitPointTangentVelocity = hitPointVelocity - hitPointNormalVelocity;
										auto const penetrationDist = std::sqrt(penetrationDistSq);

										auto const springForce = k_groundEllasticity * penetrationDist * hitNormal;
										auto const dampenerForce = -solidDampenerStrength * hitPointNormalVelocity;
										auto frictionForce = glm::vec3{ 0.0f };
										if (glm::length(hitPointTangentVelocity) > glm::epsilon<float>())
										{
											// TODO: friction should include dampenerForce?
											frictionForce = -glm::normalize(hitPointTangentVelocity)
												* std::min(k_groundFriction * glm::length(springForce), 0.5f * glm::length(hitPointTangentVelocity) * solidMass / t);
										}

										auto const hitForce = springForce + dampenerForce + frictionForce;

										dlv += (k_gravity + hitForce / solidMass) * t / 2.0f;

										ddlv += k_gravity + hitForce / solidMass;

										auto const localHitForce = glm::transpose(glm::mat3{ solidTransform }) * hitForce;
										auto const localLever = glm::transpose(glm::mat3{ solidTransform }) * lever;
										auto const localDav = invertedSolidInertia * glm::cross(localLever, localHitForce);
										dav += glm::mat3{ solidTransform } *localDav * t / 2.0f;

										ddav += glm::mat3{ solidTransform } *localDav;
									}

									return std::make_pair(ddlv, ddav);
								};

							auto const t0 = 0.0f;
							auto const lv0 = solidLinearVelocity;
							auto const av0 = solidAngularVelocity;
							auto const p0 = solidPosition;
							auto const r0 = solidRotation;
							auto const [klv0, kav0] = rk4_step(t0, p0, r0, lv0, av0);
							auto const t1 = ds / 2.0f;
							auto const lv1 = solidLinearVelocity + t1 * klv0;
							auto const av1 = solidAngularVelocity + t1 * kav0;
							auto const p1 = solidPosition + t1 * lv1;
							auto const r1 = glm::quat{ t1 * av1 } *solidRotation;
							auto const [klv1, kav1] = rk4_step(t1, p1, r1, lv1, av1);
							auto const t2 = ds / 2.0f;
							auto const lv2 = solidLinearVelocity + t2 * klv1;
							auto const av2 = solidAngularVelocity + t2 * kav1;
							auto const p2 = solidPosition + t2 * lv2;
							auto const r2 = glm::quat{ t2 * av2 } *solidRotation;
							auto const [klv2, kav2] = rk4_step(t2, p2, r2, lv2, av2);
							auto const t3 = ds;
							auto const lv3 = solidLinearVelocity + t3 * klv2;
							auto const av3 = solidAngularVelocity + t3 * kav2;
							auto const p3 = solidPosition + t3 * lv3;
							auto const r3 = glm::quat{ t3, av3 } *solidRotation;
							auto const [klv3, kav3] = rk4_step(t3, p3, r3, lv3, av3);

							auto const dp = ds / 6.0f * (lv0 + 2.0f * lv1 + 2.0f * lv2 + lv3);
							auto const dr = ds / 6.0f * (av0 + 2.0f * av1 + 2.0f * av2 + av3);
							auto const dlv = ds / 6.0f * (klv0 + 2.0f * klv1 + 2.0f * klv2 + klv3);
							auto const dav = ds / 6.0f * (kav0 + 2.0f * kav1 + 2.0f * kav2 + kav3);

							k_ellipsoidPosition += dp;
							k_ellipsoidRotation = glm::quat{ dr } *k_ellipsoidRotation;
							k_ellipsoidLinearVelocity += dlv;
							k_ellipsoidAngularVelocity += dav;

							dt -= ds;
						}
					}

					if (!solidIsGrounded)
					{
						solidLinearVelocity += k_gravity * dt;
						auto linearMove = solidLinearVelocity * dt;
						auto angularMove = solidAngularVelocity * dt;

						auto closestHitTime = 1.0f;
						for (auto& solidShapePart : solidShape.parts)
						{
							auto ellipsoidPos = cleanup(solidTransform * glm::vec4{ solidShapePart.position, 1.0f });
							auto ellipsoidRot = k_ellipsoidRotation * solidShapePart.rotation;

							std::vector<std::pair<glm::mat4, aoegl::rgba>> _testTransforms;
							std::int32_t _testCount;
							auto const [hitTime, ellipsoidPoint, trianglePoint] = _ellipsoid_move(
								ellipsoidPos, ellipsoidRot, solidShapePart.radiuses, linearMove, angularMove, k_groundTriangle, _testTransforms, _testCount);
							if (0 <= hitTime && hitTime < closestHitTime)
							{
								closestHitTime = hitTime;
							}
						}

						// clamp to k_integrationTimeStep to make sure we are inside collision when integrating forces
						auto const cheatedClosestHitTime = std::min(1.0f, closestHitTime + k_integrationTimeStep / dt);

						k_ellipsoidPosition += cheatedClosestHitTime * linearMove;
						k_ellipsoidRotation = glm::quat(cheatedClosestHitTime * angularMove) * k_ellipsoidRotation;
						dt -= cheatedClosestHitTime * dt;
						if (closestHitTime < 1.0f)
						{
							solidIsGrounded = true;
						}
					}
				}
				return;
			}
			else
			{
				while (dt > 0.0f)
				{
					auto solidTransform = aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation);
					if (solidIsGrounded)
					{
						auto deepestPenetrationDistSq = -1.0f;
						auto deepestEllipsoidPos = glm::vec3{ 0.0f };
						auto deepestEllipsoidPoint = glm::vec3{ 0.0f };
						auto deepestTrianglePoint = glm::vec3{ 0.0f };
						auto deepestEllipsoidRot = glm::quat{};
						auto const triangle = k_groundTriangle;
						for (auto& solidShapePart : solidShape.parts) // & triangle pairs
						{
							// TODO: for each triangle

							auto ellipsoidPos = cleanup(solidTransform * glm::vec4{ solidShapePart.position, 1.0f });
							auto ellipsoidRot = k_ellipsoidRotation * solidShapePart.rotation;
							auto const [unitDist, ellipsoidPoint, trianglePoint] = _ellipsoid_intersect(
								ellipsoidPos, ellipsoidRot, solidShapePart.radiuses, triangle);
							if (unitDist >= 0.0f)
							{
								continue;
							}

							auto const ellipsoidPointToTrianglePoint = trianglePoint - ellipsoidPoint;
							auto const penetrationDistSq = glm::dot(ellipsoidPointToTrianglePoint, ellipsoidPointToTrianglePoint);
							if (penetrationDistSq < deepestPenetrationDistSq)
							{
								continue;
							}

							deepestPenetrationDistSq = penetrationDistSq;
							deepestEllipsoidPos = ellipsoidPos;
							deepestEllipsoidPoint = ellipsoidPoint;
							deepestTrianglePoint = trianglePoint;
							deepestEllipsoidRot = ellipsoidRot;
						}

						auto const ds = std::min(dt, k_integrationTimeStep);
						if (deepestPenetrationDistSq < 0.0f)
						{
							auto const dV = k_gravity * ds;
							solidLinearVelocity = solidLinearVelocity + dV;

							k_ellipsoidPosition += solidLinearVelocity * ds;
							solidIsGrounded = false;
						}
						else
						{
							k_lastContactTrianglePosition = deepestTrianglePoint;
							k_lastContactEllipsoidPosition = deepestEllipsoidPoint;
							auto const invertedSolidInertia = glm::inverse(solidInertia);

							auto const ellipsoidToTriangle = deepestTrianglePoint - deepestEllipsoidPoint;
							auto const hitNormal = glm::dot(ellipsoidToTriangle, ellipsoidToTriangle) < glm::epsilon<float>()
								? glm::normalize(deepestEllipsoidPoint - deepestEllipsoidPos) : glm::normalize(ellipsoidToTriangle);
							auto const penetrationDist = std::sqrt(deepestPenetrationDistSq);

							auto const barycenterPoint = cleanup(solidTransform * glm::vec4{ solidShape.barycenter, 1.0f });
							// auto const leverVector = deepestTrianglePoint - barycenterPoint;
							auto const leverVector = deepestEllipsoidPoint - barycenterPoint;
							auto const hitPointVelocity = solidLinearVelocity + glm::cross(solidAngularVelocity, leverVector);
							auto const hitPointVelocityNormal = glm::dot(hitPointVelocity, hitNormal) * hitNormal;
							auto const hitPointVelocityTangent = hitPointVelocity - hitPointVelocityNormal;
							auto const springForce = k_groundEllasticity * penetrationDist * hitNormal;
							auto const dampenerForce = -solidDampenerStrength * hitPointVelocityNormal;
							auto frictionForce = glm::vec3{ 0.0f };
							if (glm::length(hitPointVelocityTangent) != 0.0f)
							{
								// TODO: where is dampenerForce here?
								frictionForce = -glm::normalize(hitPointVelocityTangent)
									* std::min(k_groundFriction * glm::length(springForce), 0.5f * glm::length(hitPointVelocityTangent) * solidMass / ds);
							}

							// TODO: where is dampenerForce here?
							auto const hitForce = springForce + dampenerForce + frictionForce / 2.0f;
							k_lastContactForce = hitForce;

							auto const dV = (k_gravity + hitForce / solidMass) * ds;
							auto const localHitForce = glm::transpose(glm::mat3{ solidTransform }) * hitForce;
							auto const localLeverVector = glm::transpose(glm::mat3{ solidTransform }) * leverVector;
							auto const localDW = invertedSolidInertia * glm::cross(localLeverVector, localHitForce) * ds;
							auto const dW = glm::mat3{ solidTransform } *localDW;

							solidLinearVelocity = solidLinearVelocity + dV;
							solidAngularVelocity = solidAngularVelocity - (solidAngularVelocity * k_groundRollingFriction * ds) + dW;

							k_ellipsoidPosition += solidLinearVelocity * ds;

							auto const theta = solidAngularVelocity * ds;
							auto const thetaMagnitude = glm::length(theta);
							if (thetaMagnitude > glm::epsilon<float>()) // some friction constant?
							{
								glm::vec3 axis = theta / thetaMagnitude;
								float halfTheta = thetaMagnitude / 2.0f;

								k_ellipsoidRotation = glm::quat(std::cos(halfTheta), axis * std::sin(halfTheta)) * k_ellipsoidRotation;
							}
						}
						dt -= ds;
					}

					if (!solidIsGrounded)
					{
						solidLinearVelocity += k_gravity * dt;
						auto linearMove = solidLinearVelocity * dt;
						auto angularMove = solidAngularVelocity * dt;

						auto closestHitTime = 1.0f;
						for (auto& solidShapePart : solidShape.parts)
						{
							auto ellipsoidPos = cleanup(solidTransform * glm::vec4{ solidShapePart.position, 1.0f });
							auto ellipsoidRot = k_ellipsoidRotation * solidShapePart.rotation;

							std::vector<std::pair<glm::mat4, aoegl::rgba>> _testTransforms;
							std::int32_t _testCount;
							auto const [hitTime, ellipsoidPoint, trianglePoint] = _ellipsoid_move(
								ellipsoidPos, ellipsoidRot, solidShapePart.radiuses, linearMove, angularMove, k_groundTriangle, _testTransforms, _testCount);
							if (0 <= hitTime && hitTime < closestHitTime)
							{
								closestHitTime = hitTime;
							}
						}

						k_ellipsoidPosition += closestHitTime * linearMove;
						k_ellipsoidRotation = glm::quat(closestHitTime * angularMove) * k_ellipsoidRotation;
						dt -= closestHitTime * dt;
						if (closestHitTime < 1.0f)
						{
							solidIsGrounded = true;
						}
					}
				}
			}

			k_isEllipsoidGrounded = solidIsGrounded;

			return;
		}

		if (false)
		{
			auto const ellipsoidInertia = k_ellipsoidMass / 5.0f * glm::mat3{
				glm::vec3{k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y + k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z, 0.0f, 0.0f},
				glm::vec3{0.0f, k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z + k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x, 0.0f},
				glm::vec3{0.0f, 0.0f, k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x + k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y} };

			k_ellipsoidEnergy = calc_energy(k_ellipsoidMass, k_ellipsoidLinearVelocity, ellipsoidInertia, k_ellipsoidAngularVelocity, k_gravity, k_ellipsoidPosition);

			auto integrate = [&](auto const maxTime)
				{
					auto z = 0.0f;
					auto pos = k_ellipsoidPosition;
					auto rot = k_ellipsoidRotation;
					auto V = k_ellipsoidLinearVelocity;
					auto W = k_ellipsoidAngularVelocity;

					auto const m = k_ellipsoidMass;
					auto const k = k_groundEllasticity;
					auto const b = -std::log(k_groundRestitution) * std::sqrt(k_ellipsoidMass * k_groundEllasticity) / std::numbers::pi_v<float>;
					auto const r = k_groundFriction;
					// auto const N = glm::normalize(glm::cross(k_groundTriangle.m_p1 - k_groundTriangle.m_p0, k_groundTriangle.m_p2 - k_groundTriangle.m_p0));
					auto const I = ellipsoidInertia;
					auto const iI = glm::inverse(I);

					auto S = 0.0f;
					auto const ds = k_integrationTimeStep;
					auto integrationStepsDone = 0;
					while (z <= 0.001f && S <= maxTime)
					{
						++integrationStepsDone;
#pragma region FOO
						S += ds;
						auto const Tr = aoest::combine(pos, rot);
						glm::mat4 const iTr = glm::inverse(Tr);
						//auto const [newZ, ellipsoidP, triangleP] =
						//	ellipsoid_intersect2(glm::scale(Tr, k_ellipsoidRadiuses), glm::inverse(glm::scale(Tr, k_ellipsoidRadiuses)), k_groundTriangle);
						auto const [unitDist, ellipsoidPoint, trianglePoint] = _ellipsoid_intersect(pos, rot, k_ellipsoidRadiuses, k_groundTriangle);
						if (unitDist > 0.001f)
						{
							V += k_gravity * (maxTime - S);
							pos += V * (maxTime - S);
							auto const theta = W * (maxTime - S);
							auto const thetaMagnitude = glm::length(theta);
							if (thetaMagnitude > glm::epsilon<float>())
							{
								glm::vec3 axis = theta / thetaMagnitude;
								float halfTheta = thetaMagnitude / 2.0f;
								rot = glm::quat(std::cos(halfTheta), axis * std::sin(halfTheta)) * rot;
							}

							z = 1;

							break;
						}

						z = -glm::distance(trianglePoint, ellipsoidPoint);
						auto const ellipsoidToTriangle = trianglePoint - ellipsoidPoint;
						auto const N = glm::dot(ellipsoidToTriangle, ellipsoidToTriangle) < glm::epsilon<float>() ?
							glm::normalize(ellipsoidPoint - pos) : glm::normalize(ellipsoidToTriangle);

						// vob k_lastContactTrianglePosition = trianglePoint;
						// vob k_lastContactEllipsoidPosition = ellipsoidPoint;

						auto const iIw = glm::mat3{ Tr } *iI * glm::inverse(glm::mat3{ Tr });

						auto const R = trianglePoint - pos;
						auto const Vp = V + glm::cross(W, R);
						auto const Vpn = glm::dot(Vp, N) * N;
						auto const Vpt = Vp - Vpn;
						auto const Fpn = -k * z * N;
						auto const Fdamp = -b * Vpn; // not applied to angular momentum, else energy is just transfered

						// For now assuming static friction = 2 * dynamic friction, or something like that
						auto Fpt = glm::vec3{ 0.0f };
						auto const maxFrictionlessSpeed = 2.0f * r / m * glm::length(Fpn);
						if (glm::length(Vpt) != 0.0f)
						{
							Fpt = -glm::normalize(Vpt) * std::min(r * glm::length(Fpn), 0.5f * glm::length(Vpt) * m / ds);
						}

						auto const Fp = (Fpn + Fpt);

						auto const dV = (k_gravity + (Fp + Fdamp) / m) * ds;

						auto const lFp = glm::transpose(glm::mat3{ Tr }) * Fp;
						auto const lR = glm::transpose(glm::mat3{ Tr }) * R;
						auto const lDw = iI * glm::cross(lR, lFp) * ds;
						auto const dW = glm::mat3{ Tr } *lDw;

						V = V + dV;
						W = W - (W * k_groundRollingFriction * ds) + dW;
#pragma endregion

						pos += V * ds;

						auto const theta = W * ds;
						auto const thetaMagnitude = glm::length(theta);
						if (thetaMagnitude > glm::epsilon<float>()) // some friction constant?
						{
							glm::vec3 axis = theta / thetaMagnitude;
							float halfTheta = thetaMagnitude / 2.0f;
							rot = glm::quat(std::cos(halfTheta), axis * std::sin(halfTheta)) * rot;
						}
					}

					if (integrationStepsDone > k_maxIntegrationStepsDone)
					{
						k_maxIntegrationStepsDone = integrationStepsDone;
					}

					k_isEllipsoidGrounded = z <= 0.001f;

					k_ellipsoidLinearVelocity = V;
					k_ellipsoidAngularVelocity = W;
					k_ellipsoidPosition = pos;
					k_ellipsoidRotation = rot;
				};

			if (k_isEllipsoidGrounded)
			{
				integrate(simulationTimeStep);
			}
			else if (!k_isEllipsoidGrounded)
			{
				k_ellipsoidLinearVelocity += k_gravity * simulationTimeStep;
				auto linearMove = k_ellipsoidLinearVelocity * simulationTimeStep;
				auto angularMove = k_ellipsoidAngularVelocity * simulationTimeStep;
				std::vector<std::pair<glm::mat4, aoegl::rgba>> _testTransforms;
				std::int32_t _testCount;
				auto const [hitTime, ellipsoidPoint, trianglePoint] = _ellipsoid_move(k_ellipsoidPosition, k_ellipsoidRotation, k_ellipsoidRadiuses, linearMove, angularMove, k_groundTriangle, _testTransforms, _testCount);
				if (0 <= hitTime && hitTime < 1.0f)
				{
					k_ellipsoidPosition += hitTime * linearMove;
					k_ellipsoidRotation = glm::quat(hitTime * angularMove) * k_ellipsoidRotation;

					integrate(simulationTimeStep * (1.0f - hitTime));
				}
				else
				{
					k_ellipsoidPosition += linearMove;
					k_ellipsoidRotation = glm::quat(angularMove) * k_ellipsoidRotation;
				}
			}
		}

		auto const computationStopTime = std::chrono::high_resolution_clock().now();
		k_computationTime = (computationStopTime - computationStartTime).count() * 0.001f;
	}

	void test_system::update_v5() const
	{

		// 0. statics
		// A. time
		static auto k_useFixedTimeStep = true;
		static auto k_maxDeltaTime = 1.0f / 30.0f;
		static auto k_fixedTimeStep = 1.0f / 100.0f;
		static auto k_integrationTimeStep = 1.0f / 400.0f;
		static auto k_simulationStep = 0;

		static auto k_unusedElapsedTime = 0.0f;
		static auto k_computationTime = 0.0f;
		auto const computationStartTime = std::chrono::high_resolution_clock().now();
		// B. world
		static auto k_gravity = glm::vec3{ 0.0f, -25.0f, 0.0f };
		static auto k_groundTriangles = std::vector<triangle>();
		static auto k_groundTriangle = triangle{
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 0.0f, 100.0f },
			glm::vec3{ 100.0f, 0.0f, 0.0f }
		};
		static auto k_groundTriangle2 = triangle{
			glm::vec3{ 0.0f, 1.0f, 0.0f },
			glm::vec3{ 0.0f, 100.0f, 0.0f },
			glm::vec3{ 0.0f, 1.0f, 100.0f }
		};
		static auto k_groundEllasticity = 100'000.0f;
		static auto k_groundRestitution = 1.0f;
		static auto k_groundFriction = 1.0f;
		static auto k_groundRollingFriction = 1.0f; // how sticky+soft surface prevent rolling
		// C. ellipsoid
		static auto k_ellipsoidRadiuses = glm::vec3{ 3.0f, 3.0f, 1.0f };
		static auto k_ellipsoidMass = 1.0f;
		static auto k_ellipsoidStartPosition = glm::vec3{ 21.2f, 14.0f, 20.0f };
		static auto k_ellipsoidStartRotation = glm::vec3{ 0.0f, 0.0f, 0.0f };
		static auto k_ellipsoidStartLinearVelocity = glm::vec3{ 0.0f, 0.0f, 0.0f };
		static auto k_ellipsoidStartAngularVelocity = glm::vec3{ 0.0f, 0.0f, 0.0f };

		static auto k_ellipsoidPosition = k_ellipsoidStartPosition;
		static auto k_ellipsoidRotation = glm::quat(k_ellipsoidStartRotation);
		static auto k_ellipsoidLinearVelocity = k_ellipsoidStartLinearVelocity;
		static auto k_ellipsoidAngularVelocity = k_ellipsoidStartAngularVelocity;
		static auto k_isEllipsoidGrounded = false;
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
			solid_shape::part{ glm::vec3{ -0.01553f, 0.36325f, -1.75357f } - glm::vec3{ 0.0f, 0.35f, 0.0f }, glm::quat(glm::vec3{glm::half_pi<float>(), glm::half_pi<float>(), 0.0f}), glm::vec3{0.385f, 0.905f, 0.283f}},
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

		if (m_inputs->keyboard.keys[aoein::keyboard::key::P].is_pressed())
		{
			k_ellipsoidLinearVelocity = k_ellipsoidStartLinearVelocity;
			k_ellipsoidAngularVelocity = k_ellipsoidStartAngularVelocity;
			k_ellipsoidPosition = k_ellipsoidStartPosition;
			k_ellipsoidRotation = glm::quat(k_ellipsoidStartRotation);
			k_isEllipsoidGrounded = false;
			k_maxIntegrationStepsDone = 0;
			k_simulationStep = 0;
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
		ImGui::InputFloat("Integration Time Step", &k_integrationTimeStep);
		ImGui::BeginDisabled();
		ImGui::InputFloat("Computation Time", &k_computationTime);
		ImGui::InputInt("Step", &k_simulationStep);
		auto fps = 1000.0f * m_simulationTimeContext->m_elapsedTime.get_value();
		ImGui::InputFloat("Frame Time (ms)", &fps);
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

		auto const frameTime = m_simulationTimeContext->m_elapsedTime.get_value();
		k_unusedElapsedTime += std::clamp(frameTime, 0.0f, k_maxDeltaTime);
		auto const isPaused = frameTime == 0.0f;
		if (isPaused)
		{
			k_unusedElapsedTime = 0;
		}
		auto const simulationTimeStep = k_useFixedTimeStep ? (k_unusedElapsedTime > k_fixedTimeStep ? k_fixedTimeStep : 0.0f) : std::min(k_unusedElapsedTime, k_maxDeltaTime);
		k_unusedElapsedTime = k_useFixedTimeStep ? k_unusedElapsedTime - simulationTimeStep : 0.0f;
		if (simulationTimeStep > 0.0f)
		{
			++k_simulationStep;
		}

		ImGui::SeparatorText("State");
		if (!isPaused)
		{
			ImGui::BeginDisabled();
		}

		ImGui::InputFloat3("Position", &k_ellipsoidPosition.x);
		auto ellipsoidEuler = glm::eulerAngles(k_ellipsoidRotation);
		ImGui::InputFloat3("Rotation", &ellipsoidEuler.x);
		k_ellipsoidRotation = glm::quat(ellipsoidEuler);
		ImGui::InputFloat3("Linear Velocity", &k_ellipsoidLinearVelocity.x);
		ImGui::InputFloat3("Angular Velocity", &k_ellipsoidAngularVelocity.x);

		if (isPaused)
		{
			ImGui::BeginDisabled();
		}
		ImGui::InputFloat("Energy", &k_ellipsoidEnergy);
		ImGui::Checkbox("Is Grounded", &k_isEllipsoidGrounded);
		ImGui::InputFloat("Max Y", &k_yMax);
		ImGui::EndDisabled();
		static bool k_rk4 = false;
		ImGui::Checkbox("RK4", &k_rk4);

		ImGui::End();

		// 2. draw
		draw_triangle(*m_debugMeshWorldComponent, k_groundTriangle.m_p0, k_groundTriangle.m_p1, k_groundTriangle.m_p2, aoegl::to_rgba(glm::vec4{ 0.25f }));
		// draw_triangle(*m_debugMeshWorldComponent, k_groundTriangle2.m_p0, k_groundTriangle2.m_p1, k_groundTriangle2.m_p2, aoegl::to_rgba(glm::vec4{ 0.25f }));
		// vob draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses, aoegl::to_rgba(glm::vec4{ 0.5f }));
		draw_ellipsoid(*m_debugMeshWorldComponent, aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses, aoegl::to_rgba(glm::vec4{ 0.4f, 0.1f, 0.1f, 1.0f }));

		// draw solid
		for (auto const& solidPart : k_solidShape.parts)
		{
			/* draw_ellipsoid(
				*m_debugMeshWorldComponent,
				aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation) * aoest::combine(solidPart.position, solidPart.rotation),
				solidPart.radiuses,
				// vob aoegl::to_rgba(glm::vec4{ 0.4f, 0.1f, 0.1f, 1.0f }));
				aoegl::to_rgba(glm::vec4{ 0.5f })); */
		}
		//draw_line(*m_debugMeshWorldComponent, k_ellipsoidPosition, k_ellipsoidPosition + k_ellipsoidLinearVelocity, aoegl::k_green);
		//draw_line(*m_debugMeshWorldComponent, k_ellipsoidPosition, k_ellipsoidPosition + k_ellipsoidAngularVelocity, aoegl::k_red);

		// draw hits
		//draw_line(*m_debugMeshWorldComponent, k_lastContactEllipsoidPosition, k_lastContactEllipsoidPosition + k_lastContactForce, aoegl::k_orange);
		//draw_sphere(*m_debugMeshWorldComponent, k_lastContactEllipsoidPosition, 0.1f, aoegl::k_red);

		// 3. physics
		auto const ellipsoidTransform = glm::scale(aoest::combine(k_ellipsoidPosition, k_ellipsoidRotation), k_ellipsoidRadiuses);
		auto const ellipsoidTransformInv = glm::inverse(ellipsoidTransform);

		auto const k_ellipsoidInertia = k_ellipsoidMass / 5.0f * glm::diagonal3x3(glm::vec3{
			k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y + k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z,
			k_ellipsoidRadiuses.z * k_ellipsoidRadiuses.z + k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x,
			k_ellipsoidRadiuses.x * k_ellipsoidRadiuses.x + k_ellipsoidRadiuses.y * k_ellipsoidRadiuses.y });
		auto const k_ellipsoidInertiaInv = glm::inverse(k_ellipsoidInertia);

		auto dt = simulationTimeStep;
		auto const dampenerFactor = -std::log(k_groundRestitution) * std::sqrt(k_ellipsoidMass * k_groundEllasticity) / std::numbers::pi_v<float>;
		auto const gravityForce = k_gravity.y * k_ellipsoidMass;
		while (dt > 0.0f)
		{
			auto const ds = std::min(dt, std::max(0.0001f, k_integrationTimeStep));

			dt -= ds;

			struct ellipsoid_state
			{
				glm::vec3 position;
				glm::vec3 velocity;
				glm::quat rotation;
				glm::vec3 angularVelocity;
			};

			ellipsoid_state s{ k_ellipsoidPosition, k_ellipsoidLinearVelocity, k_ellipsoidRotation, k_ellipsoidAngularVelocity };

			struct ellipsoid_derivative
			{
				glm::vec3 dPosition;
				glm::vec3 dVelocity;
				glm::quat dRotation;
				glm::vec3 dAngularVelocity;
			};

			auto quatDerivative = [](glm::quat q, glm::vec3 omega)
				{
					glm::quat omegaQuat(0.0f, omega.x, omega.y, omega.z);
					return 0.5f * q * omegaQuat;
				};

			auto integrateQuat = [](glm::quat q, glm::vec3 omega, float dt) -> glm::quat
				{
					float angle = glm::length(omega) * dt;
					if (angle < glm::epsilon<float>()) return q;

					glm::vec3 axis = glm::normalize(omega);
					glm::quat dq = glm::angleAxis(angle, axis);
					return glm::normalize(dq * q);
				};

			auto evaluate = [&](ellipsoid_state const& s) -> ellipsoid_derivative
				{
					glm::mat3 R = glm::mat3_cast(s.rotation);
					glm::mat3 worldInertia = R * k_ellipsoidInertia * glm::transpose(R);
					glm::mat3 worldInertiaInv = glm::inverse(worldInertia);

					glm::vec3 force = k_gravity * k_ellipsoidMass;
					glm::vec3 torque(0.0f);

					auto [unitDist, ellipsoidPoint, trianglePoint] = _ellipsoid_intersect(s.position, s.rotation, k_ellipsoidRadiuses, k_groundTriangle);
					if (unitDist < 0.0f)
					{
						k_lastContactEllipsoidPosition = ellipsoidPoint;

						auto lever = ellipsoidPoint - s.position;
						auto hitPointVelocity = s.velocity + glm::cross(s.angularVelocity, lever);
						auto ellipsoidToTriangle = trianglePoint - ellipsoidPoint;
						auto penetration = glm::length(ellipsoidToTriangle);
						auto hitNormal = penetration > glm::epsilon<float>() ? glm::normalize(ellipsoidToTriangle) : glm::vec3(0, 1, 0);

						hitNormal.z = 0;

						auto vN = glm::dot(hitPointVelocity, hitNormal) * hitNormal;
						auto vT = hitPointVelocity - vN;

						static float k_maxPenetration = 0.1f;
						auto clampedPenetration = std::min(penetration, k_maxPenetration);

						auto springForce = k_groundEllasticity * clampedPenetration * hitNormal;
						auto const lne = std::log(k_groundRestitution);
						auto k_zetaLow = std::sqrt(lne * lne / (lne * lne + std::numbers::pi_v<float> *std::numbers::pi_v<float>));
						auto k_zetaHigh = k_zetaLow;// 1.0f;
						float speed = glm::length(vN);
						float zeta = glm::mix(k_zetaHigh, k_zetaLow, glm::smoothstep(0.01f, 0.2f, speed));
						float dampingCoeff = 2.0f * std::sqrt(k_groundEllasticity * k_ellipsoidMass) * zeta;
						auto dampenerForce = -dampingCoeff * vN;
						glm::vec3 frictionForce(0.0f);
						if (glm::length(vT) > glm::epsilon<float>())
						{
							auto const frictionDir = -glm::normalize(vT);
							auto const maxFriction = k_groundFriction * glm::length(springForce);
							frictionForce = frictionDir * std::min(maxFriction, k_ellipsoidMass * glm::length(vT) / ds);
						}

						force += springForce + dampenerForce + frictionForce;
						torque += glm::cross(lever, springForce + dampenerForce + frictionForce);
						torque -= k_groundRollingFriction * s.angularVelocity;

						if (penetration > k_maxPenetration && glm::dot(s.velocity, hitNormal) < 0.0f)
						{
							// force -= k_ellipsoidMass * glm::dot(s.velocity, hitNormal) * hitNormal / ds;
						}

						k_lastContactForce = force * 10.0f;
					}

					ellipsoid_derivative d;
					d.dPosition = s.velocity;
					d.dVelocity = force / k_ellipsoidMass;
					d.dRotation = quatDerivative(s.rotation, glm::transpose(R) * s.angularVelocity);
					d.dAngularVelocity = worldInertiaInv * (torque - glm::cross(s.angularVelocity, worldInertia * s.angularVelocity));
					return d;
				};

			auto integrateRK4 = [&](ellipsoid_state& s, float dt)
				{
					auto k1 = evaluate(s);
					auto s2 = s;
					s2.position += k1.dPosition * (dt / 2.0f);
					s2.velocity += k1.dVelocity * (dt / 2.0f);
					s2.rotation = glm::normalize(s2.rotation + k1.dRotation * (dt / 2.0f));
					s2.angularVelocity += k1.dAngularVelocity * (dt / 2.0f);

					auto k2 = evaluate(s2);
					s2 = s;
					s2.position += k2.dPosition * (dt / 2.0f);
					s2.velocity += k2.dVelocity * (dt / 2.0f);
					s2.rotation = glm::normalize(s2.rotation + k2.dRotation * (dt / 2.0f));
					s2.angularVelocity += k2.dAngularVelocity * (dt / 2.0f);

					auto k3 = evaluate(s2);
					s2 = s;
					s2.position += k3.dPosition * dt;
					s2.velocity += k3.dVelocity * dt;
					s2.rotation = glm::normalize(s2.rotation + k3.dRotation * dt);
					s2.angularVelocity += k3.dAngularVelocity * dt;

					auto k4 = evaluate(s2);

					k_ellipsoidPosition += (dt / 6.0f) * (k1.dPosition + 2.0f * k2.dPosition + 2.0f * k3.dPosition + k4.dPosition);
					k_ellipsoidLinearVelocity += (dt / 6.0f) * (k1.dVelocity + 2.0f * k2.dVelocity + 2.0f * k3.dVelocity + k4.dVelocity);
					k_ellipsoidRotation = glm::normalize(k_ellipsoidRotation + (dt / 6.0f) * (k1.dRotation + 2.0f * k2.dRotation + 2.0f * k3.dRotation + k4.dRotation));
					k_ellipsoidAngularVelocity += (dt / 6.0f) * (k1.dAngularVelocity + 2.0f * k2.dAngularVelocity + 2.0f * k3.dAngularVelocity + k4.dAngularVelocity);
				};

			integrateRK4(s, ds);

			//// Runge Kutta 4
			//auto const f = [&](auto const t, auto ly, auto lv)
			//	{
			//		auto const z = ly - k_ellipsoidRadiuses.y;
			//		auto const springForce = z < 0.0f ? -k_groundEllasticity * z : 0.0f;
			//		auto const dampenerForce = z < 0.0f ? -dampenerFactor * lv : 0.0f;
			//		return (springForce + dampenerForce + gravityForce) / k_ellipsoidMass;
			//	};

			//auto const v = k_ellipsoidLinearVelocity.y;
			//auto const y = k_ellipsoidPosition.y;

			//auto const C0 = v;
			//auto const K0 = f(0.0f, y, C0);
			//auto const C1 = v + ds / 2.0f * K0;
			//auto const K1 = f(0.0f + ds / 2.0f, y + ds / 2.0f * C0, C1);
			//auto const C2 = v + ds / 2.0f * K1;
			//auto const K2 = f(0.0f + ds / 2.0f, y + ds / 2.0f * C1, C2);
			//auto const C3 = v + ds * K2;
			//auto const K3 = f(0.0f + ds, y + ds * C2, C3);

			//auto const dy = ds / 6.0f * (C0 + 2.0f * C1 + 2.0f * C2 + C3);
			//auto const dv = ds / 6.0f * (K0 + 2.0f * K1 + 2.0f * K2 + K3);

			//k_ellipsoidLinearVelocity.y += dv;
			//k_ellipsoidPosition.y += dy;
		}

		if (k_y0 <= k_y1 && k_y1 > k_ellipsoidPosition.y)
		{
			k_yMax = k_y1 - k_ellipsoidRadiuses.y;
		}

		k_y0 = k_y1;
		k_y1 = k_ellipsoidPosition.y;
	}

	void test_system::update_test() const
	{
		static auto k_radiuses = glm::vec3{ 1.0f, 2.0f, 1.0f };
		static auto k_position = glm::vec3{ -1.5, 4.5f, 5.0f };
		static auto k_rotation = glm::vec3{ 0.0f, 0.0f, 1.0f };
		static auto k_linearMove = glm::vec3{ 0.0f, -5.0f, 0.0f };
		static auto k_angularMove = glm::vec3{ 0.0f, 0.0f, 4.0f };


		static auto k_p0 = glm::vec3{ 0.0f, 0.0f, 0.0f };
		static auto k_p1 = glm::vec3{ 0.0f, 0.0f, 100.0f };
		static auto k_p2 = glm::vec3{ 100.0f, 0.0f, 0.0f };

		ImGui::Begin("Test System");
		ImGui::InputFloat3("Radiuses", &k_radiuses.x);
		ImGui::InputFloat3("Position", &k_position.x);
		ImGui::InputFloat3("Rotation", &k_rotation.x);
		ImGui::InputFloat3("P0", &k_p0.x);
		ImGui::InputFloat3("P1", &k_p1.x);
		ImGui::InputFloat3("P2", &k_p2.x);
		ImGui::InputFloat3("Linear Move", &k_linearMove.x);
		ImGui::InputFloat3("Angular Move", &k_angularMove.x);
		ImGui::BeginDisabled();
		static auto k_ratio = 0.0f;
		static auto k_testCount = 0;
		ImGui::InputFloat("Ratio", &k_ratio);
		ImGui::InputInt("Count", &k_testCount);
		ImGui::EndDisabled();
		ImGui::End();


		auto const transform = aoest::combine(k_position, glm::quat{ k_rotation });

		draw_triangle(*m_debugMeshWorldComponent, k_p0, k_p1, k_p2, aoegl::k_forest);
		draw_ellipsoid(*m_debugMeshWorldComponent, transform, k_radiuses, aoegl::k_gray);

		std::vector<std::pair<glm::mat4, aoegl::rgba>> testTransforms;
		auto const [ratio, ellipsoidPoint, trianglePoint] = _ellipsoid_move(
			k_position, glm::quat{ k_rotation }, k_radiuses, k_linearMove, k_angularMove, triangle{ k_p0, k_p1, k_p2 }, testTransforms, k_testCount);
		k_ratio = ratio;
		auto const targetPosition = k_position + k_linearMove;
		auto const targetRotation = glm::eulerAngles(glm::quat{ k_angularMove } *glm::quat{ k_rotation });
		auto const targetTransform = aoest::combine(targetPosition, glm::quat{ targetRotation });
		auto const finalPosition = k_position + ratio * k_linearMove;
		auto const finalRotation = glm::eulerAngles(glm::quat{ ratio * k_angularMove } *glm::quat{ k_rotation });
		auto const finalTransform = aoest::combine(finalPosition, glm::quat{ finalRotation });

		if (m_inputs->mouse.buttons[aoein::mouse::button::Left].is_pressed())
		{
			k_position.x -= m_inputs->mouse.axes[aoein::mouse::axis::X].get_change() * 0.01f;
			k_position.y -= m_inputs->mouse.axes[aoein::mouse::axis::Y].get_change() * 0.01f;
		}

		if (ratio < 1.0f)
		{
			draw_ellipsoid(*m_debugMeshWorldComponent, finalTransform, k_radiuses, aoegl::k_cyan);
			draw_sphere(*m_debugMeshWorldComponent, trianglePoint, 0.05f, aoegl::k_red);
			draw_sphere(*m_debugMeshWorldComponent, ellipsoidPoint, 0.05f, aoegl::k_orange);
			draw_line(*m_debugMeshWorldComponent, ellipsoidPoint, ellipsoidPoint + 2.0f * _ellipsoid_normal(finalPosition, finalRotation, k_radiuses, ellipsoidPoint), aoegl::k_yellow);
		}
		else
		{
			draw_ellipsoid(*m_debugMeshWorldComponent, targetTransform, k_radiuses, aoegl::k_green);
		}
		auto prevTr = transform;
		for (auto [testTransform, testColor] : testTransforms)
		{
			draw_ellipsoid(*m_debugMeshWorldComponent, testTransform, k_radiuses, testColor);
			if (testColor == aoegl::k_red)
			{
				// draw_line(*m_debugMeshWorldComponent, aoest::get_position(prevTr), aoest::get_position(testTransform), testColor);
			}
			prevTr = testTransform;
		}

	}

	void test_system::update() const
	{
		update_v5();
	}
}