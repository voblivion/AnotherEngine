#include <vob/aoe/rendering/MaterialParamsLayout.h>

#include <vob/aoe/debug/Check.h>

#include <algorithm>
#include <cstring>
#include <format>


namespace vob::aoegl
{
	namespace
	{
		constexpr uint32_t k_blockAlignment = 16;

		constexpr uint32_t alignUp(uint32_t a_offset, uint32_t a_alignment)
		{
			return ((a_offset + a_alignment - 1) / a_alignment) * a_alignment;
		}
	}

	uint32_t MaterialParamsLayout::getBlockSize() const
	{
		auto blockEnd = uint32_t{ 0 };
		for (auto const& [name, slot] : slots)
		{
			blockEnd = std::max(blockEnd, slot.offset + getUniformSize(slot.variantIndex));
		}

		return alignUp(blockEnd, k_blockAlignment);
	}

	MaterialParamsLayout computeMaterialParamsLayout(UniformValueMap const& a_uniformDefaults)
	{
		auto layout = MaterialParamsLayout{};
		auto offset = uint32_t{ 0 };

		auto const addSlot = [&layout, &offset](auto const& a_entry)
			{
				auto const variantIndex = a_entry.second.index();
				offset = alignUp(offset, getUniformAlignment(variantIndex));
				layout.slots.emplace(
					a_entry.first
					, MaterialParamsLayout::Slot{ offset, static_cast<uint8_t>(variantIndex) });
				offset += getUniformSize(variantIndex);
			};

		for (auto const& entry : a_uniformDefaults)
		{
			if (getUniformSize(entry.second.index()) == 16)
			{
				addSlot(entry);
			}
		}

		auto nextPackableScalar = a_uniformDefaults.begin();
		for (auto const& entry : a_uniformDefaults)
		{
			if (getUniformSize(entry.second.index()) == 12)
			{
				addSlot(entry);
				nextPackableScalar = std::find_if(
					nextPackableScalar
					, a_uniformDefaults.end()
					, [](auto const& a_entry) { return getUniformSize(a_entry.second.index()) == 4; });
				if (nextPackableScalar != a_uniformDefaults.end())
				{
					addSlot(*nextPackableScalar);
					++nextPackableScalar;
				}
			}
		}

		for (auto const& entry : a_uniformDefaults)
		{
			if (getUniformSize(entry.second.index()) == 8)
			{
				addSlot(entry);
			}
		}

		for (auto it = nextPackableScalar; it != a_uniformDefaults.end(); ++it)
		{
			if (getUniformSize(it->second.index()) == 4)
			{
				addSlot(*it);
			}
		}

		return layout;
	}

	std::string generateMaterialParamsBlockSource(MaterialParamsLayout const& a_layout)
	{
		if (a_layout.isEmpty())
		{
			return {};
		}

		auto source = std::string{ "struct UniformMaterialParams\n{\n" };
		for (auto const& [name, slot] : a_layout.slots)
		{
			source += std::format("    {} {};\n", getUniformGlslType(slot.variantIndex), name.view());
		}
		source += "};\n\n"
			"layout(std140, binding = BINDING_UBO_MATERIAL) uniform MaterialParams\n"
			"{\n"
			"    UniformMaterialParams uMaterial;\n"
			"};\n";

		return source;
	}

	std::vector<std::byte> packMaterialParams(
		MaterialParamsLayout const& a_layout
		, UniformValueMap const& a_uniformDefaults
		, UniformValueMap const& a_uniformValues)
	{
		auto block = std::vector<std::byte>(a_layout.getBlockSize(), std::byte{ 0 });

		auto const write = [&block](MaterialParamsLayout::Slot const& a_slot, UniformValue const& a_value)
			{
				std::visit(
					[&block, &a_slot](auto const& a_typedValue)
					{
						std::memcpy(block.data() + a_slot.offset, &a_typedValue, sizeof(a_typedValue));
					}
					, a_value);
			};

		for (auto const& [name, defaultValue] : a_uniformDefaults)
		{
			write(a_layout.slots.find(name)->second, defaultValue);
		}

		for (auto const& [name, value] : a_uniformValues)
		{
			auto const slot = a_layout.slots.find(name);
			if (!VOB_AOE_CHECK_LOG(
				slot != a_layout.slots.end(), "Material sets uniform {} which is unknown.", name.view()))
			{
				continue;
			}

			if (!VOB_AOE_CHECK_LOG(
				slot->second.variantIndex == value.index()
				, "Material sets uniform {} with the wrong type."
				, name.view()))
			{
				continue;
			}

			write(slot->second, value);
		}

		return block;
	}
}
