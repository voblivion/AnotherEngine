//#pragma once
//
//#include <vob/aoe/core/visitor/Aggregate.h>
//
//#include <vob/aoe/common/space/Vector.h>
//#include <vob/aoe/common/gui/elements/AElement.h>
//
//namespace vob::aoe::common::gui
//{
//	struct GuiComponent final
//		: public vis::Aggregate<GuiComponent>
//	{
//		// Attributes
//		vec2 m_size{};
//		type::Cloneable<AElement> m_root;
//
//		// Constructors
//		explicit GuiComponent(type::Cloner const& a_cloner)
//			: m_root{ a_cloner }
//		{}
//
//		// Methods
//		friend class vis::Aggregate<GuiComponent, aoecs::AComponent>;
//		template <typename VisitorType, typename ThisType>
//		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
//		{
//
//		}
//	};
//}
