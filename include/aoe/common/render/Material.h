#pragma once

#include <aoe/common/render/Texture.h>
#include <aoe/core/data/Handle.h>


namespace aoe
{
	namespace common
	{
		struct Material final
			: public vis::Aggregate<Material, sta::ADynamicType>
		{
			// Attributes
			data::Handle<Texture> m_diffuse;
			data::Handle<Texture> m_specular;

			// Constructor
			explicit Material(data::ADatabase& a_database)
				: m_diffuse{ a_database }
				, m_specular{ a_database }
			{}

		private:
			// Methods
			friend class vis::Aggregate<Material, sta::ADynamicType>;
			template <typename VisitorType, typename ThisType>
			static void makeVisit(VisitorType& a_visitor, ThisType& a_this)
			{
				a_visitor.visit(vis::makeNameValuePair("Diffuse Texture"
					, a_this.m_diffuse));
				a_visitor.visit(vis::makeNameValuePair("Specular Texture"
					, a_this.m_specular));
			}
		};
	}
}
