#pragma once

#include "vob/aoe/editor/AxisTranslateHandle.h"
#include "vob/aoe/editor/AxisRotateHandle.h"

#include <memory>


namespace vob::aoedi
{
	struct TranslateGizmoComponent
	{
		std::shared_ptr<AxisTranslateHandle> x;
		std::shared_ptr<AxisTranslateHandle> y;
		std::shared_ptr<AxisTranslateHandle> z;
		// std::shared_ptr<PlaneTranslateHandle> xy;
		// std::shared_ptr<PlaneTranslateHandle> xz;
		// std::shared_ptr<PlaneTranslateHandle> yz;
		// std::shared_ptr<FreeTranslateHandle> xyz;

		// TMP
		std::shared_ptr<AxisRotateHandle> rx;
		std::shared_ptr<AxisRotateHandle> ry;
		std::shared_ptr<AxisRotateHandle> rz;
	};
}
