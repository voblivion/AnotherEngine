#pragma once

#include <vob/aoe/input/bindings.h>

#include <glm/glm.hpp>

#include <array>
#include <numbers>
#include <optional>
#include <utility>
#include <bullet/BulletDynamics/ConstraintSolver/btHinge2Constraint.h>
#include <bullet/BulletCollision/CollisionShapes/btSphereShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCompoundShape.h>
#include <bullet/BulletCollision/CollisionShapes/btConvexHullShape.h>

class btRigidBody;
class btCollisionShape;


namespace vob::aoeph
{
	struct contact_point
	{
		float m_compressionRatio;
		glm::vec3 m_position;
		glm::vec3 m_normal;
		float m_friction;
	};

	struct wheel
	{
		glm::vec3 m_attachmentRelativePosition = {};
		float m_rotationFactor = 0.0f;
		// float m_engineFactor = 0.0f;
		// float m_breakFactor = 0.0f;
		float m_suspensionLength = 0.2f;
		float m_springStrength = 12000.0f;
		float m_damperStrength = 1650.0f;
		float m_radius = 0.364f;

		std::optional<contact_point> m_contactPoint;
		float m_compressionSpeed = 0.0f;

		void update_contact_point(
			float a_compressionRatio, glm::vec3 a_position, glm::vec3 a_normal, float a_friction, float a_dt)
		{
			if (!m_contactPoint.has_value())
			{
				m_compressionSpeed = a_compressionRatio / a_dt;
			}
			else
			{
				m_compressionSpeed = (a_compressionRatio - m_contactPoint->m_compressionRatio) / a_dt;
			}

			m_contactPoint = contact_point{ a_compressionRatio, a_position, a_normal, a_friction };
		}

		void reset_contact_point()
		{
			m_compressionSpeed = 0.0f;
			m_contactPoint.reset();
		}

		float calculate_suspension_strength()
		{
			if (!m_contactPoint.has_value())
			{
				return 0.0f;
			}

			return m_contactPoint->m_compressionRatio * m_springStrength + m_compressionSpeed * m_damperStrength;
		}

		// just for display for now
		float m_width = 0.25f;
	};

	struct car_controller
	{
		explicit car_controller(float a_track = 1.72f, float a_wheelbase = 2.4f)
		{
			m_wheels[0] = wheel{ glm::vec3{a_track / 2, 0.125, a_wheelbase / 2} };
			m_wheels[1] = wheel{ glm::vec3{a_track / 2, 0.125f, -a_wheelbase / 2}, 1.0f };
			m_wheels[2] = wheel{ glm::vec3{-a_track / 2, 0.125f, a_wheelbase / 2} };
			m_wheels[3] = wheel{ glm::vec3{-a_track / 2, 0.125f, -a_wheelbase / 2}, 1.0f };
		}

		std::array<wheel, 4> m_wheels;
		
		std::vector<std::unique_ptr<btCollisionShape>> m_collisionShapes;
		std::unique_ptr<btRigidBody> m_chassisRigidBody;
		
		std::vector<std::pair<std::unique_ptr<btRigidBody>, glm::vec3>> m_wheelRigidBodies;
		std::vector<std::unique_ptr<btHinge2Constraint>> m_wheelHinges;

		std::shared_ptr<btCompoundShape> m_chassisCompound;
		std::vector<std::unique_ptr<btConvexHullShape>> m_chassisShapes;
		std::vector<std::unique_ptr<btSphereShape>> m_wheelShapes;
	};

	// this for debugg
	struct car_controller_world_component
	{
		// aoein::bindings::axis_id m_engine = 0;
		aoein::bindings::axis_id m_turn = 0;
		aoein::bindings::switch_id m_forward = 0;
		aoein::bindings::switch_id m_reverse = 0;
		aoein::bindings::switch_id m_respawn = 0;
	};
}

/*
*							| Type		| Values		|
*	CHASSIS
* Input Steer				| float		| -1 .. 1		|
* Input Is Braking			| bool		| 0, 1			|
* Input Gas Pedal			| bool		| 0, 1			|
* Input Brake Pedal			| bool		| 0, 1			|
* Is Ground Contact			| bool		| 0, 1			| any wheel
* Is Wheels Burning			| bool		| 0, 1			| complex condition, triggered by a brake, but not just if any wheel sliding
* Ground Dist				| float		| 0 .. inf		| raycast from vehicle's center, 0.0025 at rest (so with wheel radius and damper len...)
* Cur Gear					| int		| 1 .. 5		| mostly speed based
* World Vel					| vec3		| - .. +277.5	| mps, always slight drift
* Front Speed				| float		| 0 .. 277.5	| car speed in mps
* Air Brake Normed			| float		| 0 .. 1		| only in air, if braking, and somehow dependent on speed
* Spoiler Open Normed		| float		| 0 .. 1		| opens when car speed > 50kph
* Wings Open Normed			| float		| 0 .. 0.08		| opens when releasing accel while airborn
* Is Top Contact			| bool		| 0, 1			| doesn't have to turtle
* 
*	WHEELS
* Ground Contact Material	| enum		| <var>			|
* Steer Angle				| float		| - .. +0.698	| from 40deg (0kph) to 25deg (200kph)
* Wheel Rot					| float		| 0 .. 512pi	| 256 turns
* Wheel Rot Speed			| float		| - .. +762.36	| max 200 while airborn -> 0.364 radius? (max front speed / max wheel rot speed) width = .3, wheel to wheel 1.72
* Damper Len				| float		| 0.01 .. 0.2	| 0.2 in the air or turtle or touching upside down
* Slip Coef					| float		| 0, 1			| when tire sliding / drifting
* 
*	TURBO
* Is Turbo					| bool		| 0, 1			| true until turbo time elapsed
* Turbo Time				| float		| 0, 0.8		| unsure about max
* 
*	FRAGILE
* Tire Wear 01				| float		| 0 .. 1		| increases when driving over snow/sand/grass while fragile
* Break Normed Coef			| float		| 0 .. 1		| increases (uniform all wheels?) when colliding while fragile
* 
*	ICE
* Icing 01					| float		| 0 .. 1		| increases on ice above some speed threshold, decreases otherwise
* 
*	WATER
* Wetness Value 01			| float		| 0 .. 1		|
* Water Immersion Coef
* Water Over Dist Normed
* Water Over Surface Pos
* 
*	REACTOR
* Reactor Air Control		| vec3		| 0, 1			| boolean type, pbly multiplied by input
* Is Reactor Ground Mode	| bool		| 0, 1			| is ground contact && speed > 108
* Reactor Inputs X
* 
*	SLOWMO
* Simulation Time Coef		| float		| 0.57^n		|
* 
* Discontinuity Count ?
* 
*/
/*

	Vertical Drag / Mass : 0.35 -> 0.03 when accelerating
	Gravity : 25

*/