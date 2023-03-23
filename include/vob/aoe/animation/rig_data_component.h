#pragma once

#include <vob/aoe/rendering/data/rig_data.h>

#include <vob/misc/visitor/macros.h>

#include <memory>


namespace vob::aoegl
{
	struct rig_data_component
	{
		std::shared_ptr<rig_data const> m_rigData;
	};
}

namespace vob::misvi
{
	VOB_MISVI_ACCEPT(aoegl::rig_data_component)
	{
		VOB_MISVI_NVP("Rig Data", rigData);
		return true;
	}
}
