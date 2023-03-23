#pragma once

#include <vob/aoe/rendering/graphic_types.h>

namespace vob::aoegl
{
	struct material
	{
		graphic_id m_albedo = 0;
		graphic_id m_normal = 0;
		graphic_id m_metallicRoughness = 0;
	};
}
