#pragma once

#include <aoe/common/render/Material.h>
#include <aoe/common/render/Model.h>
#include <aoe/core/data/Handle.h>
#include <aoe/core/ecs/Component.h>
#include <aoe/core/visitor/Standard.h>

namespace aoe
{
	namespace common
	{
		struct GraphicComponent final
			: public vis::Aggregate<GraphicComponent, ecs::AComponent>
		{
			// Attributes
			data::Handle<Model> m_model;
			std::pmr::vector<data::Handle<Material>> m_materials;

			// Constructor
			explicit GraphicComponent(data::ADatabase& a_database)
				: m_model{ a_database }
			{}

			// Methods
			friend class vis::Aggregate<GraphicComponent, ecs::AComponent>;
			template <typename VisitorType, typename ThisType>
			static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
			{
				a_visitor.visit(vis::makeNameValuePair("Model", a_this.m_model));

				auto t_materials = vis::makeContainerHolder(a_this.m_materials
					, type::Factory<data::Handle<Material>>{ a_this.m_model.getDatabase() });
				a_visitor.visit(vis::makeNameValuePair("Materials", t_materials));
			}
		};
	}
}
