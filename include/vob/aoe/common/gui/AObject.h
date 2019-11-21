#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include "vob/sta/ignorable_assert.h"

#include "vob/aoe/common/space/Vector.h"
#include "vob/aoe/core/type/ADynamicType.h"
#include <vob/aoe/core/ecs/WorldDataProvider.h>
#include "vob/aoe/common/space/TransformComponent.h"
#include "vob/aoe/common/map/HierarchyComponent.h"
#include "Transform.h"

namespace vob::aoe::common::gui
{
	struct BoundingBox
	{
		Vector2 m_position;
		Vector2 m_size;
	};

	struct ObjectComponent;

	using ObjectEntityList = ecs::EntityList<
		ObjectComponent const
		, HierarchyComponent const
	>;
	using ObjectEntity = ObjectEntityList::EntityType;

	struct AObject
		: public vis::Aggregate<AObject, type::ADynamicType>
	{
		// Constructors
		explicit AObject(type::CloneCopier const& a_cloneCopier)
			: m_transform{ a_cloneCopier }
		{}

		// Methods
		friend class vis::Aggregate<AObject, type::ADynamicType>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(vis::nvp("Transform", a_this.m_transform));
		}

		virtual BoundingBox boundingBox(
			ObjectEntity const& a_objectEntity
			, ObjectEntityList const& a_objectEntityList
			, Constraint const& a_constraint
			, Vector2 const& viewSize
		) const = 0;

		virtual void render(
			ObjectEntity const& a_objectEntity
			, ObjectEntityList const& a_objectEntityList
			, Constraint const& a_constraint
			, Vector2 const& viewSize
		) const = 0;

		// Attributes
		Transform m_transform;
	};

}