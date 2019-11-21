#pragma once

#include <vob/aoe/core/visitor/Aggregate.h>

#include <vob/aoe/common/gui/AObject.h>


namespace vob::aoe::common::gui
{
	struct DummyObject final
		: public vis::Aggregate<DummyObject, AObject>
	{
		using Base = vis::Aggregate<DummyObject, AObject>;

		explicit DummyObject(type::CloneCopier const& a_cloneCopier)
			: Base{ a_cloneCopier }
		{}

		// Methods
		friend class vis::Aggregate<DummyObject, AObject>;
		template <typename VisitorType, typename ThisType>
		// ReSharper disable once CppMemberFunctionMayBeStatic
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			using AObjectMaybeConst = std::conditional_t<
				std::is_const_v<ThisType>
				, AObject const
				, AObject
			>;
			a_visitor.visit(static_cast<AObjectMaybeConst&>(a_this));
			AObject::makeVisit(a_visitor, a_this);
		}

		BoundingBox boundingBox(
			ObjectEntity const& a_objectEntity
			, ObjectEntityList const& a_objectEntityList
			, Constraint const& a_constraint
			, Vector2 const& a_viewSize
		) const override
		{
			auto t_constraint = a_constraint;
			m_transform.apply(t_constraint, a_viewSize);
			return {
				{t_constraint.m_x.value_or(0.0f)
					, t_constraint.m_x.value_or(0.0f)}
				, {t_constraint.m_width.value_or(100.0f)
					, t_constraint.m_height.value_or(100.0f)}
			};
		}

		void render(
			ObjectEntity const& a_objectEntity
			, ObjectEntityList const& a_objectEntityList
			, Constraint const& a_constraint
			, Vector2 const& a_viewSize
		) const override
		{}
	};
}
