#pragma once

#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/resources/Texture.h>

#include <vob/misc/visitor/name_value_pair.h>


namespace vob::aoe::common
{
	struct Material final
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
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::Material>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(misvi::nvp("Albedo Texture", a_this.m_albedo));
		a_visitor.visit(misvi::nvp("Normal Texture", a_this.m_normal));
		a_visitor.visit(misvi::nvp("MetallicRoughness Texture", a_this.m_metallicRoughness));
		return true;
	}
}
