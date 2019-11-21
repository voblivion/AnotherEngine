#pragma once

#include <vob/aoe/common/opengl/Handle.h>
#include <vob/aoe/common/opengl/resources/StaticModel.h>
#include <vob/aoe/common/render/Material.h>
#include <vob/aoe/core/data/Handle.h>
#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/visitor/Standard.h>

namespace vob::aoe::common
{
	struct GraphicComponent final
		: public vis::Aggregate<GraphicComponent, ecs::AComponent>
	{
		// Attributes
		data::Handle<ogl::Handle<ogl::StaticModel>> m_model;
		std::pmr::vector<data::Handle<ogl::Material>> m_materials;

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

			auto t_materials = vis::makeContainerHolder(
				a_this.m_materials
				, type::Factory<data::Handle<ogl::Material>>{ a_this.m_model.getDatabase() }
			);
			a_visitor.visit(vis::makeNameValuePair("Materials", t_materials));
		}
	};
}
