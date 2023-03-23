#pragma once

#include <vob/aoe/rendering/resources/model.h>

#include <vob/misc/visitor/macros.h>

#include <memory>


namespace vob::aoegl
{
	struct model_component
	{
		model m_model;
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(vob::aoegl::model_component)
	{
		return true;
	}
}
