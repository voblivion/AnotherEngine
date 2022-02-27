#pragma once

#include <vob/aoe/common/render/Material.h>
#include <vob/aoe/common/render/GraphicResourceHandle.h>
#include <vob/aoe/common/render/model/StaticModel.h>

#include <vob/misc/visitor/name_value_pair.h>


namespace vob::aoe::common
{
	struct ModelComponent final
	{
		// Attributes
		std::shared_ptr<common::GraphicResourceHandle<common::StaticModel> const> m_model;
		std::vector<std::shared_ptr<common::Material const>> m_materials;
	};
}

namespace vob::misvi
{
	template <typename VisitorType, typename ThisType>
	requires std::is_same_v<std::remove_cvref_t<ThisType>, aoe::common::ModelComponent>
	bool accept(VisitorType& a_visitor, ThisType& a_this)
	{
		a_visitor.visit(misvi::nvp("Model", a_this.m_model));
		// TODO : load material?
		return true;
	}
}
