//#pragma once
//
//#include <vob/aoe/common/gui/elements/ALayout.h>
//
//
//namespace vob::aoe::common::gui
//{
//	template <bool t_vertical>
//	class LinearLayout final
//		: public ALayout
//	{
//	private:
//		// Attributes
//		float m_spacing = 0.0f;
//		bool m_evenSplit = true;
//
//		// Methods
//		void renderInside(Canvas& a_canvas, Transform const& a_transform) const override;
//		float getDesiredInsideWidth() const final;
//		float getDesiredInsideHeight() const final;
//	};
//
//	using VerticalLayout = LinearLayout<true>;
//	using HorizontalLayout = LinearLayout<false>;
//}