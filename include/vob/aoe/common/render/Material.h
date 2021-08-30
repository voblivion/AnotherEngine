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
		std::shared_ptr<GraphicResourceHandle<Texture> const> m_diffuse;
		std::shared_ptr<GraphicResourceHandle<Texture> const> m_specular;
	};
}

namespace vob::aoe::vis
{
	template <typename VisitorType, typename ThisType>
	visitIfType<common::Material, ThisType> accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(vis::makeNameValuePair("Diffuse Texture", a_this.m_diffuse));
		a_visitor.visit(vis::makeNameValuePair("Specular Texture", a_this.m_specular));
	}
}
