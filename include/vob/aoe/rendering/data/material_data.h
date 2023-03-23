#pragma once

#include <memory>

#include <vob/aoe/rendering/data/texture_data.h>


namespace vob::aoegl
{
	struct material_data
	{
		std::shared_ptr<texture_data const> m_albedo;
		std::shared_ptr<texture_data const> m_normal;
		std::shared_ptr<texture_data const> m_metallicRoughness;
	};
}
