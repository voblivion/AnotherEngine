#pragma once

#include <vob/aoe/rendering/data/model_data.h>
#include <vob/aoe/rendering/resources/model.h>

#include <vob/misc/visitor/macros.h>


namespace vob::aoegl
{
	struct model_data_component
	{
		std::shared_ptr<model_data const> m_modelData;
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoegl::model_data_component)
	{
		VOB_MISVI_NVP("Model Data", modelData);
		return true;
	}
}
