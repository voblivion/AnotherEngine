#pragma once

#include <vob/aoe/core/data/Handle.h>
#include <vob/aoe/core/ecs/Component.h>
#include <vob/aoe/core/visitor/Standard.h>

#include <vob/aoe/common/render/Material.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/model/StaticModel.h>

namespace vob::aoe::common
{
	struct ModelComponent final
		: public vis::Aggregate<ModelComponent, ecs::AComponent>
	{
		// Attributes
		data::Handle<common::GraphicResourceHandle<common::StaticModel>> m_model;
		std::vector<data::Handle<common::Material>> m_materials;

		// Constructor
		explicit ModelComponent(data::ADatabase& a_database)
			: m_model{ a_database }
		{}

		// Methods
		friend class vis::Aggregate<ModelComponent, ecs::AComponent>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(vis::makeNameValuePair("Model", a_this.m_model));
		}
	};
}
