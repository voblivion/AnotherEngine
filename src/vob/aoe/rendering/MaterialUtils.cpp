#include <vob/aoe/rendering/MaterialUtils.h>

#include <vob/aoe/debug/Check.h>


namespace vob::aoegl
{
	UniformValue readMaterialParam(GraphicId a_paramsUbo, MaterialParamsLayout::Slot const& a_slot)
	{
		auto value = makeUniformValue(a_slot.variantIndex);
		std::visit(
			[a_paramsUbo, &a_slot](auto& a_typedValue)
			{
				glGetNamedBufferSubData(a_paramsUbo, a_slot.offset, sizeof(a_typedValue), &a_typedValue);
			}
			, value);

		return value;
	}

	void writeMaterialParam(
		GraphicId a_paramsUbo, MaterialParamsLayout::Slot const& a_slot, UniformValue const& a_value)
	{
		if (!VOB_AOE_CHECK_LOG(a_slot.variantIndex == a_value.index(), "Material param written with the wrong type."))
		{
			return;
		}

		std::visit(
			[a_paramsUbo, &a_slot](auto const& a_typedValue)
			{
				glNamedBufferSubData(a_paramsUbo, a_slot.offset, sizeof(a_typedValue), &a_typedValue);
			}
			, a_value);
	}
}
