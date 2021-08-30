//#pragma once
//
//#include <optional>
//#include <vector>
//
//#include <vob/aoe/core/type/ADynamicType.h>
//#include <vob/aoe/core/type/Clone.h>
//#include <vob/aoe/common/gui/Canvas.h>
//#include <vob/aoe/common/gui/Transform.h>
//
//namespace vob::aoe::common::gui
//{
//	struct SizeConstraint
//	{
//		std::optional<float> m_width;
//		std::optional<float> m_height;
//	};
//
//	class AElement
//		: public type::ADynamicType
//	{
//	public:
//		// Methods
//		void render(Canvas& a_canvas, Transform const& a_transform) const
//		{
//			// margin ? padding ? border ? background ?
//			renderInside(a_canvas, a_transform);
//		}
//		float getDesiredWidth() const
//		{
//			return getDesiredInsideWidth();
//		}
//		float getDesiredHeight() const
//		{
//			return getDesiredInsideHeight();
//		}
//
//	private:
//		// Methods
//		virtual void renderInside(Canvas& a_canvas, Transform const& a_transform) const = 0;
//		virtual float getDesiredInsideWidth() const = 0;
//		virtual float getDesiredInsideHeight() const = 0;
//	};
//}
