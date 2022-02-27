#pragma once

#include <vob/aoe/common/physic/Utils.h>

#include <vob/misc/visitor/name_value_pair.h>

#include <bullet/BulletCollision/CollisionShapes/btBoxShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCylinderShape.h>
#include <bullet/BulletCollision/CollisionShapes/btSphereShape.h>


namespace vob::misvi
{
	template <typename VisitorType>
	bool accept(VisitorType& a_visitor, btBoxShape& a_boxShape)
	{
		auto t_halfExtent = glm::vec3{ 0.5f, 0.5f, 0.5f };
		a_visitor.visit(misvi::nvp("HalfExtent", t_halfExtent));

		a_boxShape = btBoxShape(aoe::common::toBtVector(t_halfExtent));
		return true;
	}

	template <typename VisitorType>
	bool accept(VisitorType& a_visitor, btBoxShape const& a_boxShape)
	{
		auto const t_halfExtent = aoe::common::toGlmVec3(
			a_boxShape.getHalfExtentsWithoutMargin());
		a_visitor.visit(misvi::nvp("HalfExtent", t_halfExtent));
		return true;
	}

	template <typename VisitorType>
	bool accept(VisitorType& a_visitor, btCapsuleShape& a_capsuleShape)
	{
		auto t_radius = btScalar{ 0.5f };
		a_visitor.visit(misvi::nvp("Radius", t_radius));
		auto t_height = btScalar{ 1.0f };
		a_visitor.visit(misvi::nvp("Height", t_height));

		a_capsuleShape = btCapsuleShape(t_radius, t_height);
		return true;
	}

	template <typename VisitorType>
	bool accept(VisitorType& a_visitor, btCapsuleShape const& a_capsuleShape)
	{
		auto const t_radius = a_capsuleShape.getRadius();
		a_visitor.visit(misvi::nvp("Radius", t_radius));
		auto const t_height = a_capsuleShape.getHalfHeight() * btScalar{ 2.0f };
		a_visitor.visit(misvi::nvp("Height", t_height));
		return true;
	}

	template <typename VisitorType>
	bool accept(VisitorType& a_visitor, btCylinderShape& a_cylinderShape)
	{
		auto t_halfExtent = glm::vec3{ 0.5f, 0.5f, 0.5f };
		a_visitor.visit(misvi::nvp("HalfExtent", t_halfExtent));

		a_cylinderShape = btCylinderShape(aoe::common::toBtVector(t_halfExtent));
		return true;
	}

	template <typename VisitorType>
	bool accept(VisitorType& a_visitor, btCylinderShape const& a_cylinderShape)
	{
		auto const t_halfExtent = aoe::common::toGlmVec3(
			a_cylinderShape.getHalfExtentsWithoutMargin());
		a_visitor.visit(misvi::nvp("HalfExtent", t_halfExtent));
		return true;
	}

	template <typename VisitorType>
	bool accept(VisitorType& a_visitor, btSphereShape& a_sphereShape)
	{
		auto t_radius = btScalar{ 1.0f };
		a_visitor.visit(misvi::nvp("Radius", t_radius));

		a_sphereShape = btSphereShape(t_radius);
		return true;
	}

	template <typename VisitorType>
	bool accept(VisitorType& a_visitor, btSphereShape const& a_sphereShape)
	{
		auto const t_radius = a_sphereShape.getRadius();
		a_visitor.visit(misvi::nvp("Radius", t_radius));
		return true;
	}
}
