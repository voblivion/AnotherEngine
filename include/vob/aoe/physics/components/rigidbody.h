#pragma once

#include <vob/aoe/physics/maths.h>
#include <vob/aoe/physics/shapes.h>

#include <vob/aoe/data/database.h>

#include <vob/misc/visitor/macros.h>

#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/LinearMath/btDefaultMotionState.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>

#include <array>
#include <memory>
#include <numbers>


namespace vob::aoeph
{
	struct DebugForceSet
	{
		void clear()
		{
			forces.clear();
		}

		void add(glm::vec3 const& a_position, glm::vec3 const& a_forceOver100)
		{
			for (auto& [position, forceOver100] : forces)
			{
				if (aoeph::lengthSquared(position - a_position) < mergeDistanceSquared)
				{
					forceOver100 += a_forceOver100;
					return;
				}
			}

			forces.emplace_back(a_position, a_forceOver100);
		}

		decltype(auto) begin() const
		{
			return forces.begin();
		}

		decltype(auto) end() const
		{
			return forces.end();
		}

		float mergeDistanceSquared = 0.01f;
		std::vector<std::pair<glm::vec3, glm::vec3>> forces;
	};

	struct material
	{
		float ellasticityOver100 = 10'000'0.0f;
		float restitution = 0.1f;
		float friction = 0.5f;

		float logRestitution = std::log(restitution);
		float zetaLow = std::sqrt(logRestitution * logRestitution / (logRestitution * logRestitution + std::numbers::pi_v<float> *std::numbers::pi_v<float>));
		float zetaHigh = zetaLow; // or 1.0f
	};

	struct static_collider
	{
		struct part
		{
			material material;
			std::vector<triangle> triangles;

			// TMP debug:
			std::vector<int32_t> debugTriangleIndices;
		};

		std::vector<part> parts;

		aabb bounds;
	};

	struct debug_contact
	{
		std::chrono::high_resolution_clock::time_point time;
		glm::vec3 groundPosition;
		glm::vec3 groundNormal;
	};

	struct car_collider
	{
		struct chassis_part
		{
			// Config
			glm::vec3 position = glm::vec3{ 0.0f };
			glm::quat rotation = glm::quat();
			glm::vec3 radiuses = glm::vec3{ 1.0f };
			material material;
		};

		struct wheel
		{
			// Config
			glm::vec3 attachPosition = glm::vec3{ 0.0f };
			glm::quat rotation = glm::quat();
			glm::vec3 radiuses = glm::vec3{ 1.0f };

			material rimMaterial = material{
				10'000'000.0f,
				0.3f,
				0.9f
			};

			material tireMaterial = material{
				50'000'000.0f,
				0.05f,
				0.0f
			};

			float suspensionRestLength = 0.3f;
			float suspensionMaxLength = 0.2f;
			float suspensionEllasticityOver100 = 50.0f;
			float suspensionDamperOver100 = 50.0f;

			float compressedEllasticityOver100 = 20'000'0.f;
			float compressedRestitution = 0.1f;

			float mass = 10.0f;
			float tireMaxAngle = std::numbers::pi_v<float> / 4.0f;
			float turnFactor = 0.0f; 
			float suspensionLength = 0.0f;
			float suspensionVelocity = 0.0f;
			float isTireGrounded = false;

			glm::vec3 groundNormal = glm::vec3{ 0.0f };
			glm::vec3 groundPosition = glm::vec3{ 0.0f };
			glm::vec3 tirePosition = glm::vec3{ 0.0f };
			material groundMaterial;
			float turn = 0.0f;

			// Debug:
			std::vector<std::pair<glm::vec3, glm::vec3>> debugRk4StepForceOver100s;
		};

		std::vector<chassis_part> chassisParts;

		std::vector<wheel> wheels;

		glm::vec3 force{ 0.0f };
		glm::vec3 torque{ 0.0f };
		glm::vec3 torqueLocal{ 0.0f };

		int32_t broadphaseSize = 0;
		glm::ivec3 testCounts = glm::ivec3{ 0 };

		std::vector<std::pair<glm::vec3, glm::vec3>> debugUpdateForces;
		std::vector<std::pair<glm::vec3, glm::vec3>> debugRk4StepForceOver100s;

		float mass = 1000.0f;
		glm::mat3 inertia = glm::mat3{ 1.0f };
		glm::vec3 barycenter = glm::vec3{ 0.0f };
		aabb boundsLocal;
	};

	struct linear_velocity : public glm::vec3
	{
		template <typename... TArgs>
		linear_velocity(TArgs&&... a_args)
			: glm::vec3{ std::forward<TArgs>(a_args)... }
		{}
	};

	struct angular_velocity_local : public glm::vec3
	{
		template <typename... TArgs>
		angular_velocity_local(TArgs&&... a_args)
			: glm::vec3{ std::forward<TArgs>(a_args)... }
		{}
	};
}
