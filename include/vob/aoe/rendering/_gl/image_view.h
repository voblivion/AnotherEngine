#pragma once

#include "types.h"


namespace vob::aoegl
{
	template <typename TChannel>
	struct image_view
	{
		graphic_enum m_format;
		graphic_size m_width;
		graphic_size m_height;
		TChannel* m_data;
	};
}
