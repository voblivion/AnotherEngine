#pragma once

#include <vob/aoe/common/render/gui/elements/AElement.h>

namespace vob::aoe::common
{
	struct CanvasComponent
		: public vis::Aggregate<CanvasComponent, ecs::AComponent>
	{
		// Attributes
		type::Clone<AElement, type::ADynamicType> m_rootElement;
		vec2 m_size{ 2048.f, 1024.f };

		// Constructor
		explicit CanvasComponent(type::CloneCopier<type::ADynamicType> const& a_cloneCopier)
			: m_rootElement{ a_cloneCopier }
		{}

		// Methods
		friend class vis::Aggregate<CanvasComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(vis::makeNameValuePair("Root Element", a_this.m_rootElement));
			a_visitor.visit(vis::makeNameValuePair("Size", a_this.m_size));
		}
	};
}