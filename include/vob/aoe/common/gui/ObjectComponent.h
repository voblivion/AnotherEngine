#pragma once

#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/type/Clone.h>
#include <vob/aoe/core/visitor/Aggregate.h>

#include <vob/aoe/common/gui/AObject.h>


namespace vob::aoe::common::gui
{
	struct ObjectComponent final
		: public vis::Aggregate<ObjectComponent, ecs::AComponent>
	{
		// Attributes
		type::Clone<AObject> m_object;

		// Constructor
		explicit ObjectComponent(type::CloneCopier const& a_cloneCopier)
			: m_object{ a_cloneCopier }
		{}

		// Methods
		friend class vis::Aggregate<ObjectComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(vis::nvp("Object", a_this.m_object));
		}
	};
}
