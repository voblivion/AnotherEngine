#pragma once

#include "vob/aoe/rendering/data/ImageData.h"

#include <istream>


namespace vob::aoegl
{
	struct ImageLoader
	{
		ImageData load(std::istream& a_stream) const;
	};
}
