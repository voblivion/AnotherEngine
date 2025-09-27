#pragma once

#include <vob/aoe/physics/maths.h>
#include <vob/aoe/physics/shapes.h>

#include <vob/aoe/data/database.h>

#include <vob/misc/visitor/macros.h>

#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/LinearMath/btDefaultMotionState.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>

#include <memory>
#include <numbers>


namespace vob::aoeph
{
	struct material
	{
		float ellasticity = 10'000'000.0f;
		float restitution = 0.5f;
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
			glm::vec3 position = glm::vec3{ 0.0f };
			glm::quat rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
			glm::vec3 radiuses = glm::vec3{ 1.0f };
			material material;

			std::vector<debug_contact> debugContacts;
		};

		std::vector<chassis_part> chassisParts;

		struct wheel
		{
			// -- Config
			glm::vec3 attachPosition;
			glm::quat rotation;
			glm::vec3 radiuses = glm::vec3{ 1.0f };

			material rimMaterial;

			float suspensionMaxLength = 0.2f;
			float suspensionEllasticity = 32'894.0f;
			float suspensionDamper = 5'735.0f;

			float tireMaxAngle = std::numbers::pi_v<float> / 4.0f;
			float turnFactor = 0.0f;

			// -- State
			float suspensionLength = 0.0f;
			float suspensionVelocity = 0.0f;
			float isTireGrounded = false;

			bool isGrounded = false;
			glm::vec3 groundNormal = glm::vec3{ 0.0f };
			glm::vec3 groundPosition = glm::vec3{ 0.0f };

			// TMP debug:
			std::vector<debug_contact> debugRimContacts;
			std::vector<debug_contact> debugTireContacts;
		};

		std::vector<wheel> wheels;

		glm::vec3 force{ 0.0f };
		glm::vec3 torque{ 0.0f };

		float mass = 1.0f;
		glm::mat3 inertia = glm::mat3{ 1.0f };
		glm::vec3 barycenter = glm::vec3{ 0.0f };
	};

	/*
		chassis
			position, rotation, radiuses
			material
		wheel
			position, rotation, radiuses
			suspension
			rimMaterial
			
			tireMaterial
			maxTireAngle
			turnAngle
	*/




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

	struct dynamic_body
	{
		struct part
		{
			glm::vec3 position = glm::vec3{ 0.0f };
			glm::quat rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
			glm::vec3 radiuses = glm::vec3{ 1.0f };
			material material;

			// TODO: debug only
			bool debug_draw_enabled = true;
			struct contact
			{
				glm::vec3 ellipsoid_point;
				glm::vec3 static_point;
			};
			mutable std::optional<contact> debug_contact;
		};

		std::vector<part> parts;

		float mass = 1.0f;
		glm::mat3 inertia = glm::mat3{ 1.0f };
		glm::vec3 barycenter = glm::vec3{ 0.0f };

		glm::vec3 force = glm::vec3{ 0.0f };
		glm::vec3 torque = glm::vec3{ 0.0f };
	};
}
