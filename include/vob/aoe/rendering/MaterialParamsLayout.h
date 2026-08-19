#pragma once

#include "vob/aoe/rendering/UniformValue.h"

#include <cstddef>
#include <string>
#include <vector>


namespace vob::aoegl
{
	struct MaterialParamsLayout
	{
		struct Slot
		{
			uint32_t offset = 0;
			uint8_t variantIndex = 0;
		};

		mistd::string_vector_map<Slot> slots;

		[[nodiscard]] bool isEmpty() const
		{
			return slots.empty();
		}

		[[nodiscard]] uint32_t getBlockSize() const;
	};

	MaterialParamsLayout computeMaterialParamsLayout(UniformValueMap const& a_uniformDefaults);

	std::string generateMaterialParamsBlockSource(MaterialParamsLayout const& a_layout);

	std::vector<std::byte> packMaterialParams(
		MaterialParamsLayout const& a_layout
		, UniformValueMap const& a_uniformDefaults
		, UniformValueMap const& a_uniformValues);
}
