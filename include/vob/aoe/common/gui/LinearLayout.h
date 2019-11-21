#pragma once

#include <vob/aoe/common/gui/AObject.h>
#include <vob/aoe/common/gui/ObjectComponent.h>
#include <SFML/Graphics/Font.hpp>

namespace vob::aoe::common::gui
{
	struct LinearLayout final
		: public vis::Aggregate<LinearLayout, AObject>
	{
		using Base = vis::Aggregate<LinearLayout, AObject>;

		explicit LinearLayout(type::CloneCopier const& a_cloneCopier)
			: Base{ a_cloneCopier }
		{}

		friend class vis::Aggregate<LinearLayout, AObject>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			AObject::makeVisit(a_visitor, a_this);
		}

		BoundingBox boundingBox(
			ObjectEntity const& a_objectEntity
			, ObjectEntityList const& a_objectEntityList
			, Constraint const& a_constraint
			, Vector2 const& a_viewSize
		) const override
		{
			auto t_childConstraint = a_constraint;
			m_transform.apply(t_childConstraint, a_viewSize);

			// todo : Hmm...
			t_childConstraint.m_x = t_childConstraint.m_x.value_or(0.0f);
			t_childConstraint.m_y = t_childConstraint.m_y.value_or(0.0f);

			common::Vector2 const t_position{
				t_childConstraint.m_x.value_or(0.0f)
				, t_childConstraint.m_y.value_or(0.0f)
			};

			common::Vector2 t_size{
				t_childConstraint.m_width.value_or(0.0f)
				, t_childConstraint.m_height.value_or(0.0f)
			};

			auto const& t_hierarchy = a_objectEntity.getComponent<HierarchyComponent>();
			switch (m_type)
			{
			case Type::Horizontal:
			{
				if (t_childConstraint.m_width.has_value())
				{
					t_childConstraint.m_width = t_childConstraint.m_width.value()
						/ t_hierarchy.m_children.size();
				}
				break;
			}
			case Type::Vertical:
			default:
			{
				if (t_childConstraint.m_height.has_value())
				{
					t_childConstraint.m_height = t_childConstraint.m_height.value()
						/ t_hierarchy.m_children.size();
				}
				break;
			}
			}

			for (auto const& t_childEntityHandle : t_hierarchy.m_children)
			{
				auto const t_childEntity = a_objectEntityList.find(t_childEntityHandle);
				if (t_childEntity == nullptr)
				{
					continue;
				}

				auto t_childObject = t_childEntity->getComponent<ObjectComponent>();
				if (t_childObject.m_object == nullptr)
				{
					continue;
				}

				// Ask child for his own bounding box
				auto t_childBoundingBox = t_childObject.m_object->boundingBox(
					*t_childEntity
					, a_objectEntityList
					, t_childConstraint
					, a_viewSize);

				// Update constraint of next child
				switch (m_type)
				{
				case Type::Horizontal:
				{
					t_size.y = std::max(t_size.y, t_childBoundingBox.m_size.y);
					t_childConstraint.m_x = t_childConstraint.m_x.value_or(0.0f)
						+ t_childBoundingBox.m_size.x;
					break;
				}
				case Type::Vertical:
				default:
				{
					t_size.x = std::max(t_size.x, t_childBoundingBox.m_size.x);
					t_childConstraint.m_y = t_childConstraint.m_y.value_or(0.0f)
						+ t_childBoundingBox.m_size.y;
					break;
				}
				}
			}

			switch (m_type)
			{
			case Type::Horizontal:
			{
				t_size.x = t_childConstraint.m_x.value();
				break;
			}
			case Type::Vertical:
			default:
			{
				t_size.y = t_childConstraint.m_y.value();
				break;
			}
			}

			return { t_position, t_size };
		}

		void render(
			ObjectEntity const& a_objectEntity
			, ObjectEntityList const& a_objectEntityList
			, Constraint const& a_constraint
			, Vector2 const& a_viewSize
		) const override
		{
			auto t = boundingBox(a_objectEntity, a_objectEntityList, a_constraint, a_viewSize);
		}

		enum class Type
		{
			Vertical
			, Horizontal
		};

		enum class ChildSizeMode
		{
			Even
			, Free
		};

		Type m_type = Type::Vertical;
		ChildSizeMode m_childSizeMode = ChildSizeMode::Even;
	};
}