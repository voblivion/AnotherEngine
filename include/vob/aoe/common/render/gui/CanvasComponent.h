#pragma once

#include <vob/aoe/common/render/gui/elements/AElement.h>

namespace vob::aoe::common
{
	struct CanvasComponent
		: public ecs::AComponent
	{
		// Attributes
		type::Cloneable<AElement> m_rootElement;
		vec2 m_size{ 2048.f, 1024.f };

		// Constructor
		explicit CanvasComponent(type::Cloner<> const& a_cloner)
			: m_rootElement{ a_cloner }
		{}

		template <typename VisitorType, typename ThisType>
		static void accept(VisitorType& a_visitor, ThisType& a_this)
        {
            a_visitor.visit(vis::makeNameValuePair("Root Element", a_this.m_rootElement));
            a_visitor.visit(vis::makeNameValuePair("Size", a_this.m_size));
		}
	};
}

