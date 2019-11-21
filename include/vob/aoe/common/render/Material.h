#pragma once

#include <vob/aoe/core/data/Handle.h>
#include <vob/aoe/core/visitor/Aggregate.h>

#include <vob/aoe/common/opengl/Handle.h>
#include <vob/aoe/common/opengl/resources/Texture.h>

namespace vob::aoe::ogl
{
	struct Material final
		: public vis::Aggregate<Material, type::ADynamicType>
	{
		// Attributes
		data::Handle<Handle<Texture>> m_diffuse;
		data::Handle<Handle<Texture>> m_specular;

		// Constructor
		explicit Material(data::ADatabase& a_database)
			: m_diffuse{ a_database }
			, m_specular{ a_database }
		{}

	private:
		// Methods
		friend class vis::Aggregate<Material, type::ADynamicType>;
		template <typename VisitorType, typename ThisType>
		static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
		{
			a_visitor.visit(vis::makeNameValuePair("Diffuse Texture", a_this.m_diffuse));
			a_visitor.visit(vis::makeNameValuePair("Specular Texture", a_this.m_specular));
		}
	};
}
