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
			: public ecs::ComponentDefaultImpl<GraphicComponent>
		{
			// Attributes
			data::Handle<Model> m_model;
			std::pmr::vector<data::Handle<Material>> m_materials;

			// Constructor
			explicit GraphicComponent(data::ADatabase& a_database)
				: m_model{ a_database }
			{}

			// Methods
			template <typename VisitorType>
				void accept(VisitorType& a_visitor)
			{
				a_visitor.visit("Model", m_model);
				a_visitor.visit("Materials", std::make_pair(std::ref(m_materials)
					, type::Factory<data::Handle<Material>>{ m_model.getDatabase() }));
			}
		};
	}
}
