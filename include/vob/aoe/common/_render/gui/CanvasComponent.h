#pragma once

#include <vob/aoe/common/_render/gui/elements/AElement.h>

#include <vob/misc/visitor/name_value_pair.h>
#include <vob/misc/visitor/macros.h>


namespace vob::aoe::common
{
	struct CanvasComponent
	{
		// Attributes
		type::dynamic_type_clone<AElement> m_rootElement;
		glm::vec2 m_size{ 2048.f, 1024.f };

		// Constructor
		explicit CanvasComponent(type::dynamic_type_clone_copier const& a_cloner)
			: m_rootElement{ a_cloner }
		{}

		template <typename VisitorType, typename ThisType>
		static bool accept(VisitorType& a_visitor, ThisType& a_this)
        {
            a_visitor.visit(misvi::nvp("Root Element", a_this.m_rootElement));
            a_visitor.visit(misvi::nvp("Size", a_this.m_size));
			return true;
		}
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoe::common::CanvasComponent)
	{
		VOB_MISVI_NVP("Root Element", rootElement);
		VOB_MISVI_NVP("Size", size);
		return true;
	}
}
