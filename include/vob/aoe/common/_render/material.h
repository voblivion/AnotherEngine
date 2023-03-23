#pragma once

#include <vob/aoe/common/_render/GraphicResourceHandle.h>
#include <vob/aoe/common/_render/resources/Texture.h>
#include <vob/aoe/common/_render/model/model_shader_program.h>

#include <vob/misc/visitor/name_value_pair.h>


namespace vob::aoe::common
{
	struct old_material final
		: public type::ADynamicType
	{
		// Attributes
		std::shared_ptr<GraphicResourceHandle<Texture> const> m_albedo;
		std::shared_ptr<GraphicResourceHandle<Texture> const> m_normal;
		std::shared_ptr<GraphicResourceHandle<Texture> const> m_metallicRoughness;
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::old_material>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(misvi::nvp("Albedo Texture", a_this.m_albedo));
		a_visitor.visit(misvi::nvp("Normal Texture", a_this.m_normal));
		a_visitor.visit(misvi::nvp("MetallicRoughness Texture", a_this.m_metallicRoughness));
		return true;
	}
}

namespace vob::aoegl
{
	struct texture_data_old
	{
		sf::Image m_image;
	};

	struct material_data_old
	{
		texture_data_old m_albedo;
		texture_data_old m_normal;
		texture_data_old m_metallicRoughness;
	};

	struct texture
	{
		graphic_object_id m_id;

		texture(texture_data_old const& a_data)
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
			glBindTexture(GL_TEXTURE_2D, m_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(
				GL_TEXTURE_2D
				, 0 // level
				, GL_RGBA // internal format
				, a_data.m_image.getSize().x
				, a_data.m_image.getSize().y
				, 0 // border
				, GL_RGBA // format
				, GL_UNSIGNED_BYTE
				, a_data.m_image.getPixelsPtr());
		}
	};

	struct old_material
	{
		texture m_albedo;
		texture m_normal;
		texture m_metallicRoughness;

		old_material(material_data_old const& a_data)
			: m_albedo{ a_data.m_albedo }
			, m_normal{ a_data.m_normal }
			, m_metallicRoughness{ a_data.m_metallicRoughness }
		{}

		void set_uniform(model_shader_program const& a_program)
		{
			glActiveTexture(a_program.m_albedoTexture);
			glBindTexture(GL_TEXTURE_2D, m_albedo.m_id);

			glActiveTexture(a_program.m_normalTexture);
			glBindTexture(GL_TEXTURE_2D, m_normal.m_id);

			glActiveTexture(a_program.m_metallicRoughnessTexture);
			glBindTexture(GL_TEXTURE_2D, m_metallicRoughness.m_id);
		}
	};
}

