#pragma once

#include <aoe/common/render/Texture.h>
#include <aoe/core/data/Handle.h>


namespace aoe
{
	namespace common
	{
		struct Material final
			: public sta::ADynamicType
		{
			// Attributes
			data::Handle<Texture> m_diffuse;
			data::Handle<Texture> m_specular;

			// Constructor
			explicit Material(data::ADatabase& a_database)
				: m_diffuse{ a_database }
				, m_specular{ a_database }
			{}

			// Methods
			template <typename VisitorType>
			void accept(VisitorType& a_visitor)
			{
				a_visitor.visit("Diffuse Texture", m_diffuse);
				a_visitor.visit("Specular Texture", m_specular);
			}
		};
	}
}
