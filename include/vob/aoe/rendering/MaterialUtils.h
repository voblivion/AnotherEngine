#pragma once

#include "vob/aoe/rendering/GraphicTypes.h"
#include "vob/aoe/rendering/MaterialParamsLayout.h"


namespace vob::aoegl
{
	UniformValue readMaterialParam(GraphicId a_paramsUbo, MaterialParamsLayout::Slot const& a_slot);

	void writeMaterialParam(
		GraphicId a_paramsUbo, MaterialParamsLayout::Slot const& a_slot, UniformValue const& a_value);
}
