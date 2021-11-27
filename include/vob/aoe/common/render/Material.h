#pragma once

#include <vob/aoe/core/visitor/Standard.h>
#include <vob/aoe/core/visitor/Traits.h>

#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/resources/Texture.h>

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

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::Material, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(vis::makeNameValuePair("Albedo Texture", a_this.m_albedo));
		a_visitor.visit(vis::makeNameValuePair("Normal Texture", a_this.m_normal));
		a_visitor.visit(vis::makeNameValuePair("MetallicRoughness Texture", a_this.m_metallicRoughness));
	}
}
