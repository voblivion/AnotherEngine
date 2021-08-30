//#include <vob/aoe/common/_gui/elements/LinearLayout.h>
//
//#include <numeric>
//#include <algorithm>
//
//
//namespace vob::aoe::common::gui
//{
//	inline float getElementDesiredWidth(AElement const& a_element)
//	{
//		return a_element.getDesiredWidth();
//	}
//
//	inline float getElementDesiredHeight(AElement const& a_element)
//	{
//		return a_element.getDesiredHeight();
//	}
//
//	template <float getElementDesiredSize(AElement const&)>
//	float getChildDesiredSize(type::Clone<AElement> const& a_child)
//	{
//		assert(a_child.get() != nullptr);
//		return getElementDesiredSize(*a_child);
//	}
//
//	template <float getElementDesiredSize(AElement const&)>
//	float addChildDesiredSize(float const a_total, type::Clone<AElement> const& a_child)
//	{
//		return a_total + getChildDesiredSize<getElementDesiredSize>(a_child);
//	}
//
//	template <float getElementDesiredSize(AElement const&)>
//	float getMaxChildDesiredSize(std::vector<type::Clone<AElement>> const& a_children)
//	{
//		auto maxChildDesiredSize = 0.0f;
//		for (auto& child : a_children)
//		{
//			maxChildDesiredSize = std::max(
//				maxChildDesiredSize
//				, getChildDesiredSize<getElementDesiredSize>(child)
//			);
//		}
//		return maxChildDesiredSize;
//	}
//
//	template <float getElementDesiredPrimarySize(AElement const&)>
//	float getDesiredInsidePrimarySize(
//		std::vector<type::Clone<AElement>> const& a_children
//		, bool const a_evenSplit
//		, float const a_spacing
//	)
//	{
//		if (a_children.empty())
//		{
//			return 0.0f;
//		}
//
//		if (a_evenSplit)
//		{
//			auto const max = getMaxChildDesiredSize<getElementDesiredPrimarySize>(a_children);
//			return (max + a_spacing) * a_children.size() - a_spacing;
//		}
//
//		return std::accumulate(
//			a_children.begin()
//			, a_children.end()
//			, a_spacing * (a_children.size() - 1)
//			, addChildDesiredSize<getElementDesiredPrimarySize>
//		);
//	}
//
//	float& getXPart(vec2& a_vector)
//	{
//		return a_vector.x;
//	}
//
//	float& getYPart(vec2& a_vector)
//	{
//		return a_vector.y;
//	}
//
//	template <float& getPrimaryPart(vec2&), float getElementDesiredPrimarySize(AElement const&)>
//	void renderInsideImpl(
//		Canvas& a_canvas
//		, Transform const& a_transform
//		, std::vector<type::Clone<AElement>> const& a_children
//		, bool const a_evenSplit
//		, float const a_spacing
//	)
//	{
//		if (a_children.empty())
//		{
//			return;
//		}
//
//		auto childTransform = a_transform;
//		auto const childrenSize = getPrimaryPart(childTransform.m_size)
//			- (a_children.size() - 1) * a_spacing;
//		if (childrenSize < 0)
//		{
//			return;
//		}
//
//		if (a_evenSplit)
//		{
//			getPrimaryPart(childTransform.m_size) = childrenSize / a_children.size();
//			for (auto const& a_child : a_children)
//			{
//				a_child->render(a_canvas, childTransform);
//				getPrimaryPart(childTransform.m_position) += getPrimaryPart(childTransform.m_size)
//					+ a_spacing;
//			}
//		}
//		else
//		{
//			auto const desiredChildrenSize = std::accumulate(
//				a_children.begin()
//				, a_children.end()
//				, a_spacing * (a_children.size() - 1)
//				, addChildDesiredSize<getElementDesiredPrimarySize>
//			);
//			auto const sizeRatio = childrenSize / desiredChildrenSize;
//
//			for (auto const& a_child : a_children)
//			{
//				childTransform.m_size.x = sizeRatio
//					* getChildDesiredSize<getElementDesiredPrimarySize>(a_child);
//				a_child->render(a_canvas, childTransform);
//				getPrimaryPart(childTransform.m_position) += getPrimaryPart(childTransform.m_size)
//					+ a_spacing;
//			}
//		}
//	}
//
//	template <bool t_vertical>
//	void LinearLayout<t_vertical>::renderInside(
//		Canvas& a_canvas
//		, Transform const& a_transform
//	) const
//	{
//		if constexpr(t_vertical)
//		{
//			renderInsideImpl<getYPart, getElementDesiredHeight>(
//				a_canvas
//				, a_transform
//				, m_children
//				, m_evenSplit
//				, m_spacing
//			);
//		}
//		else
//		{
//			renderInsideImpl<getXPart, getElementDesiredWidth>(
//				a_canvas
//				, a_transform
//				, m_children
//				, m_evenSplit
//				, m_spacing
//			);
//		}
//	}
//
//	template <bool t_vertical>
//	float LinearLayout<t_vertical>::getDesiredInsideWidth() const
//	{
//		if constexpr (!t_vertical)
//		{ 
//			return getDesiredInsidePrimarySize<getElementDesiredWidth>(
//				m_children
//				, m_evenSplit
//				, m_spacing
//			);
//		}
//		else
//		{
//			return getMaxChildDesiredSize<getElementDesiredWidth>(m_children);
//		}
//	}
//
//	template <bool t_vertical>
//	float LinearLayout<t_vertical>::getDesiredInsideHeight() const
//	{
//		if constexpr (t_vertical)
//		{
//			return getDesiredInsidePrimarySize<getElementDesiredHeight>(
//				m_children
//				, m_evenSplit
//				, m_spacing
//			);
//		}
//		else
//		{
//			return getMaxChildDesiredSize<getElementDesiredHeight>(m_children);
//		}
//	}
//
//	template class LinearLayout<true>;
//	template class LinearLayout<false>;
//}