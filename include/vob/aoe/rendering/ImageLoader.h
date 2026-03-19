#pragma once

#include "vob/aoe/rendering/Image.h"

#include <istream>


namespace vob::aoegl
{
	struct ImageLoader
	{
		Image load(std::istream& a_stream) const;
	};
}
