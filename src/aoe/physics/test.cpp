//#include <vob/aoe/physics/physics_system.h>
//
//#include <vob/aoe/physics/maths.h>
//#include <glm/gtx/matrix_operation.hpp>
//
//#include "imgui.h"
//#include "optick.h"
//
//namespace vob::aoeph
//{
//	physics_system::physics_system(aoeng::world_data_provider& a_wdp)
//		: m_simulationTimeContext{ a_wdp }
//		, m_physicsContext{ a_wdp }
//		, m_inputs{ a_wdp }
//		, m_staticColliderEntities{ a_wdp }
//		, m_carColliderEntities{ a_wdp }
//	{
//	}
//
//	struct broadphase_result
//	{
//		glm::vec3 p0;
//		float x1;
//		float x2;
//		float y2;
//		glm::mat3 rotationInv;
//		material material;
//	};
//
//	inline bool test_intersection(aabb const& a_lhs, aabb const& a_rhs)
//	{
//		if (a_lhs.max.x < a_rhs.min.x || a_lhs.max.y < a_rhs.min.y || a_lhs.max.z < a_rhs.min.z)
//		{
//			return false;
//		}
//
//		if (a_rhs.max.x < a_lhs.min.x || a_rhs.max.y < a_lhs.min.y || a_rhs.max.z < a_lhs.min.z)
//		{
//			return false;
//		}
//
//		return true;
//	}
//
//	inline bool test_aabb_intersection(glm::vec3 const& a_halfExtents, glm::vec3 const& a_p0, glm::vec3 const& a_p1, glm::vec3 const& a_p2)
//	{
//		auto const e0 = a_p1 - a_p0;
//		auto const e1 = a_p2 - a_p1;
//		auto const e2 = a_p0 - a_p2;
//
//		std::array<glm::vec3, 9> axes = {
//			glm::vec3{0.0f, -e0.z, e0.y}, glm::vec3{0.0f, -e1.z, e1.y}, glm::vec3{0.0f, -e2.z, e2.y},
//			glm::vec3{e0.z, 0.0f, -e0.x}, glm::vec3{e1.z, 0.0f, -e1.z}, glm::vec3{e2.z, 0.0f, -e2.x},
//			glm::vec3{-e0.y, e0.x, 0.0f}, glm::vec3{-e1.y, e1.x, 0.0f}, glm::vec3{-e2.y, e2.x, 0.0f}
//		};
//		for (int32_t i = 0; i < 9; ++i)
//		{
//			glm::vec3 axis = axes[i];
//			float len = glm::length(axis);
//			if (len < std::numeric_limits<float>::epsilon())
//			{
//				continue;
//			}
//			axis /= len;
//
//			// project box
//			float r = a_halfExtents.x * std::abs(axis.x) + a_halfExtents.y * std::abs(axis.y) + a_halfExtents.z * std::abs(axis.z);
//
//			float p0 = glm::dot(a_p0, axis);
//			float p1 = glm::dot(a_p1, axis);
//			float p2 = glm::dot(a_p2, axis);
//
//			float minP = std::min({ p0, p1, p2 });
//			float maxP = std::max({ p0, p1, p2 });
//
//			if (minP > r || maxP < -r)
//			{
//				return false;
//			}
//		}
//
//		for (int i = 0; i < 3; ++i)
//		{
//			float minT = std::min({ a_p0[i], a_p1[i], a_p2[i] });
//			float maxT = std::max({ a_p0[i], a_p1[i], a_p2[i] });
//			if (minT > a_halfExtents[i] || maxT < -a_halfExtents[i])
//			{
//				return false;
//			}
//		}
//
//		glm::vec3 normal = glm::cross(e0, e1);
//		float lenN = glm::length(normal);
//		if (lenN < std::numeric_limits<float>::epsilon())
//		{
//			return false;
//		}
//		normal /= lenN;
//
//		float d = glm::dot(normal, a_p0);
//		float r = a_halfExtents.x * std::abs(normal.x) + a_halfExtents.y * std::abs(normal.y) + a_halfExtents.z * std::abs(normal.z);
//
//		if (std::abs(d) > r)
//		{
//			return false;
//		}
//
//		return true;
//	}
//
//	inline aabb compute_car_broadphase_bounds(glm::mat4x3 const& a_carTransform, car_collider const& a_carCollider)
//	{
//		glm::vec3 right(a_carTransform[0]);
//		glm::vec3 up(a_carTransform[1]);
//		glm::vec3 forward(a_carTransform[2]);
//		glm::vec3 position(a_carTransform[3]);
//
//		auto boundsCenterLocal = (a_carCollider.boundsLocal.min + a_carCollider.boundsLocal.max) / 2.0f;
//		auto boundsHalfExtentsLocal = (a_carCollider.boundsLocal.max - a_carCollider.boundsLocal.min) / 2.0f;
//
//		glm::vec3 boundsCenterWorld = position + right * boundsCenterLocal.x + up * boundsCenterLocal.y + forward * boundsCenterLocal.z;
//		glm::mat3 rotation = glm::mat3(glm::abs(right), glm::abs(up), glm::abs(forward));
//
//		auto boundsHalfExtentsWorld = rotation * boundsHalfExtentsLocal;
//
//		return aabb{ boundsCenterWorld - boundsHalfExtentsWorld, boundsCenterWorld + boundsHalfExtentsWorld };
//	}
//
//	struct rk4_car_state
//	{
//		struct rk4_suspension_state
//		{
//			float length;
//			float velocity;
//		};
//
//		glm::vec3 position;
//		glm::quat rotation;
//		glm::vec3 linearVelocity;
//		glm::vec3 angularVelocityLocal;
//
//		std::vector<rk4_suspension_state> suspensionStates;
//	};
//
//	struct contact
//	{
//		material carMaterial;
//		material staticMaterial;
//		float distance = 0.0f;
//		glm::vec3 carPoint = glm::vec3{ 0.0f };
//		glm::vec3 staticPoint = glm::vec3{ 0.0f };
//		car_collider::chassis_part* chassisPart = nullptr;
//		glm::mat4 wheelTransform = glm::mat4{ 1.0f };
//	};
//
//	// TODO: rename, it's intersection of some triangle's obb and a sphere.
//	inline bool rough_intersect(
//		glm::vec3 const& a_position,
//		float const a_radius,
//		glm::vec3 const& a_triangleP0,
//		float const a_triangleX1,
//		float const a_triangleX2,
//		float const a_triangleY2,
//		glm::mat3 const& a_triangleRotationInv)
//	{
//		auto const minX = std::min(0.0f, a_triangleX2);
//		auto const maxX = std::max(a_triangleX1, a_triangleX2);
//		auto const minY = 0.0f;
//		auto const maxY = a_triangleY2;
//
//		auto const pos = a_triangleRotationInv * (a_position - a_triangleP0);
//
//		return (minX <= pos.x + a_radius && pos.x - a_radius <= maxX)
//			&& (minY <= pos.y + a_radius && pos.y - a_radius <= maxY)
//			&& (0 <= pos.z + a_radius && pos.z - a_radius <= 0.0f);
//	}
//
//	contact compute_closest_contact(
//		glm::vec3 const& a_position,
//		glm::quat const& a_rotation,
//		glm::vec3 const& a_radiuses,
//		std::vector<broadphase_result> const& a_broadphaseResults,
//		glm::ivec3& a_testCounts)
//	{
//		auto const rotationInv = glm::inverse(a_rotation);
//
//		auto const maxRadius = std::max({ a_radiuses.x, a_radiuses.y, a_radiuses.z });
//
//		contact closestContact;
//		for (auto const& [p0, x1, x2, y2, triangleRotationInv, staticMaterial] : a_broadphaseResults)
//		{
//			++a_testCounts.x;
//
//			if (!rough_intersect(a_position, maxRadius, p0, x1, x2, y2, triangleRotationInv))
//			{
//				continue;
//			}
//
//			auto const triangleRotation = glm::transpose(triangleRotationInv);
//			auto const staticTriangle = triangle{
//				p0,
//				p0 + triangleRotation * glm::vec3{x1, 0.0f, 0.0f},
//				p0 + triangleRotation * glm::vec3{x2, y2, 0.0f}
//			};
//
//			++a_testCounts.y;
//			auto const intersectionResult = intersectEllipsoidWithTriangle(
//				a_position, a_rotation, rotationInv, a_radiuses, staticTriangle, &a_testCounts.z);
//
//			if (intersectionResult.signedDistance < closestContact.distance)
//			{
//				closestContact = contact{
//					material{},
//					staticMaterial,
//					intersectionResult.signedDistance,
//					intersectionResult.firstPoint,
//					intersectionResult.secondPoint
//				};
//			}
//		}
//
//		return closestContact;
//	}
//
//	std::vector<contact> compute_chassis_contacts(
//		glm::vec3 const& a_carPosition,
//		glm::quat const& a_carRotation,
//		std::vector<car_collider::chassis_part>& a_chassisParts,
//		std::vector<broadphase_result> const& a_broadphaseResults,
//		glm::ivec3& a_testCounts)
//	{
//		OPTICK_EVENT("Chassis Search");
//		std::vector<contact> chassisContacts;
//		for (auto& chassisPart : a_chassisParts)
//		{
//			auto const partPosition = a_carPosition + a_carRotation * chassisPart.position;
//			auto const partRotation = a_carRotation * chassisPart.rotation;
//
//			contact chassisContact = compute_closest_contact(partPosition, partRotation, chassisPart.radiuses, a_broadphaseResults, a_testCounts);
//			if (chassisContact.distance < 0.0f)
//			{
//				chassisContact.carMaterial = chassisPart.material;
//				chassisContact.chassisPart = &chassisPart;
//
//				chassisContacts.push_back(chassisContact);
//			}
//		}
//		return chassisContacts;
//	}
//
//	std::vector<contact> compute_wheel_contacts(
//		glm::vec3 const& a_carPosition,
//		glm::quat const& a_carRotation,
//		std::vector<car_collider::wheel>& a_wheels,
//		std::vector<rk4_car_state::rk4_suspension_state> const& a_suspensionStates,
//		std::vector<broadphase_result> const& a_broadphaseResults,
//		glm::ivec3& a_testCounts)
//	{
//		OPTICK_EVENT("Wheel Search");
//		std::vector<contact> wheelContacts;
//		for (int32_t w = 0; w < a_suspensionStates.size(); ++w)
//		{
//			auto const& wheel = a_wheels[w];
//			auto const& suspensionState = a_suspensionStates[w];
//
//			auto const wheelPosition = a_carPosition
//				+ a_carRotation * (wheel.attachPosition + wheel.rotation * glm::vec3{ 0.0f, -suspensionState.length, 0.0f });
//			auto const wheelRotation = a_carRotation * wheel.rotation;
//
//			contact wheelContact = compute_closest_contact(wheelPosition, wheelRotation, wheel.radiuses, a_broadphaseResults, a_testCounts);
//			wheelContact.wheelTransform = aoest::combine(wheelPosition, wheelRotation);
//			wheelContacts.push_back(wheelContact);
//		}
//		return wheelContacts;
//	}
//
//	glm::vec3 compute_contact_spring_force_over_100(contact const& a_contact, float a_ellasticityOver100)
//	{
//		return (a_contact.staticPoint - a_contact.carPoint) * a_ellasticityOver100;
//	}
//
//	float compute_damping_coefficient(float a_restitution, float a_ellasticityOver100, float a_referenceMass)
//	{
//		auto const logRestitutionSquared = square(std::log(a_restitution));
//		auto const piSquared = square(std::numbers::pi_v<float>);
//		auto const zeta = std::sqrt(logRestitutionSquared / (logRestitutionSquared + piSquared));
//		// zeta = glm::mix(zetaHigh, zetaLow, glm::smoothstep(0.0, 0.2, smoothstep(0.01, 0.2, 100.0 * hitSpeedNormalOver100))));
//		auto const dampingCoefficient = 2.0f * 10.0f * std::sqrt(a_ellasticityOver100 * a_referenceMass) * zeta;
//		return dampingCoefficient;
//	}
//
//	glm::vec3 compute_contact_dampener_force_over_100(
//		contact const& a_contact, glm::vec3 a_barycenter, glm::vec3 const& a_hitVelocityOver100, float a_dampingCoefficient)
//	{
//		auto const hitNormal = glm::normalize(a_contact.staticPoint - a_contact.carPoint);
//		auto const hitVelocityNormalOver100 = glm::dot(a_hitVelocityOver100, hitNormal) * hitNormal;
//		return -a_dampingCoefficient * hitVelocityNormalOver100;
//	}
//
//	glm::vec3 compute_contact_friction_force_over_100(
//		contact const& a_contact,
//		glm::vec3 const& a_hitVelocityOver100,
//		glm::vec3 const& a_restitutionForceNormalOver100,
//		float a_frictionFactor,
//		float a_referenceMass)
//	{
//		auto const hitNormal = glm::normalize(a_contact.staticPoint - a_contact.carPoint);
//		auto const hitVelocityNormalOver100 = glm::dot(a_hitVelocityOver100, hitNormal) * hitNormal;
//
//		glm::vec3 frictionForceOver100 = glm::vec3{ 0.0f };
//		if (glm::dot(hitVelocityNormalOver100, hitVelocityNormalOver100) > square(glm::epsilon<float>()))
//		{
//			auto const frictionDir = -glm::normalize(hitVelocityNormalOver100);
//			auto const maxFrictionOver100 = a_frictionFactor * glm::length(a_restitutionForceNormalOver100);
//			frictionForceOver100 = frictionDir * std::min(maxFrictionOver100, glm::length(a_restitutionForceNormalOver100) * a_referenceMass);
//		}
//		return frictionForceOver100;
//	}
//
//	glm::vec3 compute_contact_force_over_100(
//		contact const& a_contact, contact_parameters const& a_contactParameters, glm::vec3 const& a_barycenterPosition, glm::vec3 const& a_hitVelocityOver100)
//	{
//		// 1. spring
//		auto const springForceOver100 = compute_contact_spring_force_over_100(a_contact, a_contactParameters.ellasticityOver100);
//
//		// 2. dampener
//		auto const dampingCoefficient = compute_damping_coefficient(
//			a_contactParameters.restitution, a_contactParameters.ellasticityOver100, a_contactParameters.mass);
//		auto const dampenerForceOver100 = compute_contact_dampener_force_over_100(
//			a_contact, a_barycenterPosition, a_hitVelocityOver100, dampingCoefficient);
//
//		// 3. friction
//		auto frictionForceOver100 = compute_contact_friction_force_over_100(
//			a_contact, a_hitVelocityOver100, springForceOver100 + dampenerForceOver100, a_contactParameters.friction, a_contactParameters.mass);
//
//		return springForceOver100 + dampenerForceOver100 + frictionForceOver100;
//	}
//
//	struct contact_parameters
//	{
//		float mass;
//		float ellasticityOver100;
//		float restitution;
//		float friction;
//	};
//
//	inline glm::quat differentiate_quaternion(glm::quat const& a_rotation, glm::vec3 const& a_angularVelocity)
//	{
//		auto const angularVelocity = glm::quat{ 0.0f, a_angularVelocity.x, a_angularVelocity.y, a_angularVelocity.z };
//		return 0.5f * a_rotation * angularVelocity;
//	}
//
//	rk4_car_state rk4_derivate(
//		car_collider& a_carCollider,
//		rk4_car_state const& a_state,
//		std::vector<broadphase_result> const& a_broadphaseResults,
//		glm::ivec3& a_testCounts)
//	{
//		OPTICK_EVENT("RK4");
//		auto const rotationMatrix = glm::mat3_cast(a_state.rotation);
//		auto const rotationMatrixInv = glm::transpose(rotationMatrix);
//		auto const inertia = rotationMatrix * a_carCollider.inertia * glm::transpose(rotationMatrix);
//		auto const inertiaInv = glm::inverse(inertia);
//		auto const barycenterPosition = a_state.position + rotationMatrix * a_carCollider.barycenter;
//
//		std::vector<contact> chassisContacts = compute_chassis_contacts(
//			a_state.position, a_state.rotation, a_carCollider.chassisParts, a_broadphaseResults, a_testCounts);
//
//		std::vector<contact> wheelContacts = compute_wheel_contacts(
//			a_state.position, a_state.rotation, a_carCollider.wheels, a_state.suspensionStates, a_broadphaseResults, a_testCounts);
//
//		auto const mass = a_carCollider.mass;
//		glm::vec3 forceOver100 = a_carCollider.force / 100.0f;
//		auto torqueOver100 = a_carCollider.torque / 100.0f;
//		for (auto const& chassisContact : chassisContacts)
//		{
//			auto const contactParameters = contact_parameters{
//				a_carCollider.mass,
//				1.0f / (1.0f / chassisContact.carMaterial.ellasticityOver100 + 1.0f / chassisContact.staticMaterial.ellasticityOver100),
//				std::max(chassisContact.carMaterial.restitution, chassisContact.staticMaterial.restitution),
//				std::sqrt(chassisContact.carMaterial.friction * chassisContact.staticMaterial.friction)
//			};
//
//			auto const lever = chassisContact.carPoint - barycenterPosition;
//			auto const hitVelocityOver100 = a_state.linearVelocity / 100.0f + glm::cross(rotationMatrix * a_state.angularVelocityLocal / 100.0f, lever);
//
//			auto const totalContactForceOver100 = compute_contact_force_over_100(
//				chassisContact, contactParameters, barycenterPosition, hitVelocityOver100);
//			forceOver100 += totalContactForceOver100;
//			torqueOver100 += glm::cross(lever, totalContactForceOver100);
//
//			a_carCollider.debugRk4StepForceOver100s.emplace_back(chassisContact.carPoint, totalContactForceOver100);
//		}
//
//		std::vector<float> wheelForceOver100s;
//		for (int32_t w = 0; w < a_carCollider.wheels.size(); ++w)
//		{
//			auto& wheel = a_carCollider.wheels[w];
//			auto const& wheelContact = wheelContacts[w];
//
//			auto const suspensionPosition = a_state.position + a_state.rotation * wheel.attachPosition;
//			auto const wheelDown = a_state.rotation * wheel.rotation * glm::vec3{ 0.0f, -1.0f, 0.0f };
//
//			float suspensionForceOver100 = 0.0f;
//			suspensionForceOver100 += -(a_state.suspensionStates[w].length - wheel.suspensionRestLength) * wheel.suspensionEllasticityOver100;
//			suspensionForceOver100 += -a_state.suspensionStates[w].velocity * wheel.suspensionDamperOver100;
//
//
//			float wheelForceOver100 = suspensionForceOver100;
//
//			forceOver100 += -suspensionForceOver100 * wheelDown;
//			torqueOver100 += glm::cross(suspensionPosition - barycenterPosition, -suspensionForceOver100 * wheelDown);
//
//			wheel.debugRk4StepForceOver100s.emplace_back(suspensionPosition, -suspensionForceOver100 * wheelDown);
//
//			auto const wheelPosition = a_state.position + a_state.rotation * wheel.attachPosition + wheelDown * a_state.suspensionStates[w].length;
//
//			// wheel.isTireGrounded = false;
//			if (wheelContact.distance < -glm::epsilon<float>())
//			{
//				auto const lever = wheelContact.carPoint - barycenterPosition;
//				auto const hitVelocityOver100 =
//					a_state.linearVelocity / 100.0f
//					+ glm::cross(rotationMatrix * a_state.angularVelocityLocal / 100.0f, lever);
//				// + wheelDown * a_state.suspensionStates[w].velocity / 100.0f;
//				auto const hitNormal = glm::normalize(wheelContact.staticPoint - wheelContact.carPoint);
//
//				auto const wheelRight = a_state.rotation * wheel.rotation * glm::vec3{ 1.0f, 0.0f, 0.0f };
//				auto const contactAngleSin = glm::dot(wheelRight, hitNormal);
//				auto const tireMaxAngleSin = std::cos(std::numbers::pi_v<float> / 2.0f - wheel.tireMaxAngle);
//
//				auto const isRimContact = std::abs(contactAngleSin) > tireMaxAngleSin;
//				auto const contactParameters = isRimContact
//					? contact_parameters{
//							wheel.mass,
//							1.0f / (1.0f / wheelContact.carMaterial.ellasticityOver100 + 1.0f / wheelContact.staticMaterial.ellasticityOver100),
//							std::max(wheelContact.carMaterial.restitution, wheelContact.staticMaterial.restitution),
//							std::sqrt(wheelContact.carMaterial.friction * wheelContact.staticMaterial.friction)
//				}
//					: contact_parameters{
//							wheel.mass,
//							wheel.tireMaterial.ellasticityOver100,
//							wheel.tireMaterial.restitution,
//							wheel.tireMaterial.friction
//				};
//
//				auto const totalContactForceOver100 = compute_contact_force_over_100(
//					wheelContact, contactParameters, barycenterPosition, hitVelocityOver100);
//
//				auto const wheelContactForceOver100 = glm::dot(totalContactForceOver100, wheelDown);
//				wheel.debugRk4StepForceOver100s.emplace_back(wheelContact.carPoint, wheelContactForceOver100 * wheelDown);
//
//				if (isRimContact)
//				{
//					auto const chassisContactForceOver100 = totalContactForceOver100 - wheelContactForceOver100 * wheelDown;
//					forceOver100 += chassisContactForceOver100;
//					torqueOver100 += glm::cross(lever, chassisContactForceOver100);
//
//
//					wheel.debugRk4StepForceOver100s.emplace_back(wheelContact.carPoint, chassisContactForceOver100);
//				}
//				else
//				{
//					wheel.isTireGrounded = true;
//
//					wheel.groundNormal = hitNormal;
//					wheel.groundPosition = wheelContact.staticPoint;
//					wheel.groundMaterial = wheelContact.staticMaterial;
//					wheel.tirePosition = wheelContact.carPoint;
//				}
//
//				wheelForceOver100 += wheelContactForceOver100;
//			}
//
//			if (a_state.suspensionStates[w].length < 0.0f)
//			{
//				auto const compressedSpringForceOver100 = a_state.suspensionStates[w].length * wheel.compressedEllasticityOver100 * wheelDown;
//
//				auto const compressedDampingCoefficient = compute_damping_coefficient(
//					wheel.compressedRestitution, wheel.compressedEllasticityOver100, wheel.mass);
//				auto const compressedDamperForceOver100 = -compressedDampingCoefficient * a_state.suspensionStates[w].velocity * wheelDown / 100.0f;
//
//
//				forceOver100 += compressedSpringForceOver100 + compressedDamperForceOver100;
//				torqueOver100 += glm::cross(suspensionPosition - barycenterPosition, compressedSpringForceOver100 + compressedDamperForceOver100);
//
//				a_carCollider.debugRk4StepForceOver100s.emplace_back(suspensionPosition, compressedSpringForceOver100 + compressedDamperForceOver100);
//
//				wheelForceOver100 -= glm::dot(compressedSpringForceOver100 + compressedDamperForceOver100, wheelDown);
//
//
//				wheel.debugRk4StepForceOver100s.emplace_back(suspensionPosition, -compressedSpringForceOver100 - compressedDamperForceOver100);
//			}
//
//			if (a_state.suspensionStates[w].length > wheel.suspensionMaxLength)
//			{
//				wheelForceOver100 += (wheel.suspensionMaxLength - a_state.suspensionStates[w].length) * wheel.compressedEllasticityOver100;
//				// this would be weird, makes car rotate while in air, not necessary.
//				// forceOver100 -= (wheel.suspensionMaxLength - a_state.suspensionStates[w].length) * wheel.compressedEllasticityOver100 * wheelDown;
//			}
//
//			wheelForceOver100s.push_back(wheelForceOver100);
//		}
//
//		rk4_car_state derivativeStateOver100;
//		derivativeStateOver100.position = a_state.linearVelocity / 100.0f;
//		derivativeStateOver100.linearVelocity = forceOver100 / a_carCollider.mass;
//		// TODO : are we sure it's inv?
//		derivativeStateOver100.rotation = differentiate_quaternion(a_state.rotation, a_state.angularVelocityLocal) / 100.0f;
//		auto const angularVelocityWorld = rotationMatrix * a_state.angularVelocityLocal;
//		derivativeStateOver100.angularVelocityLocal = rotationMatrixInv * (inertiaInv * (torqueOver100 - glm::cross(angularVelocityWorld, inertia * angularVelocityWorld) / 100.0f));
//
//		derivativeStateOver100.suspensionStates.reserve(a_state.suspensionStates.size());
//		for (int32_t w = 0; w < a_carCollider.wheels.size(); ++w)
//		{
//			auto const& suspensionState = a_state.suspensionStates[w];
//			auto& suspensionDerivativeStateOver100 = derivativeStateOver100.suspensionStates.emplace_back();
//
//			suspensionDerivativeStateOver100.length = suspensionState.velocity / 100.0f;
//			suspensionDerivativeStateOver100.velocity = wheelForceOver100s[w] / a_carCollider.wheels[w].mass;
//		}
//
//		return derivativeStateOver100;
//	}
//
//	glm::quat integrate_quaternion(glm::quat const& a_rotation, glm::quat const& a_rotationDerivative, float dt)
//	{
//		glm::quat dqdt = a_rotationDerivative;
//		glm::quat qInv = glm::conjugate(a_rotation);
//		glm::quat wQuat = dqdt * qInv * 2.0f;
//		glm::vec3 angularVelocityWorld = glm::vec3(wQuat.x, wQuat.y, wQuat.z);
//		float angle = glm::length(angularVelocityWorld) * dt;
//		if (angle < std::numeric_limits<float>::epsilon())
//		{
//			return a_rotation;
//		}
//
//		glm::vec3 axis = angularVelocityWorld / glm::length(angularVelocityWorld);
//		glm::quat delta = glm::angleAxis(angle, axis);
//		return glm::normalize(delta * a_rotation);
//	}
//
//	rk4_car_state rk4_step(car_collider const& a_carCollider, rk4_car_state const& a_initialState, rk4_car_state const& a_prevDerivativeStateOver100, float a_stepDuration)
//	{
//		rk4_car_state state = a_initialState;
//		state.position += a_prevDerivativeStateOver100.position * (100.0f * a_stepDuration);
//		state.rotation = integrate_quaternion(state.rotation, a_prevDerivativeStateOver100.rotation, 100.0f * a_stepDuration);
//		//state.rotation = glm::normalize(state.rotation + a_prevDerivativeStateOver100.rotation * (100.0f * a_stepDuration));
//		state.linearVelocity += a_prevDerivativeStateOver100.linearVelocity * (100.0f * a_stepDuration);
//		state.angularVelocityLocal += a_prevDerivativeStateOver100.angularVelocityLocal * (100.0f * a_stepDuration);
//
//		for (int32_t s = 0; s < a_initialState.suspensionStates.size(); ++s)
//		{
//			// Use of chassisMoveDown is cheating a bit; wheel needs to not collide with ground too deep
//			// but here car going down while wheel touching ground would prevent this (because suspension is too light and would take too long to "act").
//			// So I act like suspension is not too strong compare to ground collision (poorly explained).
//			auto const suspensionDown = state.rotation * a_carCollider.wheels[s].rotation * glm::vec3{ 0.0f, -1.0f, 0.0f };
//			auto const chassisMoveDown = glm::dot(a_prevDerivativeStateOver100.position * (100.0f * a_stepDuration), suspensionDown);
//
//			state.suspensionStates[s].length += a_prevDerivativeStateOver100.suspensionStates[s].length * (100.0f * a_stepDuration);
//			state.suspensionStates[s].velocity += a_prevDerivativeStateOver100.suspensionStates[s].velocity * (100.0f * a_stepDuration);
//		}
//
//		return state;
//	}
//
//	void physics_system::update() const
//	{
//		auto& simulationTimeContext = m_simulationTimeContext.get();
//		auto& physicsContext = m_physicsContext.get();
//
//
//		static glm::vec3 k_respawnPosition = glm::vec3{ 2.f, 5.0f, 100.0f };
//		static glm::vec3 k_respawnRotation = glm::vec3{ 0.0f, 3.145f, 0.0f };
//		static glm::vec3 k_respawnVelocity = glm::vec3{ 0.0f, 5.0f, 0.0f };
//		//
//		if (m_inputs->gamepads[0].buttons[aoein::gamepad::button::Y].is_pressed()
//			|| m_inputs->keyboard.keys[aoein::keyboard::key::P].is_pressed())
//		{
//			for (auto [carEntity, carPosition, carRotation, carLinearVelocity, carAngularVelocityLocal, carCollider] : m_carColliderEntities.get().each())
//			{
//				carPosition = k_respawnPosition - carCollider.barycenter;
//				carRotation = glm::quat(k_respawnRotation);
//				carLinearVelocity = k_respawnVelocity;
//				carAngularVelocityLocal = glm::vec3{ 0.0f };
//
//				carCollider.force = glm::vec3{ 0.0f };
//				carCollider.torque = glm::vec3{ 0.0f };
//
//				for (auto& wheel : carCollider.wheels)
//				{
//					wheel.suspensionLength = 0.0f;
//					wheel.suspensionVelocity = 0.0f;
//				}
//			}
//		}
//
//		ImGui::Begin("Respawn");
//		ImGui::InputFloat3("Respawn Position", &k_respawnPosition.x);
//		ImGui::InputFloat3("Respawn Rotation", &k_respawnRotation.x);
//		ImGui::InputFloat3("Respawn Velocity", &k_respawnVelocity.x);
//
//		ImGui::End();
//
//		if (m_inputs->keyboard.keys[aoein::keyboard::key::Z].was_pressed())
//		{
//			simulationTimeContext.play_for_duration = misph::measure_time(1.0) / 100;
//		}
//
//		physicsContext.just_updated = false;
//		if (physicsContext.m_lastUpdateTime + physicsContext.updateDuration < simulationTimeContext.tick_start_time)
//		{
//			// TODO: for now, just skipping if too slow
//			while (physicsContext.m_lastUpdateTime + physicsContext.updateDuration < simulationTimeContext.tick_start_time)
//			{
//				physicsContext.m_lastUpdateTime = physicsContext.m_lastUpdateTime + physicsContext.updateDuration;
//			}
//			physicsContext.just_updated = true;
//			// physicsContext.m_lastUpdateTime = simulationTimeContext.tick_start_time;
//
//			for (auto [carEntity, carPosition, carRotation, carLinearVelocity, carAngularVelocityLocal, carCollider] : m_carColliderEntities.get().each())
//			{
//				auto const carRotationInv = glm::inverse(carRotation);
//				auto const carTransform = aoest::combine4x3(carPosition, carRotation);
//				auto const carBoundsPosition = carPosition + (carCollider.boundsLocal.max + carCollider.boundsLocal.min) / 2.0f;
//				auto const carBoundsHalfExtents = (carCollider.boundsLocal.max - carCollider.boundsLocal.min) / 2.0f;
//
//				// 1. Broadphase
//				// TODO: this array could just be cleared to not reallocate for every car.
//				std::vector<broadphase_result> broadphaseResults;
//				{
//					OPTICK_EVENT("Broadphase")
//						auto const carBounds = compute_car_broadphase_bounds(carTransform, carCollider);
//					for (auto [staticEntity, staticPosition, staticRotation, staticCollider] : m_staticColliderEntities.get().each())
//					{
//						for (auto& staticPart : staticCollider.parts)
//						{
//							staticPart.debugTriangleIndices.clear();
//						}
//
//						if (!test_intersection(carBounds, staticCollider.bounds))
//						{
//							continue;
//						}
//
//						for (auto& staticPart : staticCollider.parts)
//						{
//							int32_t t = 0;
//							for (auto const& staticTriangleLocal : staticPart.triangles)
//							{
//								auto const staticTriangle = triangle{
//									staticPosition + staticRotation * staticTriangleLocal.p0,
//									staticPosition + staticRotation * staticTriangleLocal.p1,
//									staticPosition + staticRotation * staticTriangleLocal.p2
//								};
//
//								if (!test_obb_intersection(carBoundsPosition, carBoundsHalfExtents, carRotationInv, staticTriangle))
//								{
//									++t;
//									continue;
//								}
//
//								//auto const staticTriangleBounds = compute_triangle_bounds(staticTriangle);
//								//if (!test_intersection(searchBounds, staticTriangleBounds))
//								//{
//								//	++t;
//								//	continue;
//								//}
//
//								auto const x1 = glm::length(staticTriangle.p1 - staticTriangle.p0);
//								auto const t0 = (staticTriangle.p1 - staticTriangle.p0) / x1;
//								auto const n = glm::normalize(glm::cross(t0, staticTriangle.p2 - staticTriangle.p0));
//								auto const t1 = glm::cross(n, t0);
//								auto const rotationMatrix = glm::mat3{ t0, t1, n };
//								auto const rotationMatrixInv = glm::transpose(rotationMatrix);
//								glm::vec3 p2 = rotationMatrixInv * (staticTriangle.p2 - staticTriangle.p0);
//
//								broadphaseResults.emplace_back(staticTriangle.p0, x1, p2.x, p2.y, rotationMatrixInv, staticPart.material);
//								//broadphaseResults.emplace_back(staticTriangle, staticPart.material);
//								staticPart.debugTriangleIndices.push_back(t);
//								++t;
//							}
//						}
//					}
//				}
//
//				carCollider.debugRk4StepForceOver100s.clear();
//				carCollider.debugUpdateForces.clear();
//				for (auto& wheel : carCollider.wheels)
//				{
//					wheel.debugRk4StepForceOver100s.clear();
//
//					wheel.isTireGrounded = false;
//				}
//
//				carCollider.broadphaseSize = static_cast<int32_t>(broadphaseResults.size());
//
//				// 2. Narrowphase
//				glm::ivec3 testCounts{ 0 };
//				auto const stepDuration = std::chrono::duration<float>(physicsContext.updateDuration / physicsContext.updateStepCount).count();
//				for (int32_t updateStep = 0; updateStep < physicsContext.updateStepCount; ++updateStep)
//				{
//					OPTICK_EVENT("Narrowphase");
//					auto initialState = rk4_car_state{ carPosition, carRotation, carLinearVelocity, carAngularVelocityLocal };
//					for (auto const& wheel : carCollider.wheels)
//					{
//						initialState.suspensionStates.emplace_back(wheel.suspensionLength, wheel.suspensionVelocity);
//					}
//
//					auto const k1 = rk4_derivate(carCollider, initialState, broadphaseResults, testCounts);
//					auto const k2 = rk4_derivate(carCollider, rk4_step(carCollider, initialState, k1, stepDuration * 0.5f), broadphaseResults, testCounts);
//					auto const k3 = rk4_derivate(carCollider, rk4_step(carCollider, initialState, k2, stepDuration * 0.5f), broadphaseResults, testCounts);
//					auto const k4 = rk4_derivate(carCollider, rk4_step(carCollider, initialState, k3, stepDuration), broadphaseResults, testCounts);
//
//					carPosition += (100.0f * stepDuration / 6.0f) * (k1.position + 2.0f * k2.position + 2.0f * k3.position + k4.position);
//					carRotation = glm::normalize(carRotation + (100.0f * stepDuration / 6.0f) * (k1.rotation + 2.0f * k2.rotation + 2.0f * k3.rotation + k4.rotation));
//					carLinearVelocity += (100.0f * stepDuration / 6.0f) * (k1.linearVelocity + 2.0f * k2.linearVelocity + 2.0f * k3.linearVelocity + k4.linearVelocity);
//					carAngularVelocityLocal += (100.0f * stepDuration / 6.0f) * (k1.angularVelocityLocal + 2.0f * k2.angularVelocityLocal + 2.0f * k3.angularVelocityLocal + k4.angularVelocityLocal);
//
//					for (int32_t w = 0; w < carCollider.wheels.size(); ++w)
//					{
//						auto const& k1Sus = k1.suspensionStates[w];
//						auto const& k2Sus = k2.suspensionStates[w];
//						auto const& k3Sus = k3.suspensionStates[w];
//						auto const& k4Sus = k4.suspensionStates[w];
//						carCollider.wheels[w].suspensionLength += (100.0f * stepDuration / 6.0f) * (k1Sus.length + 2.0f * k2Sus.length + 2.0f * k3Sus.length + k4Sus.length);
//						carCollider.wheels[w].suspensionVelocity += (100.0f * stepDuration / 6.0f) * (k1Sus.velocity + 2.0f * k2Sus.velocity + 2.0f * k3Sus.velocity + k4Sus.velocity);
//					}
//				}
//
//				carCollider.testCounts = testCounts;
//				carCollider.force = glm::vec3{ 0.0f, -25.0f, 0.0f } *carCollider.mass;
//				carCollider.torque = glm::vec3{ 0.0f };
//			}
//		}
//	}
//}
